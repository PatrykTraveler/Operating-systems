//
// Created by traveler on 08.03.18.
//

#ifndef ZADANIE1_LIBRARY_H
#define ZADANIE1_LIBRARY_H

#include <glob.h>
#include <zconf.h>
#include <malloc.h>
#include <stdlib.h>
#include <memory.h>

#define MAX_SIZE 1000
#define BLOCK_SIZE 16

typedef char char_t[BLOCK_SIZE];

typedef struct block_t{
    union {
        char_t *static_ptr;
        char **dynamic_ptr;
    };

    unsigned int max_size;
    unsigned int block_size;
    unsigned int is_dynamic;
}block_t;

//memory functions
block_t* set_array(unsigned int size, unsigned int block_size, int dynamic_alloc);
void remove_array(block_t* blocks);
void set_block(block_t* blocks, const char* block, int index);
void remove_block(block_t* blocks, int index);

//utility functions
int get_character_sum(const char* block);
int find_block(block_t* blocks, const char* block);
void print_array(block_t* block);

#endif //ZADANIE1_LIBRARY_H
