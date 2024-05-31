/*
 * utils.c
 */
#include "utils.h"

void usage(char* exec) {
	char buff[256];
	int n = sprintf(buff, "Usage: %s [-{c|x}] <directory> ...\n  Where <directory> is the path to the directory that contains\n  all the nucleotide files.\n\n  -c must be used if you want to compress the previous files.\n  -x must be used if you want to decompress the previous files.\n", exec);
	
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
		case ERR_COMPR:
			perror("compressLZ78()");
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

int getNuclVal(char nucl) {
	int val = -1;
	switch (nucl) {
		case 'A': val = 0; break;	// '00'
		case 'T': val = 1; break;	// '01'
		case 'G': val = 2; break;	// '10'
		case 'C': val = 3; break;	// '11'
		default: break;
	}
	return val;
}

int compressLZ78(char* file, char* dest) {
	char block[BLOCK_SIZE];
	char dictionary[DICT_SIZE];

	// We open the source file
	FILE *rd_fp = fopen(file, "r");
	if (rd_fp == NULL) {
		return -ERR_OPFILE;
	}

	// We open the destiny file
	FILE *wr_fp = fopen(dest, "wb");	
	if (wr_fp == NULL) {
		fclose(rd_fp);
		return -ERR_MKDIR;	
	}

	int size;						// Size of the read file block

	// We read the whole file
	while ((size = fread(block, sizeof(char), sizeof(block), rd_fp)) > 0) {	
		unsigned int block_iter = 0;	// Points to the first letter of the word
										// we will add to the dictionary
		unsigned int dict_size = 0;		// Dictionary size
		unsigned char code_block[DICT_SIZE*2];
		int code_idx = 0;

		// We read the whole block
		while (block_iter < size) {

			char current_word[BLOCK_SIZE];	// Block word we are adding in the dict.
			int found = 1;	// Variable for finding the current word in the dictionary
			int ws = 0;		// Word size to insert in the dictionary
			int wi = 0;		// Iterator for the words we want to add in the dictionary
			unsigned short pos = 0;			// Dictionary iterator
			unsigned short last_pos = 0;	// Last matching position

			// We keep elongating the current word until it's not in the dictionary
			while (found && (block_iter + ws < size)) {
				int ok = 1;	// Word is matching 
				found = 0;	// Reset the found flag
				wi = 0;		// Reset the word iterator
				current_word[ws] = block[block_iter+ws];	// Update the new word

				// We check if the current word is in the dictionary
				for (pos = 0; pos < dict_size; pos++, wi++) { //&& !found; pos++, wi++) {
					// ',' means the dictionary word ended
					if (ok && (dictionary[pos] == ',') && (wi == ws+1)) {
						found = 1;
						break;
					}

					// We reset the flags because we will read a new word
					if (dictionary[pos] == ',') {
						ok = 1;
						wi = -1;
					} 
					// Word doesn't match
					else if (ok && (dictionary[pos] != current_word[wi])) {
						ok = 0;
					}
					//printf("%c", dictionary[pos]);
				}
				//printf("\n");
				if (found)
					last_pos = pos;

				// We increase the word size
				ws++;
			}
			
			if (dict_size+ws > DICT_SIZE) {
				fclose(rd_fp);
				fclose(wr_fp);
				return -ERR_COMPR;
			}

			// We add the word in the dictionary
			for (int i = 0; i < ws; ++i) {	
				dictionary[dict_size+i] = current_word[i];
			}
			dict_size += ws+1;
			dictionary[dict_size-1] = ',';
			block_iter += ws;

			// We insert the word to the compressed code
			code_block[code_idx] = (last_pos >> 5);

			code_block[code_idx+1] = (last_pos % (1 << 5)) << 3;
			code_block[code_idx+1] += getNuclVal(current_word[ws-1]) << 1;
			code_block[code_idx+1] += (block_iter >= size);

			code_idx += 2;
		}
		fwrite(code_block, 1, code_idx, wr_fp);
	}

	fclose(rd_fp);
	fclose(wr_fp);

	return 0;
}

/*int decompressLZ78(char* file, char* dest) {
	
}*/

unsigned long int* count_words(int length, char* file) {
	int n = length << 4 << 2;
	unsigned long int* times = (unsigned long int*) calloc(n, sizeof(long int));

	FILE *rd_fp = fopen(file, "r");
	//FILE *rd_fp = fopen(thread_data->file_orig, "r");
	if (rd_fp == NULL) {
		print_error(ERR_OPFILE);
		exit(0);
	}

	char block[512*length];
	int size;

	while ((size = fread(block, sizeof(char), sizeof(block), rd_fp)) > 0) {	
		int i = 0;

		for (; i < size/length; i++) {
			int value = 0;

			for (int j = 0; j < length; j++) {
				value = (value << 2) + getNuclVal(block[i*length+j]);
			}
			times[value]++;
		}
/*
		if (i % length) {
			int j = (int)size/length;
			int value = 0;

			for (j *= length; j < size; j++) {
				value += getNuclVal(block[i*length+j]) << (j*2);
			}
			times[value]++;
		}
*/
	}
	
	fclose(rd_fp);

	if (size < 0) { // Error
		free(times);
		print_error(ERR_RDFILE);
		exit(0);
	}

	return times;
}

int compress(char* file, char* dest) {
//void* compress(void* args) {
	//thread_data_t *thread_data = (thread_data_t*) args;

	char block[128];
	int size;

	FILE *rd_fp = fopen(file, "r");
	//FILE *rd_fp = fopen(thread_data->file_orig, "r");
	if (rd_fp == NULL) {
		return -ERR_OPFILE;
	}

	FILE *wr_fp = fopen(dest, "wb");	
	//FILE *wr_fp = fopen(thread_data->file_dest, "wb");	
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
//void* decompress(void* args) {
	//thread_data_t *thread_data = (thread_data_t*) args;

	FILE *rd_fp = fopen(file, "rb");
	//FILE *rd_fp = fopen(thread_data->file_orig, "rb");
	if (rd_fp == NULL) {
		return -ERR_OPFILE;
	}

	FILE *wr_fp = fopen(dest, "wb");	
	//FILE *wr_fp = fopen(thread_data->file_dest, "wb");	
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

