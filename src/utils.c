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
		n--;
	}

	sprintf(dir_orig, "%s", dir);
	dir[n-4] = '\0'; // We remove the substring ".gco"
	sprintf(dir_dest, "%s", dir);
}

char getNucl(int seq, int pos) {
	unsigned int nBin = (seq / (1 << 2*(3-pos))) % 4; 
	//printf("nBin: %ud\n", nBin);

	if (nBin == 0) {
		return 'A';
	}
	else if (nBin == 1) {
		return 'T';
	}
	else if (nBin == 2) {
		return 'G';
	}
	else { // nBin == 3
		return 'C';
	}
}

int compress(char* file, char* dest) {
	char block[128];
	int size;

	int rd_fd = open(file, O_RDONLY);
	if (rd_fd < 0) {
		return -ERR_OPFILE;
	}

	FILE *wr_fp = fopen(dest, "wb");	
	if (wr_fp == NULL) {
		close(rd_fd);
		return -ERR_MKDIR;	
	}

	unsigned int i;					// Variable for iteration
	unsigned int comprData = 0;  	// Binary data converted to a char

	while ((size = read(rd_fd, block, sizeof(block))) > 0) {	
		char comprBlk [size/NUCLXCHAR];	// Compressed block

		/*
		 * We convert the binary string to a compacted string:
		 *
		 * Each character is 8 bits long. Using 2 bits for each
		 * character, we can insert 4 nucleotides in one char
		 */
		for (i = 0; i < size;) {
			comprData <<= 2;	// We shift the bits
			
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
			if (!(i % NUCLXCHAR)) {
				// We convert the ascii value to char
				comprBlk[i/NUCLXCHAR] = comprData;
				comprData = 0;
			}
		}

		fwrite(comprBlk, 1, size/NUCLXCHAR, wr_fp);
	}
	
	//We write the reminder of letters that must be read in the last character
	fprintf(wr_fp, "%u%d", comprData, i%NUCLXCHAR);

	close(rd_fd);
	fclose(wr_fp);

	if (size < 0) { // Error
		return -ERR_RDFILE;
	}

	return 0;
}

int decompress(char* file, char* dest) {
	char block[128];
	int size;

	FILE *rd_fp = fopen(file, "rb");
	if (rd_fp == NULL) {
		return -ERR_OPFILE;
	}

	int wr_fd = creat(dest, 00644);	
	if (wr_fd < 0) {
		fclose(rd_fp);
		return -ERR_MKDIR;	
	}

	// We read size*NUCLXCHAR nucleotides
	while ((size = fread(block, 1, sizeof(block), rd_fp)) > 0) {	
		char decomprBlk[NUCLXCHAR*size]; // String with decompressed data

		/*
		 * We convert the compacted string to a char string:
		 *
		 * Each character is 8 bits long. Using 2 bits for each
		 * character, we can insert 4 nucleotides in one char
		 */
		for (int i = 0; i < size; ++i) {
			unsigned int decomprChar = (unsigned int) block[i];

			// We read the NUCLXCHAR nucleotides
			for (int j = 0; j < NUCLXCHAR; j++) {
				decomprBlk[i*NUCLXCHAR+j] = getNucl(decomprChar, j);
			}	
		}

		write(wr_fd, decomprBlk, strlen(decomprBlk));
	}
	
	fclose(rd_fp);
	close(wr_fd);

	if (size < 0) { // Error
		return -ERR_RDFILE;
	}

	return 0;
}
