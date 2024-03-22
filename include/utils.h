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
 * Compresses the file at the path 'file' to the foleder 'dest'
 */
int compress(char* file, char *dest);

#endif // __UTILS_H
