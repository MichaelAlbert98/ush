/*   CS 347 -- Builtin
 *
 *   April 16, 2019, Michael Albert
 *   Modified May 05, 2019
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "defn.h"

/* Constants */

#define BUILTINNUM 4
const char* builtins[] = { "exit", "envset", "envunset", "cd", "sstat"};

/* Prototypes */

void exitbuilt (char **parsedargs, int argc);
void envsetbuilt (char **parsedargs, int argc);
void envunsetbuilt (char **parsedargs, int argc);
void cdbuilt (char **parsedargs, int argc);
void sstat (char **parsedargs, int argc);

/* Builtin */

int isbuiltin (char **parsedargs, int argc) {
  /* Check if command is a builtin */
  for (int i = 0; i < BUILTINNUM; i++) {
    if (strcmp(parsedargs[0], builtins[i]) == 0) {
      /* Call corresponding builtin helper */
      void (*funcptrarr[])(char**, int) = {exitbuilt, envsetbuilt, envunsetbuilt,
            cdbuilt,sstat};
      (*funcptrarr[i])(parsedargs, argc);
      return 1;
    }
  }
  /* Not a builtin */
  return 0;
}

void exitbuilt (char **parsedargs, int argc) {
  /* Print error */
  if (argc > 2) {
    fprintf(stderr, "Incorrect number of arguments.\n");
  }
  /* Exit with given value */
  else if (argc == 2) {
    exit(atoi(parsedargs[1]));
  }
  /* Default exit */
  else {
    exit(0);
  }
  return;
}

void envsetbuilt (char **parsedargs, int argc) {
  /* Print error */
  if (argc != 3) {
    fprintf(stderr, "Incorrect number of arguments.\n");
  }
  /* Set the given environment. No overwriting */
  else {
    setenv(parsedargs[1],parsedargs[2],0);
  }
  return;
}

void envunsetbuilt (char **parsedargs, int argc) {
  /* Print error */
  if (argc != 2) {
    fprintf(stderr, "Incorrect number of arguments.\n");
  }
  /* Remove environment with given name */
  else {
    unsetenv(parsedargs[1]);
  }
  return;
}

void cdbuilt (char **parsedargs, int argc) {
  /* Print error */
  if (argc > 2) {
    fprintf(stderr, "Incorrect number of arguments.\n");
  }
  /* Change directory to given path */
  else if (argc == 2){
    if (chdir(parsedargs[1]) != 0) {
      fprintf(stderr, "No such file or directory.\n");
    }
  }
  /* Default go home */
  else {
    if (chdir(getenv("HOME")) != 0) {
      fprintf(stderr, "Home not set.\n");
    }
  }
  return;
}

void sstat (char **parsedargs, int argc) {
  /* Print error */
  if (argc < 2) {
    fprintf(stderr, "Incorrect number of arguments.\n");
  }
  /* Loops through and get info about each file */
  for (int i = 0; i < argc; i++) {
    /* placeholder */
    int j = 0;
  }
  return;
}
