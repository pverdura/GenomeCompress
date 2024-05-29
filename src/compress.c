/*
 * compress.c - Main code for compressing and decompressig files
 *				that contain nucleotide sequences.
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

	int comp = 1; // By default compresses
	char *directories[argc-1]; // Array for saving all paths
	int counter = 0; // To keep track the number of read directories

	// We read the arguments
	for (int i = 1; i < argc; ++i) {
		if ((strlen(argv[i]) == 2) && (argv[i][0] == '-') && (argv[i][1] == 'c') && (argv[i][2] == '\0')) {
			comp = 1;
		}
		else if ((strlen(argv[i]) == 2) && (argv[i][0] == '-') && (argv[i][1] == 'x') && (argv[i][2] == '\0')) {
			comp = 0;
		}
		else { // File to de/compress
			directories[counter] = argv[i];
			counter++;
		}
	}
	
	// We go though all directories
	for (int d = 0; d < counter; ++d) {
		DIR *dirp = opendir(directories[d]);

		if (!dirp) { // Error
			print_error(ERR_OPDIR);
		}
		else { // We compress each file of the directory
			struct dirent *dp;
			char dir_orig[150];
			char dir_aux[120];
			char dir_dest[150];

			// We canonicalize the paths
			if (comp) {
				setComPaths(directories[d], &dir_orig, &dir_dest);
			}
			else {	// (!comp)
				setDecPaths(directories[d], &dir_orig, &dir_aux);
				sprintf(dir_dest, "%s", dir_aux);
			}
			// We create the directory used to save the de/compressed files
			int ret = mkdir(dir_dest, 0775);

			if (!comp) {
				int count = 0;

				// We check the directory doesn't exist
				while (ret < 0 && errno == EEXIST) {
					sprintf(dir_dest, "%s(%d)", dir_aux, count);
					ret = mkdir(dir_dest, 0775);
					count++;
				}
			}

			if (ret < 0 && (errno != EEXIST || !comp)) {
				print_error(ERR_MKDIR);
			}
			else {
				while ((dp = readdir(dirp))) {	
					if (dp->d_type == DT_REG) { // We compress only the files
						char file_orig[512];
						char file_dest[512];

						sprintf(file_orig, "%s/%s", dir_orig, dp->d_name);
						sprintf(file_dest, "%s/%s", dir_dest, dp->d_name);

						if (comp) {
							printf("Compressing %s to %s...\n", file_orig, file_dest);
					
							// Compress the file
							ret = compress(file_orig, file_dest); 
						}
						else {	// (!comp)
							printf("Decompressing %s to %s...\n", file_orig, file_dest);
					
							// Decompress the file
							ret = decompress(file_orig, file_dest); 
						}
						
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
