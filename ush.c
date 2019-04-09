/*   CS 347 -- Micro Shell!
 *
 *   Sept 21, 2000,  Phil Nelson
 *   Modified April 8, 2001
 *   Modified January 6, 2003
 *   Modified January 8, 2017
 *
 *   April 03, 2019, Michael Albert
 *   Modified April 9, 2019
 *
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

/* Constants */

#define LINELEN 1024

/* Prototypes */

void processline (char *line);
char ** arg_parse (char *line, int *argcptr);

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
    int*   argcptr = malloc(sizeof(int));
    char** parsedArgs = arg_parse(line, argcptr);

    /* Return if no args or out of memory */
    if (parsedArgs == NULL) {
      perror ("no args");
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
      execvp (line, parsedArgs);
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
char ** arg_parse (char *line, int *argcptr)
{
    int    argc; //keeps track of total args
    size_t len; //length of user input
    int    ix, jx; //iterators
    char** returnVal; //return value

    argc = 0;
    len = strlen(line);

    /* Break up line on spaces */
    for (ix=1; ix<len; ix++) {
      /* Ignore multiple spaces */
      if (line[ix] == ' ' && (line[ix-1] != ' ' && line[ix-1] != '\0')) {
        line[ix] = '\0';
        argc++;
      }
    }

    /* Allocate exact amount of space */
    returnVal = malloc(sizeof(char*)*(argc+1));

    /* Check if enough memory */
    if (returnVal == 0) {
      perror ("memory");
      return returnVal;
    }

    /* Point to first char of each arg */
    for (ix=0,jx=0; ix<len; ix++) {
      /* Make sure we find first char of each arg */
      if (line[ix] != ' ' && (ix==0 || line[ix-1] == ' ' || line[ix-1] == '\0')) {
        returnVal[jx] = &line[ix];
        jx++;
      }
    }

    /* Set final char* to NULL */
    returnVal[jx] = NULL;
    /* Modify value argcptr points to */
    *argcptr = argc;

    return returnVal;
}
/* End of code by Michael Albert */
