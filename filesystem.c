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

int get_inode_with_path() {
	for (i = 0; i < inode_num_blocks; i++){
		inode_block = (struct ext2_inode *)(disk + (gd->bg_inode_table + i) * EXT2_BLOCK_SIZE);
		for (j = 0; j < inodes_per_block; j++){
			inode_block[j].
		}
	}
}

int dir_blocks() {
	
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

void make_dir(unsigned char *inode_bitmap, unsigned char *block_bitmap, char *path, unsigned char *disk, struct ext2_inode *it){
    int num_dirs;
    char ** seperated_path = make_path(path, &num_dirs);

    int num_checked_dirs = 0, i;
    int not_done = 1;
    int curr_inode = 2, parent_inode = -1;
    int curr_block;

    struct ext2_super_block *sb = (struct ext2_super_block *)(disk + 1024);
    int num_inodes = sb->s_inodes_count;
    int num_blocks = sb->s_blocks_count;

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


    int free_inode = get_empty_inode(inode_bitmap, num_inodes);
    int free_block = get_empty_data_block(block_bitmap, num_blocks);

    struct ext2_inode * ind = it + (sizeof(struct ext2_inode) * (free_inode - 1));
    struct ext2_dir_entry * new_block =  (struct ext2_dir_entry *) (disk + (free_block * EXT2_BLOCK_SIZE));

    
    struct ext2_inode *parent_ind = it + (sizeof(struct ext2_inode) * (parent_inode - 1));
    
    //char *names[3] = {".", "..", seperated_path[num_checked_dirs - 1]};
    new_block->rec_len = 12;
    new_block->name_len = 1;
    new_block->inode = free_inode;
    strcpy(new_block->name, ".");

    new_block += new_block->rec_len;

    new_block->rec_len = 1012;
    new_block->name_len = 2;
    new_block->inode = parent_inode;
    strcpy(new_block->name, "..");

    block_bitmap[(free_block - 1) / 8] |= (1 << (free_block - 1) % 8);

    ind->i_mode = (ind->i_mode & 0) | (EXT2_S_IFDIR);
    ind->i_block[0] = free_block;
    ind->i_size = EXT2_BLOCK_SIZE;
    ind->i_blocks = 1;
    ind->i_links_count = 2;

    inode_bitmap[(free_inode - 1) / 8] |= (1 << (free_inode - 1) % 8);

    parent_ind->i_links_count += 1;

    struct ext2_dir_entry *curr_dir;
    int rec_length = 0;

    for(i = 0; i < 15; i++){
        
        if(parent_ind->i_block[i]){
            curr_dir = (struct ext2_dir_entry *) (disk + parent_ind->i_block[i] * EXT2_BLOCK_SIZE);
            while(rec_length < EXT2_BLOCK_SIZE){
                if(rec_length + curr_dir->rec_len >= EXT2_BLOCK_SIZE && curr_dir->rec_len != curr_dir->name_len + 12 - (curr_dir->name_len % 4) && curr_dir->rec_len != curr_dir->name_len + 8){
                    if(curr_dir->name_len % 4 != 0){
                        curr_dir->rec_len = curr_dir->name_len + 12 - (curr_dir->name_len % 4);
                    }else{
                        curr_dir->rec_len = curr_dir->name_len + 8;
                    }

                    rec_length += curr_dir->rec_len;
                    curr_dir += curr_dir->rec_len;

                    curr_dir->inode = free_inode;
                    curr_dir->rec_len = EXT2_BLOCK_SIZE - rec_length;
                    curr_dir->name_len = strlen(seperated_path[num_dirs - 1]);
                    strcpy(curr_dir->name, seperated_path[num_dirs - 1]);
                    curr_dir->file_type = EXT2_FT_DIR;
                    rec_length += curr_dir->rec_len;
                    return;
                }
                else{
                    rec_length += curr_dir->rec_len;
                    curr_dir += curr_dir->rec_len;
                }
            }
        }
    }

}

