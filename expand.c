/*   CS 347 -- Expand
 *
 *   April 14, 2019, Michael Albert
 *   Modified May 05, 2019
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include "defn.h"
#include "globals.h"

/* Prototypes */

int specprocess (char *orig, char *new, int newsize);
int wildcard (char *orig, char *new, int newsize);
int checkdigits (char *orig);

/* Global Variables */
int ix = 0;
int jx = 0;

/* Expand */

int expand (char *orig, char *new, int newsize) {
  /* Reset ix and jx */
  ix = 0;
  jx = 0;

  /* Loop through orig once */
  while (orig[ix] != 0) {
    /* Replace text with something else */
    if (orig[ix] == '$') {
      int processed = specprocess (orig, new, newsize);
      /* Make sure specprocess ran correctly */
      if (processed == -1) {
        return -1;
      }
    }
    /* wildcard expansion */
    else if (orig[ix] == '*') {
      int processed = wildcard (orig, new, newsize);
      /* Make sure wildcard ran correctly */
      if (processed == -1) {
        return -1;
      }
    }
    /* Copy over char */
    else {
      /* Make sure no buffer overflow */
      if (jx == newsize) {
        fprintf(stderr, "Expansion too long.\n");
        return -1;
      }
      new[jx] = orig[ix];
      jx++;
      ix++;
    }
  }
  /* Set final char as end of string */
  new[jx] = 0;
  return 0;
}

int specprocess (char *orig, char *new, int newsize) {
  char *env; //string from getenv
  int temp; //placeholder
  int kx; //iterator
  int value = checkdigits(orig); //val of digits after $

  /* Replace $n with command line arg n+1 */
  if (value != -1) {
    char *name;
    int kx = 0;
    /* No arguments given */
    if (globalargc == 1) {
      /* Replace with shell name */
      if (value == 0) {
        name = globalargv[0];
        /* Copy over shell name */
        while (name[kx] != 0) {
          /* Make sure no buffer overflow */
          if (jx == newsize) {
            fprintf(stderr, "Expansion too long.\n");
            return -1;
          }
          new[jx] = name[kx];
          jx++;
          kx++;
        }
      }
    }
    /* Only replace if $n isn't too large */
    else if (globalargc > value + 1) {
      char *name = globalargv[value+1];
      /* Copy over arg name */
      while (name[kx] != 0) {
        /* Make sure no buffer overflow */
        if (jx == newsize) {
          fprintf(stderr, "Expansion too long.\n");
          return -1;
        }
        new[jx] = name[kx];
        jx++;
        kx++;
      }
    }
  }

  else if (orig[ix+1] == '{') {
    temp = ix+2;
    /* Loop until '}' found */
    while (orig[ix] != '}') {
      /* Return if no closing brace */
      if (orig[ix] == 0) {
        printf("No matching } found.\n");
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
        /* Make sure no buffer overflow */
        if (jx == newsize) {
          fprintf(stderr, "Expansion too long.\n");
          return -1;
        }
        new[jx] = env[kx];
        jx++;
        kx++;
      }
    }
  }

  /* Print out pid from $$ */
  else if (orig[ix+1] == '$') {
    ix = ix + 2;
    char buffer[10];
    snprintf(buffer, 10, "%d", getpid());
    kx = 0;
    /* Add pid to new */
    while (buffer[kx] != 0) {
      /* Make sure no buffer overflow */
      if (jx == newsize) {
        fprintf(stderr, "Expansion too long.\n");
        return -1;
      }
      new[jx] = buffer[kx];
      jx++;
      kx++;
    }
  }

  /* Replace $# with num of args */
  else if (orig[ix+1] == '#') {
    ix = ix + 2;
    char buffer[6];
    snprintf(buffer, 6, "%d", globalargc-1-shifted);
    kx = 0;
    /* Add num of args to new */
    while (buffer[kx] != 0) {
      /* Make sure no buffer overflow */
      if (jx == newsize) {
        fprintf(stderr, "Expansion too long.\n");
        return -1;
      }
      new[jx] = buffer[kx];
      jx++;
      kx++;
    }
  }

  /* No special character after first $, do nothing */
  else {
    /* Make sure no buffer overflow */
    if (jx == newsize) {
      fprintf(stderr, "Expansion too long.\n");
      return -1;
    }
    new[jx] = orig[ix];
    jx++;
    ix++;
  }

  return 0;
}

