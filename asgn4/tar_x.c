#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <math.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <arpa/inet.h>
#include <stdint.h>

#define BLOCKSIZE 512

void extractTar(char * filenames[], int numargs, int fd, int vpresent,
		int spresent) {
    char buf[101]; /* name of file */
    char modebuf[8]; /* mode of file */
    mode_t mode; 
    char sizebuf[12]; /* 12 is the size of the size field*/
    int cursize; /* size of file to determine how many blocks to skip*/
    int actualsize;
    char filetype; /* buffer for file type*/
    char linkname[101]; /* name if file is link */
    linkname[100] = '\0'; /* null terminate if full*/
    char magic[6]; /* for strict checking */
 //   char version[2];
    char prestr[156]; /* prefix string */
    prestr[155] = '\0';

    char fullname[257]; /* full name of the file*/
    memset(fullname, '\0', 257);

    int newfile; /* fd for new created file */
    char * filecontents; /* contents of the file */

    int ok_to_write;
    int i;

    read(fd, buf, 100);
    buf[100] = '\0';
    while (buf[0]) {
        read(fd, modebuf, 8); /* read mode field*/
        mode = (mode_t) strtol(modebuf, NULL, 8); /* get mode*/
        lseek(fd, 16, SEEK_CUR);
        read(fd, sizebuf, 12); /* read the size field byte */
        cursize = (int) strtol(sizebuf, NULL, 8);
	actualsize = cursize;
        if (cursize > 0) { /* if there are contents in file */ 
            cursize = cursize / BLOCKSIZE + 1; /* get amount of blocks */
        }
        lseek(fd, 20, SEEK_CUR); /* skip chksum, irrelevant to write */
        read(fd, &filetype, 1); /* set type of file (1 byte) */
        read(fd, linkname, 100); /* if link, save link name */
        read(fd, magic, 6);
        lseek(fd, 2, SEEK_CUR); /* skip version, irrelevant to write*/
        lseek(fd, 80, SEEK_CUR); /* skip dev numbers, irrelevant */
        read(fd, prestr, 155); /* read prefix string */
        lseek(fd, 12, SEEK_CUR);


        

        /* begin creating new file */
        memset(fullname, '\0', 257);
        if (prestr[0]) {
	    sprintf(fullname, "%s/%s", prestr, buf);
	}
	else {
	    strcpy(fullname, buf);
	}
         /* concatenate prefix and filename or just use filename */

        ok_to_write = 0;
	if (numargs > 3) {
	    i = 3;
	    while (!ok_to_write) {
	        if (strstr(fullname, filenames[i])) {
		    if (filetype == '5') {
			ok_to_write = 0;
			break;
		    }
		    ok_to_write = 1;
	        }
	        else {
 		    i++;
		    if (i == numargs) {
		        ok_to_write = 0;
		        break;
		    }
	        }
	    }	
        }
	else {
	    ok_to_write = 1;
        }

        if (ok_to_write) {
            if (filetype == '2') {
	        symlink(linkname, fullname);
	    }
            else if (filetype == '5') {
		mkdir(fullname, mode);
            } 
	    else { 
	        /* if its a regular (or alternate) file */
		char temp_fullname[257];
                memset(temp_fullname, '\0', 257);
		strcpy(temp_fullname, fullname);
                while ((newfile = open(fullname, O_WRONLY | O_CREAT | O_TRUNC,
			mode)) < 0) {
		    char* before_end = strrchr(temp_fullname, '/');
                    int temp_len = strlen(temp_fullname);
		    int be_start = temp_len - strlen(before_end);
		    temp_fullname[be_start] = '\0';
		    if (mkdir(temp_fullname, 0777) >= 0) {
		        while (strcmp(temp_fullname, fullname) != 0) {
			    mkdir(temp_fullname, 0777);
			    int end = strlen(temp_fullname);
			    temp_fullname[end] = '/';
			}
		    }
		}
   
		/* create new file */
                filecontents = (char *) malloc(cursize * BLOCKSIZE);
                /*allocate memory for buffer*/
                read(fd, filecontents, actualsize); 
		/* write into buffer */
                write(newfile, filecontents, actualsize); /* write to file */
		lseek(fd, (cursize * BLOCKSIZE) - actualsize, SEEK_CUR);
                free(filecontents);
                close(newfile);
	    }
	    if (vpresent) printf("%s\n", fullname);
	}
	else {
	    lseek(fd, cursize * BLOCKSIZE, SEEK_CUR);
	}
	memset(buf, '\0', 101);
        read(fd, buf, 100); /* read the next header name */
    }

    close(fd);

}
