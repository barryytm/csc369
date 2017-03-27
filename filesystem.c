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
