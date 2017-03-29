#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include "ext2.h"
#include "filesystem.h"

int find_empty_block(unsigned char *block_bitmap, unsigned int *it_num_blocks_ptr) {
    
    while (((block_bitmap[i] & 1 << j) >> j) != 0) {

    }
    int i;
    int j;
    // for block bit map
    for (i = 0; i < sb->s_blocks_count / 8; i++) {
        for (j = 0; j < 8; j++) {

            printf("%d", (block_bitmap[i] & 1 << j) >> j);
        }
        printf(" ");
    }
}

int *make_bitmap(unsigned char * bitmap, int bitmap_length){
    int *map = malloc(sizeof(int) * bitmap_length);

    for (i = 0; i < bitmap_length / 8; i++) {
        for (j = 0; j < 8; j++) {
            map[i*8 + j] = (bitmap[i] & 1 << j) >> j;
        }
    }

    return map;
}

//the 3 functions below are for splitting the file paths
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