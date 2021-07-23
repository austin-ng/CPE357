#ifndef MYTARH
#define MYTARH

#include <stdio.h>

/*Defined in tar_c.c*/
extern void createTar(char* filename, char* files_to_add[], int num_args,
		      int isV, int isS);

/*Defined in tar_t.c*/
extern void printTar(char* filename, char* files_to_add[], int num_args,
		      int v);

/*Defined in tar_x.c*/
extern void extractTar(char* filenames[], int numargs, int fd, int vpresent,
		int spresent);

#endif
