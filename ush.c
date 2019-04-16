/*   CS 347 -- Micro Shell!
 *
 *   Sept 21, 2000,  Phil Nelson
 *   Modified April 8, 2001
 *   Modified January 6, 2003
 *   Modified January 8, 2017
 *
 *   April 03, 2019, Michael Albert
 *   Modified April 16, 2019
 *
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "defn.h"

/* Constants */

#define LINELEN 1024

/* Prototypes */

void processline (char *line);
char ** arg_parse (char *line, int *argcptr);
void removequotes(char *line);

/* Shell main */

int
main (void)
{
    char   buffer [LINELEN];
    int    len;

    while (1) {

        /* prompt and get line */
	fprintf (stderr, "%% ");
	if (fgets (buffer, LINELEN, stdin) != buffer)
	  break;

        /* Get rid of \n at end of buffer. */
	len = strlen(buffer);
	if (buffer[len-1] == '\n')
	    buffer[len-1] = 0;

	/* Run it ... */
	processline (buffer);

    }

    if (!feof(stdin))
        perror ("read");

    return 0;		/* Also known as exit (0); */
}

void processline (char *line)
{
    pid_t  cpid;
    int    status;
    int    argcptr;
    char   newline [LINELEN];

    /* Expand line and check if error */
    int expanded = expand(line, newline, LINELEN);

    if (expanded == -1) {
      return;
    }

    /* Parse the argument */
    char** parsedArgs = arg_parse(newline, &argcptr);

    /* Return if error in arg_parse */
    if (parsedArgs == NULL) {
      return;
    }

    /* Return if no args */
    if (argcptr == 0) {
      printf("No arguments.\n");
      return;
    }

    /* Start a new process to do the job. */
    cpid = fork();
    if (cpid < 0) {
      /* Fork wasn't successful */
      perror ("fork");
      return;
    }

    /* Check for who we are! */
    if (cpid == 0) {
      /* We are the child! */
      execvp (parsedArgs[0], parsedArgs);
      /* execvp reurned, wasn't successful */
      perror ("exec");
      fclose(stdin);  // avoid a linux stdio bug
      exit (127);
    }

    /* Have the parent wait for child to complete */
    if (wait (&status) < 0) {
      /* Wait wasn't successful */
      perror ("wait");
    }

    /* Free malloc'd space */
    free(parsedArgs);
}

/* Start of code by Michael Albert */
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
      if (line[ix] != ' ') {
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
/* End of code by Michael Albert */
