/*
 * utils.c
 */
#include "utils.h"

void usage(char* exec) {
	char buff[128];
	int n = sprintf(buff, "Usage: %s <directory> ...\n  Where <directory> is the path to the directory that contains\n  all the nucleotide files.\n", exec);
	
	write(1, buff, n);
}

void print_error(int err) {
	switch (err) {
		case ERR_OPDIR:
			perror("opendir()");
			break;
		case ERR_CLSDIR:
			perror("closedir()");
			break;
		case ERR_RDDIR:
			perror("readdir()");
			break;
		case ERR_MKDIR:
			perror("mkdir()");
			break;
		case ERR_OPFILE:
			perror("open()");
			break;
		case ERR_RDFILE:
			perror("read()");
			break;
		case ERR_WRFILE:
			perror("write()");
			break;
	}	
}

void setComPaths(char* dir, void* dir_orig, void* dir_dest) {
	int n = strlen(dir);

	if (dir[n-1] == '/') {
		dir[n-1] = '\0';
	}
	
	sprintf(dir_orig, "%s", dir);
	sprintf(dir_dest, "%s.gco", dir);
}

void setDecPaths(char* dir, void* dir_orig, void* dir_dest) {
	int n = strlen(dir);

	if (dir[n-1] == '/') {
		dir[n-1] = '\0';
	}

	sprintf(dir_orig, "%s", dir);
	dir[n-5] = '\0'; // We remove the substring ".gco"
	sprintf(dir_dest, "%s", dir);
}

int compress(char* file, char* dest) {
	char block[128];
	int size;

	int rd_fd = open(file, O_RDONLY);
	if (rd_fd < 0) {
		return -ERR_OPFILE;
	}

	int wr_fd = creat(dest, 00644);	
	if (wr_fd < 0) {
		close(rd_fd);
		return -ERR_MKDIR;	
	}

	while ((size = read(rd_fd, block, sizeof(block))) > 0) {	
		int comprData = 0;	// Binary data converted to a char
		char comprBlk[size/4];	// Compressed block
	
		/*
		 * We convert the binary string to a compacted string:
		 *
		 * Each character is 8 bits long. Using 2 bits for each
		 * character, we can insert 4 nucleotides in one char
		 */
		for (int i = 0; i < size;) {
			comprData = comprData << 2;	// We shift the bits
			
			if (block[i] == 'A') {
				comprData += 0;	// We add '00'
			}
			else if (block[i] == 'T') {
				comprData += 1;	// We add '01'
			}
			else if (block[i] == 'G') {
				comprData += 2;	// We add '10'
			}
			else {	// block[i] == 'C'
				comprData += 3;	// We add '11'
			}

			++i;
			// We added 4 nucleotides to a character
			if (!(i % 4)) {
				comprBlk[i/4] = comprData + ' ';
				comprData = 0;
			}
		}

		write(wr_fd, comprBlk, size/4);	
	}
	
	close(rd_fd);
	close(wr_fd);

	if (size < 0) { // Error
		return -ERR_RDFILE;
	}

	return 0;
}
