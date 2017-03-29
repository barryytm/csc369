#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include "ext2.h"
#include "filesystem.c"

unsigned char *disk;
struct ext2_super_block *sb;
struct ext2_group_desc *gd;
struct ext2_inode * inode_table;
unsigned char *block_bitmap;
unsigned char *inode_bitmap;
struct ext2_inode *inode;
unsigned int inodes_per_block;
unsigned int it_num_blocks;
int s_blocks_count;

int main(int argc, char **argv) {

    if(argc != 2) {
        fprintf(stderr, "Usage: %s <image file name>\n", argv[0]);
        exit(1);
    }

    int fd = open(argv[1], O_RDWR);

    disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(disk == MAP_FAILED) {
        perror("failed to open the file");
        exit(ENOENT);
    }

    // make sb point to the superblock
    sb = (struct ext2_super_block *)(disk + 1024);

    // make gd point to the group_desc block
    gd = (struct ext2_group_desc *)(disk + 2 * EXT2_BLOCK_SIZE);

    // inode table
    inode_table = (struct ext2_inode *)(disk + gd->bg_inode_table * EXT2_BLOCK_SIZE);

    // both bitmaps
    block_bitmap = disk + gd->bg_block_bitmap * EXT2_BLOCK_SIZE;
	inode_bitmap = disk + gd->bg_inode_bitmap * EXT2_BLOCK_SIZE;

    // sizes
    inodes_per_block = EXT2_BLOCK_SIZE / sizeof(struct ext2_inode);
    it_num_blocks = sb->s_inodes_per_group / inodes_per_block;

    printf("Inodes: %d\n", sb->s_inodes_count);
    printf("Blocks: %d\n", sb->s_blocks_count);
    printf("Block group: \n");
    printf("	block bitmap: %d\n", gd->bg_block_bitmap);
    printf("	inode bitmap: %d\n", gd->bg_inode_bitmap);
    printf("	inode table: %d\n", gd->bg_inode_table);
    printf("	free blocks: %d\n", gd->bg_free_blocks_count);
    printf("	free inodes: %d\n", gd->bg_free_inodes_count);
    printf("	used_dirs: %d\n", gd->bg_used_dirs_count);
    printf("Block bitmap: ");
    int i;
    int j;
    // for block bit map
    for (i = 0; i < sb->s_blocks_count / 8; i++) {
        for (j = 0; j < 8; j++) {

            printf("%d", (block_bitmap[i] & 1 << j) >> j);
        }
        printf(" ");
    }
    printf("\n");
    printf("Inode bitmap: ");
    // for inode bitmap
    for (i = 0; i < sb->s_inodes_count / 8; i++) {
        for (j = 0; j < 8; j++) {

            printf("%d", (inode_bitmap[i] & 1 << j) >> j);
        }
        printf(" ");
    }
    printf("\n");
    int empty_block;
    empty_block = find_empty_block(block_bitmap, &it_num_blocks, sb->s_blocks_count);
    printf("%d", empty_block);
    printf("\n");
    return 0;

}
