/*   CS 347 -- Builtin
 *
 *   April 16, 2019, Michael Albert
 *   Modified May 11, 2019
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

void exitbuilt (char **parsedargs, int argc, int outfd);
void envsetbuilt (char **parsedargs, int argc, int outfd);
void envunsetbuilt (char **parsedargs, int argc, int outfd);
void cdbuilt (char **parsedargs, int argc, int outfd);
void shift (char **parsedargs, int argc, int outfd);
void unshift (char **parsedargs, int argc, int outfd);
void sstat (char **parsedargs, int argc, int outfd);
void strmode (mode_t mode, char *p);

/* Builtin */

int isbuiltin (char **parsedargs, int argc, int outfd) {
  /* Check if command is a builtin */
  for (int i = 0; i < BUILTINNUM; i++) {
    if (strcmp(parsedargs[0], builtins[i]) == 0) {
      /* Call corresponding builtin helper */
      void (*funcptrarr[])(char**, int, int) = {exitbuilt, envsetbuilt, envunsetbuilt,
            cdbuilt, shift, unshift, sstat};
      (*funcptrarr[i])(parsedargs, argc, outfd);
      return 1;
    }
  }
  /* Not a builtin */
  return 0;
}

void exitbuilt (char **parsedargs, int argc, int outfd) {
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

void envsetbuilt (char **parsedargs, int argc, int outfd) {
  /* Print error */
  if (argc != 3) {
    fprintf(stderr, "Incorrect number of arguments.\n");
    dollarques = 1;
  }
  /* Set the given environment. onceverwriting allowed */
  else {
    setenv(parsedargs[1],parsedargs[2],1);
    dollarques = 0;
  }
  return;
}

void envunsetbuilt (char **parsedargs, int argc, int outfd) {
  /* Print error */
  if (argc != 2) {
    fprintf(stderr, "Incorrect number of arguments.\n");
    dollarques = 1;
  }
  /* Remove environment with given name */
  else {
    unsetenv(parsedargs[1]);
    dollarques = 0;
  }
  return;
}

void cdbuilt (char **parsedargs, int argc, int outfd) {
  /* Print error */
  dollarques = 0;
  if (argc > 2) {
    fprintf(stderr, "Incorrect number of arguments.\n");
    dollarques = 1;
  }
  /* Change directory to given path */
  else if (argc == 2){
    if (chdir(parsedargs[1]) != 0) {
      fprintf(stderr, "No such file or directory.\n");
      dollarques = 1;
    }
  }
  /* Default go home */
  else {
    if (chdir(getenv("HOME")) != 0) {
      fprintf(stderr, "Home not set.\n");
      dollarques = 1;
    }
  }
  return;
}

void shift (char **parsedargs, int argc, int outfd) {
  dollarques = 1;
  /* Print error */
  if (argc > 2) {
    fprintf(stderr, "Incorrect number of arguments.\n");
  }
  /* Shift by n if possible */
  else if (argc == 2 && shifted + atoi(parsedargs[1]) < globalargc - 1) {
    shifted = shifted + atoi(parsedargs[1]);
    dollarques = 0;
  }
  /* Shift by 1 if possible */
  else if (argc == 1 && shifted + 1 < globalargc - 1) {
    shifted = shifted + 1;
    dollarques = 0;
  }
  /* Shift too large */
  else {
    fprintf(stderr, "Shift value too large\n");
  }
  return;
}

void unshift (char **parsedargs, int argc, int outfd) {
  dollarques = 1;
  /* Print error */
  if (argc > 2) {
    fprintf(stderr, "Incorrect number of arguments.\n");
  }
  /* Unshift by n if possible */
  else if (argc == 2 && atoi(parsedargs[1]) <= shifted) {
    shifted = shifted - atoi(parsedargs[1]);
    dollarques = 0;
  }
  /* Unshift entirely if possible */
  else if (argc == 1) {
    shifted = 0;
    dollarques = 0;
  }
  /* Unshift too large */
  else {
    fprintf(stderr, "Unshift value too large\n");
  }
  return;
}

void sstat (char **parsedargs, int argc, int outfd) {
  dollarques = 0;
  /* Print error */
  if (argc < 2) {
    fprintf(stderr, "Incorrect number of arguments.\n");
    dollarques = 1;
  }
  /* Loops through and get info about each file */
  else if (argc >= 2) {
    for (int i = 1; i < argc; i++) {
      struct stat sbuf;
      if (stat(parsedargs[i], &sbuf) == 0) {
        /* File name */
        dprintf(outfd, "%s ", parsedargs[i]);
        /* User name */
        struct passwd *uname = getpwuid(sbuf.st_uid);
        if (uname == NULL) {
          dprintf(outfd, "%d ", sbuf.st_uid);
        }
        else {
          dprintf(outfd, "%s ", uname->pw_name);
        }
        /* Group name */
        struct group *gname = getgrgid(sbuf.st_gid);
        if (gname == NULL) {
          dprintf(outfd, "%d ", sbuf.st_gid);
        }
        else {
          dprintf(outfd, "%s ", gname->gr_name);
        }
        /* Permission list */
        char str[11];
        strmode(sbuf.st_mode, str);
        dprintf(outfd, "%s", str);
        /* Num links */
        dprintf(outfd, "%li ", sbuf.st_nlink);
        /* File size (bytes) */
        dprintf(outfd, "%li ", sbuf.st_size);
        /* Modification time */
        time_t sec = sbuf.st_mtim.tv_sec;
        dprintf(outfd, "%s", asctime(localtime(&sec)));
        fflush(stdout);
      }
      /* Could not open file */
      else {
        perror ("open");
        dollarques = 1;
      }
    }
  }
  return;
}
