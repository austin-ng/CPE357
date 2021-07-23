#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mush.h"

#define MAX_CMD_LEN 512 /*Maximum command line length*/
#define MAX_ARGS_PIPE 20 /*Maximum arguments in pipe*/
#define MAX_ARGS_CMD 20 /*Maximum arguments in command*/
#define PRINTS_PER_STAGE 13 /*Number of printf calls per stage*/
#define MAX_ERROR_MSG_LEN 30 /*Buffer size for error messages*/

int validLine(char* line) {
    /*FIX ME*/
    return 1;
}

int main(int argc, char* argv[]) {
    char* cmd_line; /*Buffer for command line*/

    cmd_line = malloc(MAX_CMD_LEN + 1);
    memset(cmd_line, '\0', MAX_CMD_LEN + 1);

    while (strcmp(cmd_line, "^D") != 0) {
        printf("8-P "); /*Take user input and put into command line buffer*/
        if (scanf("%[^\n]%*c", cmd_line) < 0) { /*Line too long error check*/
	    fprintf(stderr, "command too long\n");
        }
        else if (strlen(cmd_line) > MAX_CMD_LEN) { /*Line too long check 2*/
	    fprintf(stderr, "command too long\n");
        }
        else if (strcmp(cmd_line, "") == 0) { /*Null command check*/
	    fprintf(stderr, "invalid null command\n");
        }
        else {
	    if (validLine(cmd_line)) {
                parseLine(cmd_line); /*read and execute command line*/
                fflush(stdout);
	    }
	}
	memset(cmd_line, '\0', MAX_CMD_LEN + 1);
    }

    free(cmd_line);	

    return 0;
}
