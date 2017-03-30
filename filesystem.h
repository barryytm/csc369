#include "filesystem.c"

#define MAX_DIRECTION_POINTERS 12
#define SIZE_BLOCK_POINTER 4

int get_empty_data_block(unsigned char *block_bitmap, int blocks_count);
int get_empty_inode(unsigned char *inode_bitmap, int blocks_count);


