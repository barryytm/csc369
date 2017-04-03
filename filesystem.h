#include "filesystem.c"

#define MAX_DIRECTION_POINTERS 12;
#define SIZE_BLOCK_POINTER 4;

int get_empty_data_block(unsigned char *block_bitmap, int blocks_count);
int get_empty_inode(unsigned char *inode_bitmap, int blocks_count);

int string_size(char *c);
void split_path(char * path, char ** result);
int num_directories_in_path(char * path);
char ** make_path(char *path, int *num_directories);
int check_block(unsigned char * disk, int curr_block, char * dir_name);

int *make_bitmap(unsigned char * bitmap, int bitmap_length);