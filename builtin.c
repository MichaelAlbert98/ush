/*   CS 347 -- Builtin
 *
 *   April 16, 2019, Michael Albert
 *   Modified May 06, 2019
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>
#include "defn.h"
#include "globals.h"

/* Constants */

#define BUILTINNUM 7
const char* builtins[] = { "exit", "envset", "envunset", "cd", "shift",
                          "unshift", "sstat"};

/* Prototypes */

void exitbuilt (char **parsedargs, int argc);
void envsetbuilt (char **parsedargs, int argc);
void envunsetbuilt (char **parsedargs, int argc);
void cdbuilt (char **parsedargs, int argc);
void shift (char **parsedargs, int argc);
void unshift (char **parsedargs, int argc);
void sstat (char **parsedargs, int argc);
void strmode (mode_t mode, char *p);

/* Builtin */

int isbuiltin (char **parsedargs, int argc) {
  /* Check if command is a builtin */
  for (int i = 0; i < BUILTINNUM; i++) {
    if (strcmp(parsedargs[0], builtins[i]) == 0) {
      /* Call corresponding builtin helper */
      void (*funcptrarr[])(char**, int) = {exitbuilt, envsetbuilt, envunsetbuilt,
            cdbuilt, shift, unshift, sstat};
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

void shift (char **parsedargs, int argc) {
  /* Print error */
  if (argc > 2) {
    fprintf(stderr, "Incorrect number of arguments.\n");
  }
  /* Shift by n if possible */
  else if (argc == 2 && shifted + atoi(parsedargs[1]) < globalargc - 1) {
    shifted = shifted + atoi(parsedargs[1]);
  }
  /* Shift by 1 if possible */
  else if (argc == 1 && shifted + 1 < globalargc - 1) {
    shifted = shifted + 1;
  }
  /* Shift too large */
  else {
    fprintf(stderr, "Shift value too large\n");
  }
}

void unshift (char **parsedargs, int argc) {
  /* Print error */
  if (argc > 2) {
    fprintf(stderr, "Incorrect number of arguments.\n");
  }
  /* Unshift by n if possible */
  else if (argc == 2 && atoi(parsedargs[1]) <= shifted) {
    shifted = shifted - atoi(parsedargs[1]);
  }
  /* Unshift entirely if possible */
  else if (argc == 1) {
    shifted = 0;
  }
  /* Unshift too large */
  else {
    fprintf(stderr, "Unshift value too large\n");
  }
}

void sstat (char **parsedargs, int argc) {
  /* Print error */
  if (argc < 2) {
    fprintf(stderr, "Incorrect number of arguments.\n");
  }
  /* Loops through and get info about each file */
  else if (argc >= 2) {
    for (int i = 1; i < argc; i++) {
      struct stat sbuf;
      if (stat(parsedargs[i], &sbuf) == 0) {
        /* File name */
        fprintf("%s ", parsedargs[i]);
        /* User name */
        struct passwd *uname = getpwuid(sbuf.st_uid);
        if (uname == NULL) {
          fprintf("%d ", sbuf.st_uid);
        }
        else {
          fprintf("%s ", uname->pw_name);
        }
        /* Group name */
        struct group *gname = getgrgid(sbuf.st_gid);
        if (gname == NULL) {
          fprintf("%d ", sbuf.st_gid);
        }
        else {
          fprintf("%s ", gname->gr_name);
        }
        /* Permission list */
        char str[11];
        strmode(sbuf.st_mode, str);
        fprintf("%s", str);
        /* Num links */
        fprintf("%li ", sbuf.st_nlink);
        /* File size (bytes) */
        fprintf("%li ", sbuf.st_size);
        /* Modification time */
        time_t sec = sbuf.st_mtim.tv_sec;
        fprintf("%s", asctime(localtime(&sec)));
      }
      /* Could not open file */
      else {
        perror ("open");
      }
    }
  }
  return;
}
