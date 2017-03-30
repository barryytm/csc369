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
	return 0;
}

int get_empty_inode(unsigned char *inode_bitmap, int blocks_count) {
	int i, j;
    // for block bit map
    for (i = 0; i < blocks_count / 8 ; i++) {
        for (j = 0; j < 8; j++) {
            if (((inode_bitmap[i] & 1 << j) >> j) == 0) {
                return i * 8 + (j + 1);
            }
        }

    }
	// no inodes avaible
    return 0;
}

int string_size(char *c){
    int i = 0;

    while(c[i] != '\0'){
        i++;
    }

    return i;
}


void split_path(char * path, char ** result){
    int i = 0;

    char * temp_path = malloc(sizeof(path));

    strcpy(temp_path, path);

    char * temp = strtok(temp_path, "/");

    while(temp != NULL){
        result[i] = malloc(sizeof(temp));
        strcpy(result[i], temp);
        temp = strtok(temp_path, "/");
        i++;
    }
}

char ** make_path(char *path, int *num_directories){
    char ** result;
    *num_directories = num_directories_in_path(path);

    result = malloc(num_directories * sizeof(char *));

    split_path(path, result);

    return result;
}

int check_block(unsigned char * disk, int curr_block, char * dir_name){
    int rec_length = 0;
    int dir_name_len = string_size(dir_name);

    char * temp = malloc(dir_name_len);

    strncpy(temp, dir_name, dir_name_len);

    struct ext2_dir_entry * directory_entry = (struct ext2_dir_entry *) (disk + curr_block * EXT2_BLOCK_SIZE);

    while(rec_length < EXT2_BLOCK_SIZE){
        if(strcmp(temp, directory_entry->name)){
            return directory_entry->inode;
        }
        rec_length += directory_entry->rec_length;
    }

    return -1;
}

int find_free_inode(unsigned char * inode_bitmap, int num_inodes){
    int bit;

    for(int i = 0; i < num_inodes; i++){
        if(i == 1 || i >= 11){
            bit = (inode_bitmap[i/8] & 1 << (i % 8)) >> (i % 8);
            if(bit == 0){
                return i + 1;
            }
        }
    }

    return -1;
}
