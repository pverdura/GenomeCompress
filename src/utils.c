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

int setPaths(char* dir, void* dir_orig, void* dir_dest) {
	int n = strlen(dir);
	int size_dest;

	if (dir[n-1] == '/') {
		sprintf(dir_orig, "%s", dir);
		dir[n-1] = '_';
		size_dest = sprintf(dir_dest, "%scomp/", dir);
	}
	else {
		sprintf(dir_orig, "%s/", dir);
		size_dest = sprintf(dir_dest, "%s_comp/", dir);
	}

	return size_dest;
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
		write(wr_fd, block, size);	
	}
	
	close(rd_fd);
	close(wr_fd);

	if (size < 0) { // Error
		return -ERR_RDFILE;
	}

	return 0;
}
