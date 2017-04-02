#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include "ext2.h"
#include "filesystem.h"

unsigned char *disk;
struct ext2_super_block *sb;
struct ext2_group_desc *gd;
struct ext2_inode * inode_block;
unsigned char *block_bitmap;
unsigned char *inode_bitmap;
struct ext2_inode *inode;
unsigned int inodes_per_block;
unsigned int inode_num_blocks;
int s_blocks_count;
int empty_data_block;
int empty_inode;
char buffer[EXT2_BLOCK_SIZE];	// usee block size as the buffer size
FILE *file;

int main(int argc, char **argv) {

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <image file name>\n", argv[0]);
        exit(1);
    }

    int fd = open(argv[1], O_RDWR);

    disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (disk == MAP_FAILED) {
        perror("failed to open the file");
        exit(ENOENT);
    }

    // make sb point to the superblock
    sb = (struct ext2_super_block *)(disk + EXT2_BLOCK_SIZE);

    // make gd point to the group_desc block
    gd = (struct ext2_group_desc *)(disk + 2 * EXT2_BLOCK_SIZE);

    // both bitmaps
    block_bitmap = disk + gd->bg_block_bitmap * EXT2_BLOCK_SIZE;
	inode_bitmap = disk + gd->bg_inode_bitmap * EXT2_BLOCK_SIZE;

    // sizes
    inodes_per_block = EXT2_BLOCK_SIZE / sizeof(struct ext2_inode);
    inode_num_blocks = sb->s_inodes_per_group / inodes_per_block;
	
	struct stat file_stat;

	// hard links
		



    return 0;

}
