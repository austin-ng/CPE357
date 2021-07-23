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

#define BLOCKSIZE 512 /* size of a data block*/
#define INIT_ARGS_LEN 3 /*Number of args up to (and including) tar filename*/

int checkifRequested(char* buf, char* files[], int numfiles) {
    int i;

    if (numfiles == -1) return 1; /*-1 numfiles means whole tar can print*/

    for (i = 3; i < (numfiles + INIT_ARGS_LEN); i++) {
	if (strstr(buf, files[i])) {
	    return 1;
	}
    }

    return 0;
}

int checkifOK(int fd) {
    int h_chksum_int;
    char h_chksum[8];

    int mode_to_chksum = 48;
    int back_to_mode = -56;

    lseek(fd, mode_to_chksum, SEEK_CUR);
    if (read(fd, h_chksum, 8) < 0) {
	h_chksum_int = 0;
    }
    else {
        h_chksum_int = (int) strtol(h_chksum, NULL, 8);
        h_chksum_int = h_chksum_int > 0;
        lseek(fd, back_to_mode, SEEK_CUR);
    }
    return h_chksum_int;
}

void printTarEntry(int fd, char* files[], int numfiles, int v) {
    char buf[101]; /* 100 is the size of name field */
    char sizebuf[12]; /* 8 is the size of the size field*/
    int cursize; /*size of file to determine how many blocks to skip*/

    char modebuf[8]; /* 8 is the size of the mode field */
    mode_t perms; /* permissions of the file */
    char typebuf; /*To fill in first part of perms*/

    char usrbuf[33]; /* buffer for owners */
    usrbuf[32] = '\0'; /* null terminate */
    char grpbuf[33];
    grpbuf[32] = '\0';
    char oblock[18]; /* fully printed owner name */
    memset(oblock, '\0', 18);

    int sz; /* size to be printed */

    int ok_to_print; /* bool to allow for printing of current file */ 

    char timebuf[13]; /* space for time block*/
    timebuf[12] = '\0';
    time_t seconds; /* time in seconds */
    struct tm *realtime; /* time converted from seconds */

    char prefixbuf[156]; /* buffer for prefix */
    prefixbuf[155] = '\0';

    char fullpath[257]; /* buffer for full name of file */
    fullpath[256] = '\0';

    if (read(fd, buf, 100) < 0) { /* 100 is the size of name field */
	return;
    }

    buf[100] = '\0'; /* null terminate in case name overflow */
    while (buf[0]) { /*while the block is not full of zero bytes */
    	if (checkifOK(fd)) {
            read(fd, modebuf, 8); /* read the mode into modebuf */
            perms = (mode_t) strtol(modebuf, NULL, 8); /* get modes as octal */



            lseek(fd, 16, SEEK_CUR); /* lseek 16 bytes to size field*/
            read(fd, sizebuf, 12); /* read the size field bytes */
            cursize = (int) strtol(sizebuf, NULL, 8); /* get size as octal */
            sz = cursize;
            if (cursize > 0) { /* if there are contents in file*/
                cursize = (cursize / BLOCKSIZE + 1); /* */
            }

            read(fd, timebuf, 12); /* read time field into buffer*/
            seconds = (time_t) strtol(timebuf, NULL, 8);
            realtime = (struct tm *) localtime(&seconds);
        

            lseek(fd, 117, SEEK_CUR); /* lseek to ownership names */
            read(fd, usrbuf, 32); /* read usr field into  buffer */
            read(fd, grpbuf, 32); /* read grp field into buffer */
            sprintf(oblock, "%s/%s", usrbuf, grpbuf); /*put into usr/grp frmt*/
            oblock[17] = '\0'; /* null terminate owner block */

            lseek(fd, -173, SEEK_CUR); /* To go to typeflag field */
            read(fd, &typebuf, 1);

            lseek(fd, 188, SEEK_CUR); /* To go to prefix field */
            read(fd, prefixbuf, 155);
            if (prefixbuf[0]) {
		sprintf(fullpath, "%s/%s", prefixbuf, buf);
	    }
	    else {
		strcpy(fullpath, buf);
	    }
            ok_to_print = checkifRequested(fullpath, files, numfiles);

            if (ok_to_print) printf("\n");

            if (v && ok_to_print) { /* if 'v' was entered in ctxfS argument */
                if (typebuf == '5') {
		    printf("d");
		}
		else if (typebuf == '2') {
		    printf("l");
		}
		else {
		    printf("-");
		}
                printf( (perms & S_IRUSR) ? "r" : "-");
                printf( (perms & S_IWUSR) ? "w" : "-");
                printf( (perms & S_IXUSR) ? "x" : "-");
                printf( (perms & S_IRGRP) ? "r" : "-");
                printf( (perms & S_IWGRP) ? "w" : "-");
                printf( (perms & S_IXGRP) ? "x" : "-");
                printf( (perms & S_IROTH) ? "r" : "-");
                printf( (perms & S_IWOTH) ? "w" : "-");
                printf( (perms & S_IXOTH) ? "x" : "-");
                printf(" ");
                printf("%17s ", oblock);
                printf("%8d ", sz);
                printf("%4d-%02d-%02d %02d:%02d ",
                realtime->tm_year + 1900, realtime->tm_mon + 1,
                   realtime->tm_mday, realtime->tm_hour, realtime->tm_min);
            }

            if (ok_to_print) {
                printf("%s", fullpath); /* print the name */
            }
            lseek(fd, 12, SEEK_CUR); /* the remaining bytes in block */
            lseek(fd, BLOCKSIZE * cursize, SEEK_CUR);

            read(fd, buf, 100);
            buf[100] = '\0';
        }
        else {
	    fprintf(stderr, "Malformed header found. Bailing.\n");
	    exit(EXIT_FAILURE);
	}
    }
    printf("\n");
}

void printTar(char* filename, char* files_to_add[], int num_args, int v) {
    int tar_fd;

    if ((tar_fd = open(filename, O_RDONLY)) < 0) {
        perror(filename);
        exit(EXIT_FAILURE);
    }

    if (num_args == INIT_ARGS_LEN) {
	printTarEntry(tar_fd, NULL, -1, v); /*-1 is an ok to print whole tar*/
    }
    else {
	printTarEntry(tar_fd, files_to_add, num_args - INIT_ARGS_LEN, v);
    }

    close(tar_fd);
}
