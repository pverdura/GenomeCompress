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

char getNucl(unsigned int seq, int pos) {
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

	FILE *rd_fp = fopen(file, "r");
	if (rd_fp == NULL) {
		return -ERR_OPFILE;
	}

	FILE *wr_fp = fopen(dest, "wb");	
	if (wr_fp == NULL) {
		fclose(rd_fp);
		return -ERR_MKDIR;	
	}

	unsigned int i;					// Variable for iteration
	unsigned int comprData = 0;  	// Binary data converted to a char

	while ((size = fread(block, sizeof(char), sizeof(block), rd_fp)) > 0) {	
		unsigned char comprBlk [size/NUCLXCHAR];	// Compressed block
		//printf("size: %d\n", size);

		/*
		 * We convert the binary string to a compacted string:
		 *
		 * Each character is 8 bits long. Using 2 bits for each
		 * character, we can insert 4 nucleotides in one char
		 */
		for (i = 0; i < size;) {
			comprData <<= 2;	// We shift the bits
			
			switch (block[i]) {
				case 'A': comprData += 0; break;	// We add '00'
				case 'T': comprData += 1; break;	// We add '01'
				case 'G': comprData += 2; break;	// We add '10'
				case 'C': comprData += 3; break;	// We add '11'
				default: break;
			}

			++i;
			// We added 4 nucleotides to a character
			if (!(i % NUCLXCHAR)) {
				// We convert the ascii value to char
				comprBlk[i/NUCLXCHAR-1] = comprData;
				comprData = 0;
			}
		}

		fwrite(comprBlk, sizeof(char), ((int)size/4), wr_fp);
	}
	
	//We write the reminder of letters that must be read in the last character
	fprintf(wr_fp, "%c%d", comprData, i%NUCLXCHAR);

	fclose(rd_fp);
	fclose(wr_fp);

	if (size < 0) { // Error
		return -ERR_RDFILE;
	}

	return 0;
}

int decompress(char* file, char* dest) {
	FILE *rd_fp = fopen(file, "rb");
	if (rd_fp == NULL) {
		return -ERR_OPFILE;
	}

	FILE *wr_fp = fopen(dest, "wb");	
	if (wr_fp == NULL) {
		fclose(rd_fp);
		return -ERR_MKDIR;	
	}

	int size;
	int end = 0;
	int first = 1;
	unsigned char lastBytes[2] = {'0', '0'};

	// We read all the compressed file
	while (!end) {
		unsigned char block[128];

		// We read size*NUCLXCHAR nucleotides
		size = fread(block, sizeof(char), sizeof(block), rd_fp);

		if (size < 0) { // Error
			end = 1;	
		}
		else if (size == 0) { // The previous block has the end of file
			int reminder = lastBytes[1] - '0'; // How many nucleotides we have to read
			unsigned char extra[reminder];

			for (int i = 0; i < reminder; ++i) {
				int pos = NUCLXCHAR-reminder+i;
				extra[i] = getNucl((unsigned int) lastBytes[0], pos);
			}

			fwrite(extra, sizeof(char), reminder, wr_fp);
			//write(1, extra, reminder);
			end = 1;
		}
		else if (size == 1) { // The current and previous block have the end of file
			int reminder = block[0] - '0'; // How many nucleotides we have to read
			char extra[NUCLXCHAR+reminder];
			
			// Include data that wasn't part of the end of file
			for (int i = 0; i < NUCLXCHAR; ++i) {
				extra[i] = getNucl((unsigned int) lastBytes[0], i);
			}

			// Previous data that was part of the end of file
			for (int i = 0; i < reminder; ++i) {
				int pos = NUCLXCHAR-reminder+i;
				extra[NUCLXCHAR+i] = getNucl((unsigned int) lastBytes[1], pos); 
			}

			fwrite(extra, sizeof(char), NUCLXCHAR+reminder, wr_fp);
			end = 1;
		}
		else { // The previous block has not the end of file
			char extra[2*NUCLXCHAR];

			if (first) {
				first = 0;
			}
			else {
				for (int i = 0; i < 2; ++i) {
					for (int j = 0; j < NUCLXCHAR; ++j) {
						extra[i*NUCLXCHAR+j] = getNucl((unsigned int)lastBytes[i], j);
					}
				}
				fwrite(extra, sizeof(char), 2*NUCLXCHAR, wr_fp);
			}

			char decomprBlk[NUCLXCHAR*size]; // String with decompressed data

			/*
			 * We convert the compacted string to a char string:
			 *
			 * Each character is 8 bits long. Using 2 bits for each
			 * character, we can insert 4 nucleotides in one char
			 */
			for (int i = 0; i < size-2; ++i) {
				// We read the NUCLXCHAR nucleotides
				for (int j = 0; j < NUCLXCHAR; ++j) {
					decomprBlk[i*NUCLXCHAR+j] = getNucl((unsigned int)block[i], j);
				}
			}

			lastBytes[0] = block[size-2];
			lastBytes[1] = block[size-1];

			fwrite(decomprBlk, sizeof(char), (size-2)*4, wr_fp);
		}
	}

	fclose(rd_fp);
	fclose(wr_fp);

	if (size < 0) { // Error
		return -ERR_RDFILE;
	}

	return 0;
}

