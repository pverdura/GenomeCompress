/* 
 * define.h
 */
#ifndef __DEFINE_H
#define __DEFINE_H

#define NUCLXCHAR	4
#define HUFFMAN_S	4
#define BLOCK_SIZE	4096
#define DICT_SIZE	8192

#define NUM_THREADS	4

#define ERR_OPDIR	1
#define ERR_CLSDIR	5
#define ERR_RDDIR	10
#define ERR_MKDIR	15
#define ERR_OPFILE	20
#define ERR_RDFILE	25
#define ERR_WRFILE	30
#define ERR_COMPR	35

typedef struct {
	char file_dest[512];
	char file_orig[512];
} thread_data_t;

#endif //_DEFINE_H
