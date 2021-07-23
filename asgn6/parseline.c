#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CMD_LEN 512 /*Maximum command line length*/
#define MAX_ARGS_PIPE 20 /*Maximum arguments in pipe*/
#define MAX_ARGS_CMD 20 /*Maximum arguments in command*/
#define PRINTS_PER_STAGE 13 /*Number of printf calls per stage*/
#define MAX_ERROR_MSG_LEN 30 /*Buffer size for error messages*/

int stage_number;

void printError(int a) {
    ;
}

void printStage(char* args[], int num_args, char* s_in,
                char* s_out) {
    /*Takes a commands's arguments (args), the number of arguments in args
     *(num_args), the stage in string (s_in) and stage out string (s_out)*/ 
    int i;
    char* border; /*Buffer to hold ---------*/

    border = malloc(9);
    border = "--------";
   
    printf("\n%s\nStage %d: %c%s%c\n%s\n", border, stage_number,
	   '"', args[0], '"', border); /*Creates stage header*/

    printf("%11s %s\n", "input:", s_in); /*Creates input line*/

    printf("%11s %s\n", "output:", s_out); /*Creates output line*/

    printf("%11s %d\n", "argc:", num_args); /*Creates argc line*/

    printf("%11s %c%s%c", "argv:", '"', args[1], '"');
    /*Creates argv line and adds first argument*/

    if (args[2]) {
        for (i = 2; i < num_args + 1; i++) { /*Add other arguments to argv*/
	    printf(",%c%s%c", '"', args[i], '"');
	}
    }

   stage_number++;
}

void readStage(char* command, int p_in, int p_out) {
    /*Takes an individual command (command), and booleans (p_in) and (p_out)
     *which are set if command is piped in and/or piped out*/ 
    char stage_in[MAX_CMD_LEN + 1];
    char stage_out[MAX_CMD_LEN + 1];
    int numargs;
    char* args[MAX_ARGS_CMD];
    char* new_arg; /*String for each argument*/
    int i;
    int new_arg_len;
    int isRdOut, isRdIn; /*Booleans based on if < or > is present*/
    int rd_out_count, rd_in_count; /*For error checks when multiple < or >*/

    /*initializing values*/
    numargs = 0;
    new_arg_len = 0;
    i = 0;
    isRdOut = isRdIn = 0;
    rd_in_count = rd_out_count = 0;

    memset(stage_in, '\0', MAX_CMD_LEN + 1);
    memset(stage_out, '\0', MAX_CMD_LEN + 1);

    new_arg = malloc(MAX_CMD_LEN + 1);

    /*Establishing stage's in and out (before checking if < or >)*/
    if (p_in) {
	sprintf(stage_in, "pipe from stage %d", stage_number - 1);
    }
    else {
	sprintf(stage_in, "original stdin");
    }
    if (p_out) {
	sprintf(stage_out, "pipe to stage %d", stage_number + 1);
    }
    else {
        sprintf(stage_out, "original stdout");
    }

    /*Parsing an individual command*/
    while (command[i] != '\0') {
        if (command[i] == ' ') { /*If space, either add arg to argv or skip*/
	    if (new_arg[0]) {
		new_arg[new_arg_len++] = '\0';
		if (numargs == 0) {
		    args[0] = command;
		}
		if (isRdIn || isRdOut) {
		/*If < or > present, sets stage_in and/or stage_out*/
		    if (isRdIn) {
			if (p_in) { /*Error checking for ambiguous input*/
			    if (!args[1]) {
				args[1] = "<";
			    }
			    fprintf(stderr, "%s: ", args[1]);
			    printError(7);
			}
			sprintf(stage_in, "%s", new_arg);
			isRdIn = 0;
		    }
		    if (isRdOut) {
			if (p_out) { /*Error checking for ambiguous output*/
			    if (!args[1]) {
				args[1] = ">";
			    }
			    fprintf(stderr, "%s: ", args[1]);
			    printError(8);
			}
			sprintf(stage_out, "%s", new_arg);
			isRdOut = 0;
		    }
		}
		else { /*If not after < or >, add arg to argv*/
		    if ((numargs + 1) > MAX_ARGS_CMD) {
			/*Error check for too many args*/
			fprintf(stderr, "%s: ", args[1]);
			printError(3);
		    }
		    args[++numargs] = new_arg;   
		}
		new_arg = malloc(MAX_CMD_LEN + 1);
		new_arg_len = 0;
	    }
	    i++;
	}
	else if (command[i] == '<') {
	    /*If there is no error and argument is loaded, add argument to
 	     *argv and continue through command. If command has error,
	     *deal with it properly based on booleans. Else, set isRdIn on.*/
	    if (!isRdIn && !rd_in_count && new_arg[0] && !p_in) {
		new_arg[new_arg_len++] = '\0';
	        if (numargs == 0) args[0] = command;
		if ((numargs + 1) > MAX_ARGS_CMD) {
		    fprintf(stderr, "%s: ", args[1]);
		    printError(3);
		}
		args[++numargs] = new_arg;
		new_arg = malloc(MAX_CMD_LEN + 1);
		new_arg_len = 0;
		isRdIn = 1;
		rd_in_count++;
	    }
	    else if (isRdIn || rd_in_count || p_in) {
		if (!args[1]) {
		    args[1] = "<";
		}
		fprintf(stderr, "%s: ", args[1]);
		if (p_in) printError(7);
		printError(5);
	    }
	    else {
		isRdIn = 1;
		rd_in_count++;
	    }
	    i++;
	}
	else if (command[i] == '>') {
	    /*Same as above condition, but for isRdOut*/
	    if (isRdIn && new_arg[0]) {
		sprintf(stage_in, "%s", new_arg);
		isRdIn = 0;
		new_arg = malloc(MAX_CMD_LEN + 1);
		new_arg_len = 0;
		isRdOut = 1;
		rd_out_count++;
	    }
	    else if (!isRdOut && !rd_out_count && new_arg[0]) {
		new_arg[new_arg_len++] = '\0';
		if (numargs == 0) args[0] = command;
		if ((numargs + 1) > MAX_ARGS_CMD) {
		    fprintf(stderr, "%s: ", args[1]);
		    printError(3);
		}
		args[++numargs] = new_arg;
		new_arg = malloc(MAX_CMD_LEN + 1);
		new_arg_len = 0;
		isRdOut = 1;
		rd_out_count++;
	    }
	    else if (isRdOut || rd_out_count) {
		if (!args[0]) {
		    args[1] = ">";
		}
		fprintf(stderr, "%s: ", args[1]);
		printError(6);
	    }
	    else {
	        isRdOut = 1;
	        rd_out_count++;
	    }
	    i++;
	}
	else { /*Just a regular character, continue on through command*/
            new_arg[new_arg_len++] = command[i++];
	}
    }
  
    if (new_arg[0]) {/*If last arg wasn't dealt with, deal with it*/
	new_arg[new_arg_len++] = '\0';
	if (numargs == 0) {
	    args[0] = command;
	}
	if (isRdIn || isRdOut) {
	    if (isRdIn) sprintf(stage_in, "%s", new_arg);
	    if (isRdOut) sprintf(stage_out, "%s", new_arg);
	}
	else {
	    if ((numargs + 1) > MAX_ARGS_CMD) {
		fprintf(stderr, "%s: ", args[1]);
		printError(3);
	    } 
            args[++numargs] = new_arg;
	}
    }
    
    if (args[0]) { /*If there are arguments*/
	if (numargs == 0) { /*Error check for empty command*/
	    printError(4);
	}
	printStage(args, numargs, stage_in, stage_out);
    }
}

