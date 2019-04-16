/*   CS 347 -- Expand
 *
 *   April 14, 2019, Michael Albert
 *   Modified April 15, 2019
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "defn.h"

/* Expand */

int expand (char *orig, char *new, int newsize) {
  int ix, jx, kx; //iterators
  char *env; //string returned from getenv
  int temp; //temp value

  /* Loop through orig once */
  ix = 0;
  jx = 0;
  while (orig[ix] != 0) {
    /* Replace text with env */ //Note: Make this a helper function to easily add other expansions.
    if (orig[ix] == '$' && orig[ix+1] == '{') {
      temp = ix+2;
      /* Loop until '}' found */
      while (orig[ix] != '}') {
        /* Return if no closing brace */
        if (orig[ix] == 0) {
          printf("Mismatched braces.\n");
          return -1;
        }
        ix++;
      }
      /* Get environment from ${NAME} */
      orig[ix] = 0;
      env = getenv(&orig[temp]);
      orig[ix] = '}';
      ix++;
      /* Add env to new if it exists */
      if (env != NULL) {
        kx = 0;
        while (env[kx] != 0) {
          new[jx] = env[kx];
          jx++;
          kx++;
        }
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
