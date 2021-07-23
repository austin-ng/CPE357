#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <math.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

#include "mytar.h"

#define MAX_LETTERS 7 /* the argument "ctxvfS" consists of 6 chars plus \0 */
#define BLOCKSIZE 512 /* size of a data block*/

int checkForChar(char* word, char c); /* check for a specific letter*/
int checkForOther(char* word); /* check for unrecognized letters */
void usage();

int main(int argc, char * argv[]) {

    if (argc < 3) {
        usage();
    }

    if (checkForOther(argv[1])) {
        usage();
    } /* if unrecognized chars in argument, usage error */

    if (!checkForChar(argv[1], 'f')) {
        usage();
    } /*  if f not present, usage error */

    int cIsPresent = checkForChar(argv[1], 'c');
    int tIsPresent = checkForChar(argv[1], 't');
    int xIsPresent = checkForChar(argv[1], 'x');
    int vIsPresent = checkForChar(argv[1], 'v');
    int SIsPresent = checkForChar(argv[1], 'S');

    int i; /* for iteration */

    if (cIsPresent) {
        if (tIsPresent || xIsPresent) {
            fprintf(stderr, 
		    "mytar: you may only choose one of the 'ctx' options.\n");
            usage();
        }
        else {
            createTar(argv[2], argv, argc, vIsPresent, SIsPresent);
        }
    }

    else if (tIsPresent) {
        if (cIsPresent || xIsPresent) {
            fprintf(stderr, 
		    "mytar: you may only choose one of the 'ctx' options.\n");
            usage();
        }
        else {
            printTar(argv[2], argv, argc, vIsPresent);
        }
    }

    else if (xIsPresent) {
        if (cIsPresent || tIsPresent) {
            fprintf(stderr,
		    "mytar: you may only choose one of the 'ctx' options.\n");
            usage();
        }
        else {
            int fd = open(argv[2], O_RDONLY);
	    if (argc == 3) {
		extractTar(NULL, argc, fd, vIsPresent, SIsPresent);
	    }
	    else {
                extractTar(argv, argc, fd, vIsPresent, SIsPresent);
	    }
        }
    }

    return 0;
}

int checkForChar(char* word, char c) {
    int count;
    int isThere = 0; /* check for char in argument */
    for (count = 0; word[count]; count++) {
        if (word[count] == c) {
            isThere = 1;
        }
    }
    return isThere;
}

int checkForOther(char *word) {
    int count;
    int isOthers = 0;
    for (count = 0; word[count]; count++) {
        if (word[count] != 'c' &&
            word[count] != 't' &&
            word[count] != 'x' &&
            word[count] != 'v' &&
            word[count] != 'f' &&
            word[count] != 'S') {
            fprintf(stderr, "mytar: unrecognized option '%c'.\n", word[count]);
            isOthers = 1;
        }
    }
    return isOthers;
}

void usage() {
    fprintf(stderr, "usage: mytar [ctxvS]f tarfile [ path [ ... ] ]\n");
    exit(EXIT_FAILURE);
}
