/*   CS 347 -- Micro Shell!
 *
 *   Sept 21, 2000,  Phil Nelson
 *   Modified April 8, 2001
 *   Modified January 6, 2003
 *   Modified January 8, 2017
 *
 *   April 03, 2019, Michael Albert
 *   Modified May 16, 2019
 *
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "defn.h"
#include "globals.h"

int globalargc;
char **globalargv;
int gotsigint;
int dollarques = 0;
int shifted = 0;

/* Constants */

#define LINELEN 200000
#define WAIT 1
#define NOWAIT 0

/* Prototypes */

int processline (char *line, int outfd, int flags);
char ** arg_parse (char *line, int *argcptr);
void removequotes(char *line);
void siginthandler (int sig);

/* Shell main */

int main (int mainargc, char **mainargv) {
  globalargc = mainargc;
  globalargv = mainargv;
  char   buffer [LINELEN];
  FILE   *input;
  int    ix;

  if (mainargc > 1) {
    if ((input = fopen(mainargv[1],"r")) == NULL) {
      perror ("open");
      exit(127);
    }
  }
  else {
    input = stdin;
  }

  signal(SIGINT, siginthandler);

  while (1) {

    gotsigint = 0;

    /* prompt and get line */
    if (input == stdin) {
    	fprintf (stderr, "%% ");
    }
  	if (fgets (buffer, LINELEN, input) != buffer)
  	  break;

    /* Get rid of \n at end of buffer or comments. */
    ix = 0;
    while (buffer[ix] != '\n' && buffer[ix] != '#') {
      /* Keep $# if found */
      if (buffer[ix] == '$' && buffer[ix+1] == '#') {
        ix++;
      }
      ix++;
    }
    buffer[ix] = 0;

  	/* Run it ... */
  	processline (buffer, 1, WAIT);

  }

    if (!feof(input))
        perror ("read");

    return 0;		/* Also known as exit (0); */
}

int processline (char *line, int outfd, int flags) {
    pid_t  cpid;
    int    status;
    int    argc;
    char   newline [LINELEN];

    /* Expand line and check if error */
    int expanded = expand(line, newline, LINELEN);

    if (expanded == -1) {
      return -1;
    }

    /* Parse the argument */
    char** parsedargs = arg_parse(newline, &argc);

    /* Return if error in arg_parse */
    if (parsedargs == NULL) {
      return -1;
    }

    /* Return if no args */
    if (argc == 0) {
      return -1;
    }

    /* Check if builtin and exec if so */
    if (isbuiltin(parsedargs, argc) == 1) {
      return 0;
    }
    /* End of code by Michael Albert */

    /* Otherwise fork the process */
    else {
      /* Start a new process to do the job. */
      cpid = fork();
      if (cpid < 0) {
        /* Fork wasn't successful */
        perror ("fork");
        return -1;
      }

      /* Check for who we are! */
      if (cpid == 0) {
        /* We are the child! */
        /* Put output onto stdout */
        if (outfd != 1) {
          dup2(outfd,1);
        }
        execvp (parsedargs[0], parsedargs);
        /* execvp reurned, wasn't successful */
        perror ("exec");
        fclose(stdin);  // avoid a linux stdio bug
        exit (127);
      }

      /* Free malloc'd space */
      free(parsedargs);

      /* Have the parent wait for child to complete */
      if (flags == WAIT) {
        if (wait (&status) < 0) {
          /* Wait wasn't successful */
          perror ("wait");
          return -1;
        }
        /* See if process ended due to signal. */
        else if (WIFSIGNALED(status)) {
          int signalint = WTERMSIG(status);
          char *issignal = strsignal(signalint);
          /* Only print out signal text if not SIGINT */
          if (signalint != SIGINT) {
            printf("%s", issignal);
            /* Check if core dumped */
            if (WCOREDUMP(status)) {
              printf(" (core dumped)");
            }
            printf("\n");
          }
          fflush(stdout);
          dollarques = 128 + WTERMSIG(status);
        }
        /* Determine if exited. If so, set dollarques to exit val */
        else if (WIFEXITED(status)) {
          dollarques = WEXITSTATUS(status);
        }
      }
      /* Return pid of child instead */
      else if (flags == NOWAIT) {
        return cpid;
      }
    }
    return 0;
}

char** arg_parse (char *line, int *argcptr)
{
    int    argc; //keeps track of total args
    int    ix, jx; //iterators
    char* startArg; //start of argument
    char** returnVal; //return value

    argc = 0;

    /* Find number of args */
    ix = 0;
    while (line[ix] != 0) {
      /* Skip spaces */
      while (line[ix] == ' ') {
        ix++;
      }
      /* Add argument */
      if (line[ix] != ' ' && line[ix] != 0) {
        argc++;
        /* Loop to end of arg */
        while (line[ix] != ' ' && line[ix] != 0) {
          if (line[ix] != '\"') {
            ix++;
          }
          /* Anything within quotes counts as arg */
          else {
            ix++;
            /* Find end of quotes */
            while (line[ix] != '\"') {
              ix++;
              /* Mismatched quotes gives error */
              if (line[ix] == 0) {
                fprintf(stderr, "Mismatched quotes.\n");
                returnVal = NULL;
                return returnVal;
              }
            }
            ix++;
          }
        }
      }
    }

    /* Allocate exact amount of space */
    returnVal = malloc(sizeof(char*)*(argc+1));

    /* Check if enough memory */
    if (returnVal == NULL) {
      fprintf(stderr, "Memory not allocated\n");
      return returnVal;
    }

    /* Assign pointers and EoS values to args */
    ix = 0;
    jx = 0;
    while (line[ix] != 0) {
      /* Skip spaces */
      while (line[ix] == ' ') {
        ix++;
      }
      /* Add pointer to start of arg */
      if (line[ix] != ' ' && line[ix] != 0) {
        startArg = &line[ix];
        returnVal[jx] = startArg;
        jx++;
        /* Loop to end of arg */
        while (line[ix] != ' ' && line[ix] != 0) {
          if (line[ix] != '\"') {
            ix++;
          }
          /* Anything within quotes counts as arg */
          else {
            ix++;
            /* Find end of quotes */
            while (line[ix] != '\"') {
              ix++;
            }
            ix++;
          }
        }
        /* Add EoS value to end of arg if not last arg */
        if (line[ix] != 0) {
          line[ix] = 0;
          ix++;
        }
        /* Remove quotes from arg */
        removequotes(startArg);
      }
    }

    /* Set final char* to NULL */
    returnVal[jx] = NULL;
    /* Modify value argcptr points to */
    *argcptr = argc;

    return returnVal;
}

void removequotes (char *line) {
  int src, dst; //iterators

  /* Remove quotes from the arg */
  src = 0;
  dst = 0;
  while (line[src] != 0) {
    /* Bring directly over if no quote */
    if (line[src] != '\"') {
      line[dst] = line[src];
      dst++;
    }
  src++;
  }

  line[dst] = 0;
  return;
}

void siginthandler (int sig) {
  gotsigint = 1;
  signal(sig, SIG_IGN);
  signal(SIGINT, siginthandler);

  return;
}
