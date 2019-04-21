/*   CS 347 -- Expand
 *
 *   April 14, 2019, Michael Albert
 *   Modified April 16, 2019
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "defn.h"

/* Prototypes */

int specprocess (char *orig, char *new, int newsize, int *ix, int *jx);

/* Expand */

int expand (char *orig, char *new, int newsize) {
  int ix, jx; //iterators

  /* Loop through orig once */
  ix = 0;
  jx = 0;
  while (orig[ix] != 0) {
    /* Replace text with something else */
    if (orig[ix] == '$') {
      int processed = specprocess (orig, new, newsize, &ix, &jx);
      /* Make sure specprocess ran correctly */
      if (processed == -1) {
        return -1;
      }
    }

    /* Copy over char */
    else {
      new[jx] = orig[ix];
      jx++;
      ix++;
    }
  }
  /* Set final char as end of string */
  new[jx] = 0;
  return 0;
}

int specprocess (char *orig, char *new, int newsize, int *ix, int *jx) {
  char *env; //string from getenv
  int temp; //placeholder
  int kx; //iterator

  if (orig[*ix+1] == '{') {
    temp = *ix+2;
    /* Loop until '}' found */
    while (orig[*ix] != '}') {
      /* Return if no closing brace */
      if (orig[*ix] == 0) {
        printf("Mismatched braces.\n");
        return -1;
      }
      (*ix)++;
    }
    /* Get environment from ${NAME} */
    orig[*ix] = 0;
    env = getenv(&orig[temp]);
    orig[*ix] = '}';
    (*ix)++;
    /* Add env to new if it exists */
    if (env != NULL) {
      kx = 0;
      while (env[kx] != 0) {
        new[*jx] = env[kx];
        (*jx)++;
        kx++;
      }
    }
  }

  /* Print out pid from $$ */
  else if (orig[*ix+1] == '$') {
    *ix = *ix + 2;
    char buffer[10];
    snprintf(buffer, 10, "%d", getpid());
    kx = 0;
    /* Add pid to new */
    while (buffer[kx] != 0) {
      new[*jx] = buffer[kx];
      (*jx)++;
      kx++;
    }
  }

  /* No special character after first $, do nothing */
  else {
    new[*jx] = orig[*ix];
    (*jx)++;
    (*ix)++;
  }

  return 0;
}
