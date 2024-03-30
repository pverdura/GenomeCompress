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
 * Canonicalizes the origin and destiny path, and returns the
 * string size of dir_dest
 */
int setPaths(char* dir, void* dir_orig, void* dir_dest);

/*
 * Compresses the file at the path 'file' to the foleder 'dest'
 */
int compress(char* file, char *dest);

#endif // __UTILS_H
