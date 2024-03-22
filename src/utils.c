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

int compress(char* file, char* dest) {
	char segment[128];
	int ret;

	int rd_fd = open(file, O_RDONLY);
	if (rd_fd < 0) {
		return -ERR_OPFILE;
	}

	int wr_fd = creat(dest, 644);	
	if (wr_fd < 0) {
		close(rd_fd);
		return -ERR_MKDIR;	
	}

	while ((ret = read(rd_fd, segment, sizeof(segment))) > 0) {	
		write(1, segment, ret);	
	}

	if (ret < 0) { // Error
		close(rd_fd);
		close(wr_fd);
		return -ERR_RDFILE;
	}

	close(rd_fd);
	close(wr_fd);
	return 0;
}