void parseLine(char* line) {
    /*Reads the entire command (line) and reads each command, which is split
     *by either pipes or null terminator*/
    int i, j;
    char* new_cmd; /*Buffer for a command*/
    int isPipeOut, isPipeIn; /*Booleans representing if | is present*/

    /*initializing global variables*/
    stage_number = 0;

    /*initializing values*/
    i = 0;
    j = 0;
    isPipeOut = isPipeIn = 0;
    new_cmd = malloc(MAX_CMD_LEN + 1);
 
    /*Read through entire command*/
    while (line[i] != '\0') {
	if (line[i] == '|') {
	/*Decides if command is pipe in or out, and then reads the stage*/
	    new_cmd[++j] = '\0';
	    if (isPipeOut == 1) {
		isPipeIn = 1;
	    }
	    isPipeOut = 1;
	    readStage(new_cmd, isPipeIn, isPipeOut);
	    isPipeIn = 0;
	    free(new_cmd);
	    new_cmd = malloc(MAX_CMD_LEN + 1);
            memset(new_cmd, '\0', MAX_CMD_LEN + 1);
	    j = 0;
	    i++;
	}
	else { /*Adds character to command*/
	    new_cmd[j] = line[i];
            i++;
	    j++;
	}
    }

    if (new_cmd) { /*If last command wasn't dealt with, deal with it*/
	new_cmd[++j] = '\0';
	if (isPipeOut == 1) {
	    isPipeIn = 1;
	}
	isPipeOut = 0;
        readStage(new_cmd, isPipeIn, isPipeOut);
	printf("\n");
	free(new_cmd);
	isPipeIn = 0;
    }
}
