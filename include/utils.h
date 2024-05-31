/*
 * utils.h
 */
#ifndef __UTILS_H
#define __UTILS_H

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include "define.h"

/*
 * Prints how must be used the executable
 */
void usage(char* exec);

/*
 * Prints the identified error
 */
void print_error(int err);

/*
 * Canonicalizes the origin and destiny path for compressing
 */
void setComPaths(char* dir, void* dir_orig, void* dir_dest);

/*
 * Canonicalizes the origin and destiny path for decompressing
 */
void setDecPaths(char* dir, void* dir_orig, void* dir_dest);

/*
 * Compresses the file at the path 'file' to the foleder 'dest'
 */
int compressLZ78(char* file, char *dest);

/*
 * Compresses the file at the path 'file' to the foleder 'dest'
 */
int compress(char* file, char *dest);
//void* compress(void* args);

/*
 * Decompresses the file at the path 'file' to the foleder 'dest'
 */
int decompress(char* file, char *dest);
//void* decompress(void* args);

unsigned long int* count_words(int lenght, char* file);

void BurrowsWheeler(char* file, char *dest);


#endif // __UTILS_H
