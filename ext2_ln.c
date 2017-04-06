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
struct ext2_inode *it;

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

struct ext2_dir_entry *get_dir_entry(int *dir_block_map, char *src_path) {
	int i;
	// hardlink
	int block_read = 0;
		// directory block
	char *current_dir = strtok(src_path, "/");

	while (current_dir != NULL) {
		for(i = 0; i < sb->s_inodes_count; i++) {
			if (dir_block_map[i] != 0) {

				struct ext2_dir_entry * directory_entry = (struct ext2_dir_entry *) (disk + dir_block_map[i] * EXT2_BLOCK_SIZE);
				while (block_read < EXT2_BLOCK_SIZE) {
					directory_entry = (void *)directory_entry + directory_entry->rec_len;
					// found the dir
					if (strncmp(current_dir, directory_entry->name, directory_entry->name_len) == 0 && strlen(current_dir) == directory_entry->name_len) {
							return directory_entry;
					}
					block_read += directory_entry->rec_len;
				}
			}
		}
		current_dir = strtok(NULL, "/");
	}
	return NULL;
}



int main(int argc, char **argv) {

    if (argc < 4 && argc > 5) {
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

    it = (struct ext2_inode *)(disk + gd->bg_inode_table * EXT2_BLOCK_SIZE);
	
	char *src_path;
	char *des_path;

	if (argc == 4) {
		src_path = malloc(sizeof(char) * strlen(argv[2]));
		src_path = argv[2];
 		des_path = malloc(sizeof(char) * strlen(argv[3]));
		des_path = argv[3];
	} else {
		src_path = malloc(sizeof(char) * strlen(argv[3]));
		src_path = argv[3];
 		des_path = malloc(sizeof(char) * strlen(argv[4]));
		des_path = argv[4];
	}
	
	int *dir_block_map = get_dir_block_map();
	struct ext2_dir_entry *src_dir = get_dir_entry(dir_block_map, src_path);
	struct ext2_dir_entry *des_dir = get_dir_entry(dir_block_map, des_path);
    int des_inode = des_dir->inode;
    int des_block = it[des_inode - 1].i_block[0];

    struct ext2_dir_entry * new_dir_entry = (struct ext2_dir_entry *) (disk + (des_block * EXT2_BLOCK_SIZE));

    int block_read = 0;

    it[des_inode - 1].i_links_count++; // update link count
    while(block_read < EXT2_BLOCK_SIZE){
   
        if(block_read + new_dir_entry->rec_len >= EXT2_BLOCK_SIZE ){
            if (new_dir_entry->name_len % 4 != 0) {
                new_dir_entry->rec_len = new_dir_entry->name_len + 12 - (new_dir_entry->name_len % 4);
            } else{
                new_dir_entry->rec_len = new_dir_entry->name_len + 8;
            }

            // get to the new empty part
            block_read += new_dir_entry->rec_len;
            new_dir_entry = (void *)new_dir_entry + new_dir_entry->rec_len;
            new_dir_entry->inode = src_dir->inode;
            new_dir_entry->rec_len = EXT2_BLOCK_SIZE - block_read;
            // hard link
            if (argc == 4) {
                new_dir_entry->name_len = strlen(src_dir->name);
                strcpy(new_dir_entry->name, src_dir->name);
            // soft link
            } else {
                new_dir_entry->name_len = strlen(argv[3]);
                strcpy(new_dir_entry->name, argv[3]);
            }
            new_dir_entry->file_type = EXT2_FT_REG_FILE;

			break;
        } else {
            block_read += new_dir_entry->rec_len;
            new_dir_entry = (void *)new_dir_entry + new_dir_entry->rec_len;
        }

    }
	return 1;
}
