/*
 * utils.c
 */
#include "utils.h"

void usage(char* exec) {
	char buff[128];
	int n = sprintf(buff, "Usage: %s <directory> ...\n  Where <directory> is the path to the directory that contains\n  all the nucleotide files.\n", exec);
	
	write(1, buff, n);
}

void print_error(int err)
{
	if (err == -ERR_OPEN) {
		perror("open()");
	} else if (err == -ERR_READ) {
		perror("read()");
	} else if (err == -ERR_WRITE) {
		perror("write()");
	}	
}

int compress(char* file, char* dest) {
	char segment[128];
	int ret;
	int rd_fd = open(file, O_RDONLY);
	//int wr_fd = creat(dest, );

	if (rd_fd < 0) { //|| wr_fd < 0) {
		return -ERR_OPEN;
	}

	while ((ret = read(rd_fd, segment, sizeof(segment))) > 0) {	
		write(1, segment, ret);	
	}

	if (ret < 0) { // Error
		close(rd_fd);
		//close(wr_fd);
		return -ERR_READ;
	}

	close(rd_fd);
	//close(wr_fd);
	return 0;
}
