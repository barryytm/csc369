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

int main(int argc, char ** argv){
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

	//struct ext2_super_block *sb = (struct ext2_super_block *)(disk + 1024);
	//int num_inodes = sb->s_inodes_count;
	//int num_blocks = sb->s_blocks_count;

	while(not_done){
		curr_block = it[curr_inode - 1].i_block[0];

		int temp = curr_inode;
		curr_inode = check_block(disk, curr_block, seperated_path[num_checked_dirs]);

		num_checked_dirs++;

		if(num_checked_dirs == num_dirs){
			not_done = 0;
			if(curr_inode){
				perror("Path already exists.\n");
				exit(EEXIST);
			}
		}else if(curr_inode == -1){
			perror("Path does not exist.\n");
			exit(ENOENT);
		}else{
			parent_inode = temp;
		}
	}

	struct ext2_inode *parent_ind = &it[curr_inode - 1];
	struct ext2_inode *target_ind = &it[parent_inode - 1];


	struct ext2_dir_entry  *curr_entry;
	struct ext2_dir_entry *prev_entry;

	if(target_ind->i_mode & EXT2_S_IFDIR){
		perror("rm can only have file and symbolic link targets.\n");
		exit(EINVAL);
	}

	int cur_rec, i = 0;
	char buf[100];

	//make sure that in the inode block the target is skipped over in the future
	//for(i = 0; i < 15; i++) {
	    cur_rec = 0;
        //if(parent_ind->i_block[0]) {
            curr_entry = (struct ext2_dir_entry *)(disk + parent_ind->i_block[i] * EXT2_BLOCK_SIZE);
            while(cur_rec < 1024){
      	      prev_entry = curr_entry;
      	      curr_entry = (struct ext2_dir_entry *)(disk + parent_ind->i_block[i] * EXT2_BLOCK_SIZE + cur_rec);
      	      
      	      strncpy(buf, curr_entry->name, curr_entry->name_len);
      	      buf[curr_entry->name_len] = '\0';

	           if(strcmp(buf, seperated_path[num_dirs - 1]) == 0){
          		 prev_entry->rec_len += curr_entry->rec_len;
          		 // clean entry just to be safe
          		 curr_entry = prev_entry;
          		 cur_rec = EXT2_BLOCK_SIZE; //done after this
	           }
	           cur_rec = cur_rec + curr_entry->rec_len; 
	         }
        //}
    //}


    //now deal with the files inode
    target_ind->i_links_count -= 1;
    if(target_ind->i_links_count <= 0){
	    inode_bitmap[(curr_inode - 1) / 8] &= ~(1 << (curr_inode - 1) % 8);
	    block_bitmap[(target_ind->i_block[0] - 1) / 8] &= ~(1 << (target_ind->i_block[0] - 1) % 8);

	    target_ind->i_dtime = time(NULL);
      	target_ind->i_size = 0;
      	target_ind->i_mode = 0;
	}




	return 0;
}