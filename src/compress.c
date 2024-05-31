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
		DIR *dirp;
		
		struct dirent *dp;
		char dir_orig[150];	// Directory of files to compute
		char dir_dest[150];	// Directory were the computed files go
		char dir_aux[120];	// Auxiliar directory
		char huffman[256];	// File path were we store de huffman code

		// We canonicalize the paths
		if (comp) {
			setComPaths(directories[d], &dir_orig, &dir_aux);
			sprintf(dir_dest, "%s/files", dir_aux);
		}
		else {	// (!comp)
			char aux[120];
			setDecPaths(directories[d], &aux, &dir_aux);
			sprintf(dir_dest, "%s", dir_aux);
			sprintf(dir_orig, "%s/files", aux);
		}
		
		dirp = opendir(dir_orig);
		sprintf(huffman, "%s/huffman", dir_aux);
		printf("%s\n", dir_dest);

		// We check there are no errors
		if (!dirp) {
			print_error(ERR_OPDIR);
		}
		// We compress each file of the directory
		else {
			// We create the directory used to save the de/compressed files
			int ret;

			if (comp) {
				mkdir(dir_aux, 0755);
				ret = mkdir(dir_dest, 0755);
			}
			else {
				ret = mkdir(dir_dest, 0775);
				int count = 0;

				// We check the directory doesn't exist
				while (ret < 0 && errno == EEXIST) {
					sprintf(dir_dest, "%s(%d)", dir_aux, count);
					ret = mkdir(dir_dest, 0775);
					count++;
				}
			}

			// We check there are no errors
			if (ret < 0 && errno != EEXIST) {
				print_error(ERR_MKDIR);
			}
			// We get the number of files
			else {
				int dir_size = 0;
				while ((dp = readdir(dirp))) {
					if (dp->d_type == DT_REG) { // We compress only the files
						dir_size++;
					}
				}
		
				ret = closedir(dirp);
				if (ret < 0) {
					print_error(ERR_CLSDIR);
				}
				printf("Compressing %d files...\n", dir_size);

				dirp = opendir(dir_orig);

				int n = HUFFMAN_S << 4 << 2; 
				unsigned long int *total_times = calloc(n, sizeof(long int));
				char* file_orig[dir_size];
				char* file_dest[dir_size];
				int counter = 0;
				
				for (int i = 0; i < dir_size; ++i) {
					file_orig[i] = (char*) malloc(sizeof(char)*256);
					file_dest[i] = (char*) malloc(sizeof(char)*256);
				}

				// We get the frequency of each word inside the directory
				while ((dp = readdir(dirp))) {	
					if (dp->d_type == DT_REG) { // We compress only the files
						if (comp) {	// We get the huffman code
							sprintf(file_orig[counter], "%s/%s", dir_orig, dp->d_name);
							sprintf(file_dest[counter], "%s/%s", dir_dest, dp->d_name);

							printf("Counting %s words\n", file_orig[counter]);

							unsigned long int *parc_times = count_words(HUFFMAN_S, file_orig[counter]);
							for (int i = 0; i < n; ++i) {
								total_times[i] += parc_times[i];
							}
							free(parc_times);
						}
						else {
							sprintf(file_orig[counter], "%s/%s", dir_orig, dp->d_name);
							sprintf(file_dest[counter], "%s/%s", dir_dest, dp->d_name);
						}
						++counter;
					}
				}

				if (dp != NULL && errno) { // Error
					for (int i = 0; i < dir_size; ++i) {
						free(file_orig[i]);
						free(file_dest[i]);
					}
					print_error(ERR_RDDIR);
				}
				
				for (int i = 0; i < dir_size; ++i) {
					if (comp) {
						printf("Compressing %s to %s...\n", file_orig[i], file_dest[i]);
						compress(file_orig[i], file_dest[i]);					
					}
					else {
						printf("Decompressing %s to %s...\n", file_orig[i], file_dest[i]);
						decompress(file_orig[i], file_dest[i]);					
					}
				}
				
				// Free data structures
				free(total_times);
				for (int i = 0; i < dir_size; ++i) {
					free(file_orig[i]);
					free(file_dest[i]);
				}
			}
		}

		int ret = closedir(dirp);
		if (ret < 0) {
			print_error(ERR_CLSDIR);
		}
	}
}
