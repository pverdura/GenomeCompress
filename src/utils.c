#include "utils.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

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
	if (nBin == 0) {
		return 'A';
	} else if (nBin == 1) {
		return 'T';
	} else if (nBin == 2) {
		return 'G';
	} else { // nBin == 3
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
		return -ERR_OPFILE;
	}

	unsigned int i;
	unsigned int comprData = 0;
	int bitsFilled = 0;

	while ((size = read(rd_fd, block, sizeof(block))) > 0) {
		char comprBlk[(size + 3) / 4];  // Ensure it can hold the compressed data

		memset(comprBlk, 0, sizeof(comprBlk));  // Initialize compressed block

		for (i = 0; i < size; ++i) {
			comprData <<= 2;  // Shift bits

			switch (block[i]) {
				case 'A': comprData += 0; break;
				case 'T': comprData += 1; break;
				case 'G': comprData += 2; break;
				case 'C': comprData += 3; break;
				default: break; // Handle unexpected characters if necessary
			}

			if (++bitsFilled == 4) {
				comprBlk[(i + 1) / 4 - 1] = (char)comprData;
				comprData = 0;
				bitsFilled = 0;
			}
		}

		if (bitsFilled != 0) {
			comprData <<= (2 * (4 - bitsFilled));  // Align remaining bits
			comprBlk[i / 4] = (char)comprData;
		}

		fwrite(comprBlk, 1, (i + 3) / 4, wr_fp);
	}

	if (bitsFilled != 0) {
		fwrite(&comprData, 1, 1, wr_fp);  // Write the remaining bits
		fwrite(&bitsFilled, 1, 1, wr_fp); // Write the number of valid bits in the last byte
	}

	close(rd_fd);
	fclose(wr_fp);

	if (size < 0) {
		return -ERR_RDFILE;
	}

	return 0;
}

int decompress(char* file, char* dest) {
    FILE *rd_fp = fopen(file, "rb");
    if (rd_fp == NULL) {
        return -ERR_OPFILE;
    }

    int wr_fd = creat(dest, 00644);
    if (wr_fd < 0) {
        fclose(rd_fp);
        return -ERR_OPFILE;
    }

    unsigned char block[128];
    int size;
    int end = 0;

    // Variables to handle the last byte and remaining bits
    unsigned char lastByte = 0;
    int bitsLeft = 0;
    long int fileSize = 0;

    // Determine file size
    fseek(rd_fp, 0, SEEK_END);
    fileSize = ftell(rd_fp);
    fseek(rd_fp, 0, SEEK_SET);

    while (!end) {
        size = fread(block, 1, sizeof(block), rd_fp);

        if (size <= 0) {
            end = 1;
        } else {
            // If this is the last block, adjust size to account for the remaining bytes
            long int currentPos = ftell(rd_fp);
            if (currentPos == fileSize) {
                size -= 2;  // The last two bytes are the lastByte and bitsLeft
                lastByte = block[size];
                bitsLeft = block[size + 1];
            }

            char decomprBlk[4 * size];
            int decomprIdx = 0;

            for (int i = 0; i < size; ++i) {
                for (int j = 0; j < 4; ++j) {
                    unsigned char nucleotide = (block[i] >> (6 - 2 * j)) & 0x03;

                    switch (nucleotide) {
                        case 0: decomprBlk[decomprIdx++] = 'A'; break;
                        case 1: decomprBlk[decomprIdx++] = 'T'; break;
                        case 2: decomprBlk[decomprIdx++] = 'G'; break;
                        case 3: decomprBlk[decomprIdx++] = 'C'; break;
                        default: break;
                    }
                }
            }

            write(wr_fd, decomprBlk, decomprIdx);
        }
    }

    // Handle the remaining bits in the last byte if any
    if (bitsLeft > 0) {
        char decomprBlk[4];
        int decomprIdx = 0;
        for (int j = 0; j < bitsLeft; ++j) {
            unsigned char nucleotide = (lastByte >> (6 - 2 * j)) & 0x03;

            switch (nucleotide) {
                case 0: decomprBlk[decomprIdx++] = 'A'; break;
                case 1: decomprBlk[decomprIdx++] = 'T'; break;
                case 2: decomprBlk[decomprIdx++] = 'G'; break;
                case 3: decomprBlk[decomprIdx++] = 'C'; break;
                default: break;
            }
        }
        write(wr_fd, decomprBlk, decomprIdx);
    }

    fclose(rd_fp);
    close(wr_fd);

    if (size < 0) {
        return -ERR_RDFILE;
    }

    return 0;
}
