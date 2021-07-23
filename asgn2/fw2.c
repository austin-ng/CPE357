#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "htable2.h"

#define MEM_CHUNK 80

char *read_file(FILE *file);
int sort_htable(const void *a, const void *b);

int main(int argc, char *argv[]) {
    int n = 10;
    int i;
    int j;
    int top_list_length;
    int file_start;
    FILE *fp;
    char *word;

    createArray();


    if (argc > 1) {

        if (strcmp(argv[1], "-n") == 0) {
            file_start = 3;
            if (argc == 2) {
                printf("usage: fw [-n num ] [file 1 [ file 2 ...]\n");
                exit(EXIT_FAILURE);
            }
            else {
                if (isdigit(*argv[2])) {
                    n = atoi(argv[2]);	    
                }
                else {
                    printf("usage: fw [-n num] [ file 1 [ file 2 ...] ]\n");
                    exit(EXIT_FAILURE);
                }
            }
        }
        else {
            file_start = 1;
        }


        for (i = file_start; i < argc; i++) {
            fp = fopen(argv[i], "r");
            if (fp == NULL) {
                printf("%s: %s\n", argv[i], strerror(errno));
            }
            else {
                word = read_file(fp);
                while (strcmp("-1", word) != 0) {
                    if (word[0] != '\0') {
                        add_entry(word);
                    }
                    word = read_file(fp);
                }
                fclose(fp);
            } 
        }
    }

    else {
        word = read_file(stdin);
        if (word[0]) {
            while (strcmp("-1", word) != 0)
                if (word[0] != '\0') {
                    add_entry(word);
                }
                word = read_file(stdin);
        }
        else {
            perror("Error: ");
            exit(EXIT_FAILURE);
        }
    }


    

    

    qsort(hasharray, cur_max_size, sizeof(entry),
          sort_htable);

    printf("The top %d word%s (out of %d) %s:\n", n, (n == 1 ? "" : "s"),
            cur_size, (n == 1 ? "is" : "are"));
    
    if (n > cur_max_size) {
	top_list_length = cur_max_size;
    }
    else {
	top_list_length = n;
    }

    for (j = 0; j < top_list_length; j++) {
	if (strcmp(hasharray[j].word, "") != 0) {
	    printf("%9d %s\n", hasharray[j].count, hasharray[j].word);
	}
    }

    free(hasharray);

    return 0;
}

int sort_htable (const void* a, const void* b) {
    entry *entry_a = (entry *) a;
    entry *entry_b = (entry *) b;
    if (entry_a->count == entry_b->count) {
	return strcmp(entry_b->word, entry_a->word);
    }
    else {
        return (entry_b->count - entry_a->count);
    }
}

char *read_file(FILE *file) {
    char *strptr;
    int ch;
    int counter = 0;
    int reallocounter = 1;
    int i = 0;
    int done = 0;

    strptr = (char *) malloc(MEM_CHUNK);
    while (done == 0) {
        ch = fgetc(file);
        if (isalpha((char) ch) == 0) {
            if (ch == EOF && strptr[0] == '\0') {
                return "-1";
            }
            done = 1;
        }
        else {
            counter++;
            if (counter >= MEM_CHUNK * reallocounter) {
                reallocounter++;
                strptr = realloc(strptr, MEM_CHUNK * reallocounter);
            }
            strptr[i++] = (char) tolower(ch);
        }
    }
    strptr[i] = (char) '\0';
    /*strptr = realloc(strptr, i + 1);*/
    return strptr;
}


