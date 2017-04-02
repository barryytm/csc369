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

    if (argc != 4) {
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

    empty_data_block = get_empty_data_block(block_bitmap, sb->s_blocks_count);
    printf("%d\n", empty_data_block);

	empty_inode = get_empty_inode(inode_bitmap, sb->s_inodes_count);
	printf("%d\n", empty_inode);

	// allocate the inode
	int i, j;
	int current_inode = 1;
	for (i = 0; i < inode_num_blocks; i++){
		inode_block = (struct ext2_inode *)(disk + (gd->bg_inode_table + i) * EXT2_BLOCK_SIZE);
		for (j = 0; j < inodes_per_block; j++){
			if (current_inode == empty_inode) {
				inode_block[j].i_mode = EXT2_S_IFREG;
				inode_block[j].i_uid = 0;
				inode_block[j].i_ctime = time(0);
				inode_block[j].i_gid = 0;
				inode_block[j].i_link_count = 1; // still not sure
				inode_block[j].i_blocks = 0;
				inode_block[j].osd1 = 0;
				inode_block[j].i_block[15]; // needed
				inode_block[j].i_generation = 0;
				inode_block[j].i_file_acl = 0;
				inode_block[j].i_dir_acl = 0;
				inode_block[j].i_faddr;
				// not sure about extra

				file = fopen(argv[2], "r");
				// get the size of the file
				fseek(file, 0L, SEEK_END);
				inode_block[j].i_size = ftell(file);
				// make sure it reads from the beginning
				fseek(file, 0, SEEK_SET);

				int pointers_used = 0;
				// write read and write
				while (fgets(buffer, EXT2_BLOCK_SIZE, file) != NULL) {
					// direct pointers
					if (inode_block[j].blocks < MAX_DIRECTION_POINTERS) {
						// write the data to the blocks
						memcpy((disk + (empty_data_block * EXT2_BLOCK_SIZE)), buffer, EXT2_BLOCK_SIZE);
						// set the bit on the block map to 1 (in use)
						block_bitmap |= (1 < (empty_data_block - 1));
						// update the pointer
						inode_block[j].i_block[inode_block[j].i_blocks] = empty_data_block;
					} else {
						// first time using indirect pointers
						if (inode_block[j].i_blocks == MAX_DIRECTION_POINTERS) {
							// set the bit on the block map to 1 (in use)
							block_bitmap |= (1 < (empty_data_block - 1));
							// update the pointer
							inode_block[j].i_block[inode_block[j].i_blocks] = empty_data_block;

							inode_block[j].i_blocks++;
							sb->s_free_blocks_count++;
							gd->bg_free_blocks_count++;
						}
						memcpy((disk + (empty_data_block * EXT2_BLOCK_SIZE + inode_block[j].i_blocks - 12)), buffer, EXT2_BLOCK_SIZE);
					}
					// update the count for blocks
					block_bitmap |= (1 < (empty_data_block - 1));
					inode_block[j].i_blocks++;
					sb->s_free_blocks_count++;
					gd->bg_free_blocks_count++;
					// request a another empty block
					empty_data_block = get_empty_data_block(block_bitmap, sb->s_blocks_count);

					// update metadata
					sb->s_free_inodes_count++;
					gd->bg_free_inodes_count++;
				}

			}
			current_inode++;

		}
	}
	
	inode_bitmap |= (1 < (empty_inode - 1));
		



    return 0;

}
