/*
 * main.c - Main code for compressing and decompressing files that contain
 *          nucleotide sequences.
 *
 * Authors: Paula Chen Guzman, Pol Verdura Mejías
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include "utils.h"
#include "define.h"

int main(int argc, char* argv[]) {
	// We check that the command is correct
	if (argc < 2) {
		usage(argv[0]);
		exit(0);
	}
	
	// We go though all directories
	for (int d = 1; d < argc; ++d) {
		DIR *dirp = opendir(argv[d]);

		if (!dirp) { // Error
			perror("opendir()");
		}
		else { // We compress each file of the directory
			struct dirent *dp;
				
			while ((dp = readdir(dirp))) {	
				// We compress only the files
				if (dp->d_type == DT_REG) {
					char file[516];
					char dest[516];
					int n = strlen(argv[d]);

					// We canonicalize the path
					if (argv[d][n-1] == '/') {
						sprintf(file, "%s%s", argv[d], dp->d_name);
						argv[d][n-1] = '_';
						sprintf(dest, "%scomp", argv[d]);
					} else {
						sprintf(file, "%s/%s", argv[d], dp->d_name);
						sprintf(dest, "%s_comp", argv[d]);
					}
					printf("Compressing %s...\n", file);
					
					// Compress the file
					int ret = compress(file, dest); 

					if (ret == -ERR_OPEN) {
						perror("open()");	
						break;
					} else if (ret == -ERR_READ) {
						perror("read()");
						break;
					}
				}
			}
	
			if (errno) { // Error
				perror("readdir()");
			}
		}
	}
}
