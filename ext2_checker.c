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

int get_dir_block_num() {
	unsigned int mask = 0b000000000111;
	int dir_block_num = 0;
	int i, j;
	for (i = 0; i < inode_num_blocks; i++){
        inode_block = (struct ext2_inode *)(disk + (gd->bg_inode_table + i) * EXT2_BLOCK_SIZE);
        for (j = 0; j < inodes_per_block; j++) {
            // found a directory block
            if(((inode_block[j].i_mode)  & EXT2_S_IFDIR) != 0 &&
                inode_block[j].i_mode & mask && inode_block[j].i_size > 0){
                    dir_block_num++;
            }
        }
    }
	return dir_block_num++;
}

int *get_dir_block_map() {
    unsigned int mask = 0b000000000111;
    int *dir_block_map = malloc(sizeof(int) * get_dir_block_num());
	int current_dir_block = 0;
    int i, j;
	
	
    for (i = 0; i < inode_num_blocks; i++){
        inode_block = (struct ext2_inode *)(disk + (gd->bg_inode_table + i) * EXT2_BLOCK_SIZE);
        for (j = 0; j < inodes_per_block; j++) {
            // found a directory block
            if(((inode_block[j].i_mode)  & EXT2_S_IFDIR) != 0 &&
                inode_block[j].i_mode & mask && inode_block[j].i_size > 0){
                    dir_block_map[current_dir_block] = inode_block[j].i_block[0];
					current_dir_block++;
            }
        }
    }

	return dir_block_map;
}

int get_num_free_bit(unsigned char *bitmap, int blocks_count) {
	int i, j;
	int num_free_bit = 0;
    // for block bit map
    for (i = 0; i < blocks_count / 8 ; i++) {
        for (j = 0; j < 8; j++) {
            if (((bitmap[i] & 1 << j) >> j) == 0) {
                num_free_bit++;
            }
        }
    }
	return num_free_bit;
}

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

	int num_free_block = num_free_bit(block_bitmap, sb->s_blocks_count);
	int num_free_inode = num_free_bit(inode_bitmap, sb->s_blocks_count);

	//a
	if (sb->s_free_blocks_count != num_free_block) {
		sb->s_free_blocks_count = num_free_block;
		printf("Fixed: superblock's free blocks counter was off by %d compared to the bitmap", abs(sb->s_free_blocks_count - num_free_block));
		if (gd-> bg_free_blocks_count != num_free_block) {
			gd-> bg_free_blocks_count = num_free_block
			printf("Fixed: block group's free blocks counter was off by %d compared to the bitmap", abs(gd-> bg_free_blocks_count - num_free_block));
		}
	}

	if (sb->s_free_inodes_count != num_free_inode) {
		sb->s_free_inodes_count = num_free_inode;
		printf("Fixed: superblock's free blocks counter was off by %d compared to the bitmap", abs(s_free_inodes_count - num_free_inode));
		if (gd-> bg_free_inodes_count != num_free_inode) {
			gd-> bg_free_inodes_count = num_free_inode;
			printf("Fixed: block group's free blocks counter was off by %d compared to the bitmap", abs(gd-> bg_free_inodes_count - num_free_inode));
		}
	}

	//b
	int i;
	int *dir_block_map = get_dir_block_map();
	int block_read = 0;
	struct ext2_inode * it = (struct ext2_inode *)(disk + gd->bg_inode_table * EXT2_BLOCK_SIZE);
	it = (struct ext2_inode *)(disk + gd->bg_inode_table * EXT2_BLOCK_SIZE);
	
	for(i = 0; i < sb->s_inodes_count; i++) {
			if (dir_block_map[i] != 0) {
				struct ext2_dir_entry * directory_entry = (struct ext2_dir_entry *) (disk + dir_block_map[i] * EXT2_BLOCK_SIZE);
				while (block_read < EXT2_BLOCK_SIZE) {
					directory_entry = (void *)directory_entry + directory_entry->rec_len;
					// soft link
					if (it[directory_entry->inode - 1]->i_mode == EXT2_FT_SYMLINK && directory_entry->file_type != EXT2_S_IFLNK) {
						directory_entry->file_type = EXT2_FT_SYMLINK;
					// dir
					} else if (it[directory_entry->inode - 1]->i_mode == EXT2_S_IFDIR && directory_entry->file_type != EXT2_FT_DIR) {
						directory_entry->file_type = EXT2_FT_DIR;
					// file
					} else if (it[directory_entry->inode - 1]->i_mode == EXT2_FT_REG_FILE && directory_entry->file_type != EXT2_S_IFREG) {
						directory_entry->file_type = EXT2_S_IFREG;
					}
				
				}
		}
	}
}
	}

}
