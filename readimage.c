#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include "ext2.h"

unsigned char *disk;



int main(int argc, char **argv) {

    if(argc != 2) {
        fprintf(stderr, "Usage: %s <image file name>\n", argv[0]);
        exit(1);
    }
    int fd = open(argv[1], O_RDWR);

    disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(disk == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    struct ext2_super_block *sb = (struct ext2_super_block *)(disk + 1024);
	struct ext2_group_desc *gd = (struct ext2_group_desc *)(disk + 2 * EXT2_BLOCK_SIZE);

	struct ext2_inode * it = (struct ext2_inode *)(disk + gd->bg_inode_table * EXT2_BLOCK_SIZE);

	unsigned char *block_bitmap = disk + gd->bg_block_bitmap * EXT2_BLOCK_SIZE;
	unsigned char *inode_bitmap = disk + gd->bg_inode_bitmap * EXT2_BLOCK_SIZE;


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


	unsigned int inodes_per_block = EXT2_BLOCK_SIZE / sizeof(struct ext2_inode);
	// Inode table
	unsigned int it_num_blocks = sb->s_inodes_per_group / inodes_per_block;

	int inode_number = 1;

	unsigned int mask = 0b000000000111;

	unsigned int dir_blocks[sb->s_inodes_count];

	printf("\n\nInodes:\n");

	for(i = 0; i < it_num_blocks; i++){
		it = (struct ext2_inode *)(disk + (gd->bg_inode_table + i) * EXT2_BLOCK_SIZE);
		for(j = 0; j < inodes_per_block; j++){
			if((it[j].i_mode & EXT2_S_IFREG) != 0 && it[j].i_mode & mask && it[j].i_size){
				printf("[%d] type: f size: %d links: %d blocks: %d\n", inode_number, it[j].i_size, it[j].i_links_count,it[j].i_blocks);
				printf("[%d] Blocks: %d\n", inode_number, it[j].i_block[0]);
				dir_blocks[inode_number - 1] = 0;
			}
			else if(((it[j].i_mode)  & EXT2_S_IFDIR) != 0 && it[j].i_mode & mask && it[j].i_size){
				printf("[%d] type: d size: %d links: %d blocks: %d\n",  inode_number, it[j].i_size, it[j].i_links_count,it[j].i_blocks);
				printf("[%d] Blocks: %d\n", inode_number, it[j].i_block[0]);
				dir_blocks[inode_number - 1] = it[j].i_block[0];
			}
			else{dir_blocks[inode_number - 1] = 0;}

			inode_number++;

		}
	}



	printf("\n\nDirectory Blocks:\n");
	int total_size = 0;
	int rec_len = 0;

	it = (struct ext2_inode *)(disk + gd->bg_inode_table * EXT2_BLOCK_SIZE);
	for(i = 0; i < sb->s_inodes_count; i++){

		if(dir_blocks[i]){

			//struct ext2_dir_entry * directory_entry2 = (struct ext2_dir_entry *) (disk + (gd->bg_inode_table + it_num_blocks + 1) * EXT2_BLOCK_SIZE);
			printf("	DIR BLOCK NUM: %d (for inode %d)\n", dir_blocks[i], i + 1);

			struct ext2_dir_entry * directory_entry = (struct ext2_dir_entry *) (disk + dir_blocks[i] * EXT2_BLOCK_SIZE);


			while(total_size < EXT2_BLOCK_SIZE){
				directory_entry = (void *)directory_entry + rec_len;

				int file_type = directory_entry->file_type;

				char * file_name = malloc(directory_entry->name_len + 1);

				file_name = strncpy(file_name, directory_entry->name, directory_entry->name_len);
				file_name[directory_entry->name_len + 1] = '\0';

				if(file_type == EXT2_FT_DIR){
					printf("Inode: %d rec_len: %d name_len: %d type = d name = %s\n", directory_entry->inode, directory_entry->rec_len, directory_entry->name_len, file_name);
				}

				else{
					printf("Inode: %d rec_len: %d name_len: %d type = f name = %s\n", directory_entry->inode, directory_entry->rec_len, directory_entry->name_len, file_name);
				}

				rec_len = directory_entry->rec_len;
				total_size += rec_len;
			}

			rec_len = 0;
			total_size = 0;
		}

	}

    return 0;
}
