#include <stdlib.h>

int get_empty_data_block(unsigned char *block_bitmap, int blocks_count) {
    int i, j;
    // for block bit map
    for (i = 0; i < blocks_count / 8 ; i++) {
        for (j = 0; j < 8; j++) {
            if (((block_bitmap[i] & 1 << j) >> j) == 0) {
                return i * 8 + (j + 1);
            }
        }
    }
	// no bits avaible
	return -1;
}

int get_empty_inode(unsigned char *inode_bitmap, int blocks_count) {
	int i, j;
    // for block bit map
    for (i = 0; i < blocks_count / 8 ; i++) {
        for (j = 0; j < 8; j++) {
            if (((inode_bitmap[i] & 1 << j) >> j) == 0) {
                int inode = i * 8 + (j + 1);
                if (inode == 1 || inode > 10) {
                    return inode;
                }
            }
        }

    }
	// no inodes avaible
    return -1;
}
/*
int get_inode_with_path() {
	for (i = 0; i < inode_num_blocks; i++){
		inode_block = (struct ext2_inode *)(disk + (gd->bg_inode_table + i) * EXT2_BLOCK_SIZE);
		for (j = 0; j < inodes_per_block; j++){
			inode_block[j].
		}
	}
}
*/



int string_size(char *c){
    int i = 0;

    while(c[i] != '\0'){
        i++;
    }

    return i;
}


void split_path(char * path, char ** result){
    int i = 0;

    char * temp_path = malloc(strlen(path));

    strcpy(temp_path, path);

    char * temp = strtok(temp_path, "/");

    while(temp != NULL){
        result[i] = malloc(strlen(temp));
        strcpy(result[i], temp);
        temp = strtok(NULL, "/");
        i++;
    }
}

int num_directories_in_path(char * path){
    int count = 0;
    int i = 0;
    
    while(path[i] != '\0'){
        if(path[i] == '/'){
            count++;
        }
        i++;
    }

    return count + 1;
}

char ** make_path(char *path, int *num_directories){
    char ** result;
    *num_directories = num_directories_in_path(path);

    result = malloc((*num_directories) * sizeof(char *));

    split_path(path, result);

    return result;
}

int check_block(unsigned char * disk, int curr_block, char * dir_name){
    int rec_length = 0;
    int dir_name_len = string_size(dir_name);

    char * temp = malloc(dir_name_len);

    strncpy(temp, dir_name, dir_name_len);

    struct ext2_dir_entry * directory_entry = (struct ext2_dir_entry *) (disk + curr_block * EXT2_BLOCK_SIZE);

    char buf[EXT2_NAME_LEN];

    while(rec_length < EXT2_BLOCK_SIZE){
        strncpy(buf, directory_entry->name, directory_entry->name_len);
        buf[directory_entry->name_len] = '\0';

        //printf("%s %s %d\n", temp, buf, strcmp(temp, buf));
        if(strcmp(temp, buf) == 0){
            return directory_entry->inode;
        }

        rec_length += directory_entry->rec_len;
        directory_entry = (void *)directory_entry + directory_entry->rec_len;
    }

    
    return 0;
}


int *make_bitmap(unsigned char * bitmap, int bitmap_length){
    int *map = malloc(sizeof(int) * bitmap_length);
    int i,j;
    for (i = 0; i < bitmap_length / 8; i++) {
        for (j = 0; j < 8; j++) {
            map[i*8 + j] = (bitmap[i] & 1 << j) >> j;
        }
    }

    return map;
}