int wildcard (char *orig, char *new, int newsize) {
  int kx; //iterator
  int temp; //placeholder
  int changed; //tells if temp changed value

  /* Character before '*' */
  if (ix != 0 && orig[ix-1] != ' ') {
    /* Add * to new, while removing '\' that was already added */
    if (orig[ix-1] == '\\') {
      new[jx-1] = '*';
      ix++;
      return 0;
    }
    /* Make sure no buffer overflow */
    if (jx == newsize) {
      fprintf(stderr, "Expansion too long.\n");
      return -1;
    }
    /* Copy the '*' to new */
    new[jx] = '*';
    ix++;
    jx++;
    return 0;
  }

  ix++;

  /* Wildcard expansion for lone '*' */
  if (orig[ix] == ' ' || orig[ix] == 0) {
    /* Open working directory */
    DIR *workingdirec = opendir(".");
    /* Make sure it was opened */
    if (workingdirec == NULL) {
      perror ("opendir()");
      return -1;
    }
    struct dirent *entry;
    /* Pull name from entry for each directory */
    while ((entry = readdir(workingdirec)) != NULL) {
        char *name = entry->d_name;
        /* Add to new if it doesn't start with '.' */
        if (name[0] != '.') {
          kx = 0;
          while (name[kx] != 0) {
            /* Make sure no buffer overflow */
            if (jx == newsize) {
              fprintf(stderr, "Expansion too long.\n");
              return -1;
            }
            new[jx] = name[kx];
            jx++;
            kx++;
          }
          /* Make sure no buffer overflow */
          if (jx == newsize) {
            fprintf(stderr, "Expansion too long.\n");
            return -1;
          }
          /* Add space between entries */
          new[jx] = ' ';
          jx++;
        }
    }
    /* Remove trailing space */
    jx--;
    closedir(workingdirec);
  }

  /* Wildcard expansion for trailing context */
  else if (orig[ix] != ' ' && orig[ix] != 0) {
    /* Open working directory */
    DIR *workingdirec = opendir(".");
    /* Make sure it was opened */
    if (workingdirec == NULL) {
      perror ("opendir()");
      return -1;
    }
    /* make temp string of context to compare names to */
    temp = ix;
    while (orig[ix] != ' ' && orig[ix] != 0) {
      ix++;
    }
    if (orig[ix] == ' ') {
      orig[ix] = 0;
      changed = 1;
    }
    /* Return if '/' in context */
    if (strchr(&orig[temp],'/') != NULL) {
      fprintf(stderr, "/ found in *<context>.\n");
      return -1;
    }
    struct dirent *entry;
    int found = jx;
    int len = strlen(&orig[temp]);
    /* Pull name from entry for each directory */
    while ((entry = readdir(workingdirec)) != NULL) {
        char *name = entry->d_name;
        /* Add to new if it doesn't start with '.' and
           the last len chars are the same as context */
        int namelen = strlen(name);
        if (namelen >= len) {
          char *lastlen = &name[namelen-len];
          if (name[0] != '.' && strcmp(&orig[temp],lastlen) == 0) {
            kx = 0;
            while (name[kx] != 0) {
              /* Make sure no buffer overflow */
              if (jx == newsize) {
                fprintf(stderr, "Expansion too long.\n");
                return -1;
              }
              new[jx] = name[kx];
              jx++;
              kx++;
            }
            /* Make sure no buffer overflow */
            if (jx == newsize) {
              fprintf(stderr, "Expansion too long.\n");
              return -1;
            }
            /* Add space between entries */
            new[jx] = ' ';
            jx++;
          }
        }
    }
    /* Put space back if changed */
    if (changed == 1) {
      orig[ix] = ' ';
    }
    /* If no file names added, add pattern to new */
    if (found == jx) {
      while (orig[temp] != 0 && orig[temp] != ' ') {
        new[jx] = orig[temp];
        jx++;
        temp++;
      }
    }
    /* Remove trailing space if file names added */
    else {
      jx--;
    }
    closedir(workingdirec);
  }
  return 0;
}

int checkdigits(char *orig) {
  /* Check if first char is digit */
  if (isdigit(orig[ix+1]) == 0) {
    return -1;
  }
  ix++;
  int temp = ix;
  /* Loop until non-digit is reached */
  while (isdigit(orig[ix]) != 0) {
    ix++;
  }
  /* orig[ix] already 0, do nothing */
  if (orig[ix] == 0) {
    /* Make sure shift does not affect $0 */
    if ((temp = atoi(&orig[temp])) == 0) {
      return 0;
    }
    return temp+shifted;
  }
  /* Replace orig[ix] with 0 temporarily */
  char tempc = orig[ix];
  orig[ix] = 0;
  /* Make sure shift does not affect $0 */
  if ((temp = atoi(&orig[temp])) == 0) {
    orig[ix] = tempc;
    return 0;
  }
  temp = temp+shifted;
  orig[ix] = tempc;
  return temp;
}
