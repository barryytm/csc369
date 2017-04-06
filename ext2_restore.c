#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include "ext2.h"
#include "filesystem.h"



int main(int argc, char **argv){
	unsigned char *disk;
	if(argc != 3) {
        fprintf(stderr, "Usage: %s <image file name> <absolute path of directory>\n", argv[0]);
        exit(-1);
    }

    int fd = open(argv[1], O_RDWR);

    disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(disk == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    char *path = argv[2];
    
    struct ext2_super_block *sb = (struct ext2_super_block *)(disk + 1024);
    struct ext2_group_desc *gd = (struct ext2_group_desc *)(disk + 2 * EXT2_BLOCK_SIZE);
    
    struct ext2_inode * it = (struct ext2_inode *)(disk + gd->bg_inode_table * EXT2_BLOCK_SIZE);

    unsigned char *block_bitmap = disk + gd->bg_block_bitmap * EXT2_BLOCK_SIZE;
    unsigned char *inode_bitmap = disk + gd->bg_inode_bitmap * EXT2_BLOCK_SIZE;

    int num_dirs;
    char ** seperated_path = make_path(path, &num_dirs);

    int num_checked_dirs = 0;
    int not_done = 1;
    int curr_inode = 2, parent_inode = 2;
    int curr_block;

    int num_inodes = sb->s_inodes_count;
    int num_blocks = sb->s_blocks_count;

    while(not_done){
        curr_block = it[curr_inode - 1].i_block[0];

        int temp = curr_inode;
        
        curr_inode = check_block(disk, curr_block, seperated_path[num_checked_dirs]);

        num_checked_dirs++;

        if(num_checked_dirs == num_dirs){
            parent_inode = temp;
            not_done = 0;
            if(curr_inode){
                perror("Path already exists.\n");
                exit(EEXIST);
            }
        }else if(curr_inode == 0){
            perror("Path does not exist.\n");
            exit(ENOENT);
        }else{
            parent_inode = temp;
        }
    }
    //int parent_block = it[parent_inode - 1].i_block[0];
    struct ext2_dir_entry *parent_block = (struct ext2_dir_entry*) (disk + it[parent_inode - 1].i_block[0] * EXT2_BLOCK_SIZE);

    int rec_len = 0;

    while(rec_len < EXT2_BLOCK_SIZE){
        if(parent_block->rec_len != (parent_block->name_len + 12 - (parent_block->name_len % 4)) && parent_block->rec_len != (parent_block->name_len + 8)){
            struct ext2_dir_entry *temp;
            if(parent_block->name_len % 4 != 0){
                temp = (void*)parent_block + parent_block->name_len + 12 - (parent_block->name_len % 4) ;
            }
            else{
                temp = (void*)parent_block + parent_block->name_len + 8;
            }

            printf("%s %s\n", seperated_path[num_dirs - 1], temp->name);

        }
    }

    
}