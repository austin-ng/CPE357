#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define TABLE_SIZE 10

/*Made by Kelechi Igwe*/

typedef struct entry {
    char* word;
    int count;
} entry;

entry* hasharray;
int cur_size = 0; /*Keeps track of number of words in table*/
int cur_max_size = TABLE_SIZE; /*Keeps track of allocated size of table*/

int hashcode(char* word) {
    int hash_prime = 5; /*Starts with prime number to increase randomness*/
    int hash_sum = 0;
    int i;

    for (i = 0; i < strlen(word);) {
	hash_sum += hash_prime*i + word[i];
	i++;
    }
    if (hash_sum < cur_max_size) {
	hash_sum += cur_max_size;
    }
    return hash_sum % cur_max_size;
}

void createArray() {
    int i;
    hasharray = (entry*) malloc(TABLE_SIZE * (sizeof(entry) + sizeof(int)));

    for (i = 0; i < TABLE_SIZE;) {
	hasharray[i].word = "";
	hasharray[i].count = 0;
	i++;
    }
}

void resize() {
    int i;
    
    cur_max_size += TABLE_SIZE;

    hasharray = realloc(hasharray, cur_max_size *
    (sizeof(entry) + sizeof(int)));

    for (i = cur_size; i < cur_max_size;) {
	hasharray[i].word = "";
	hasharray[i].count = 0;
	i++;
    }
}    

void add_entry(char* ent) {
    int pos = hashcode(ent);
    int posfound = 0;
    entry new_entry;
 
    if (cur_size == cur_max_size) {
	resize();
    }
    while (!posfound) {
	if (hasharray[pos].count != 0) {
	    if (strcmp(hasharray[pos].word, ent) == 0) {
		hasharray[pos].count += 1;
		posfound = 2;
	    }
	    else {
		pos += 1;
		if (pos == cur_max_size) {
		    pos = 0;
		}
		else if (cur_size == cur_max_size) {
		    cur_max_size += TABLE_SIZE;
		}
	    }
	}
	else {
	    posfound = 1;
	}
    }
    
    if (posfound != 2) {
        new_entry.word = ent;
        new_entry.count = 1;

        hasharray[pos] = new_entry; 
	cur_size++;
    }
}

int table_length() {
    return cur_size;
}
