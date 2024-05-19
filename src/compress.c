/*
 * compress.c - Main code for compressing files that contain
 *				compressed nucleotide sequences.
 *
 * Authors: Paula Chen Guzman, Pol Verdura Mejías
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
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
			print_error(ERR_OPDIR);
		}
		else { // We compress each file of the directory
			struct dirent *dp;
			char dir_orig[150];
			char dir_dest[150];

			// We canonicalize the paths
			setComPaths(argv[d], &dir_orig, &dir_dest);

			// We create the directory used to save the compressed files
			int ret = mkdir(dir_dest, 0775);

			if (ret < 0 && errno != EEXIST) {
				print_error(ERR_MKDIR);
			}
			else {
				while ((dp = readdir(dirp))) {	
					if (dp->d_type == DT_REG) { // We compress only the files
						char file_orig[512];
						char file_dest[512];

						sprintf(file_orig, "%s/%s", dir_orig, dp->d_name);
						sprintf(file_dest, "%s/%s", dir_dest, dp->d_name);

						printf("Compressing %s to %s...\n", file_orig, file_dest);
					
						// Decompress the file
						ret = compress(file_orig, file_dest); 
						
						if (ret < 0) {
							print_error(-ret);
							break;
						}
					}
				}
	
				if (dp != NULL && errno) { // Error
					print_error(ERR_RDDIR);
				}
			}
		}

		int ret = closedir(dirp);
		if (ret < 0) {
			print_error(ERR_CLSDIR);
		}
	}
}
