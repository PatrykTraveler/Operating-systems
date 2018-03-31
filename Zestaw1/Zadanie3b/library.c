#include "library.h"

char fixed_array[MAX_SIZE][BLOCK_SIZE] = {0};

block_t* set_array(int size, int block_size, int dynamic_alloc){
    block_t* block = calloc(1, sizeof(block));

    if(dynamic_alloc)
        block->dynamic_ptr = calloc(size, sizeof(char*));
    else
        block->static_ptr = fixed_array;

    block->max_size = dynamic_alloc ? size : MAX_SIZE;
    block->block_size = dynamic_alloc ? block_size : BLOCK_SIZE;
    block->is_dynamic = dynamic_alloc;
    return block;

}

void remove_array(block_t* blocks){
    if(blocks == NULL)
        return;
    if(!blocks->is_dynamic)
        return;

    for(int i = 0; i < blocks->max_size; i++)
        remove_block(blocks, i);
    free(blocks->dynamic_ptr);
    free(blocks);
}

void set_block(block_t* blocks, const char* block, int index){
    if(blocks == NULL || index < 0)
        return;

    size_t block_length = strlen(block);

    if(block_length > blocks->block_size)
        return;

    if(blocks->is_dynamic)
        blocks->dynamic_ptr[index] = calloc(block_length, sizeof(char));

    strncpy(blocks->is_dynamic ? blocks->dynamic_ptr[index] : blocks->static_ptr[index], block, block_length);
}

void remove_block(block_t* blocks, int index){
    if(blocks == NULL || index < 0 || blocks->dynamic_ptr[index] == NULL)
        return;

    if(blocks->is_dynamic)
        free(blocks->dynamic_ptr[index]);
    else
        strncpy(blocks->static_ptr, NULL, 0);
}

int get_character_sum(const char* block){
    int sum=0, j=0;

    while(block[j] != '\0'){
        sum += block[j];
        j++;
    }
    return sum;
}

int find_block(block_t* blocks, const char* block){
    if(blocks == NULL)
        return -1;

    int diff = INT_MAX, block_id = -1, block_sum, sum;
    block_sum = get_character_sum(block);
    for(int i = 0; i < blocks->max_size; i++){
        sum = blocks->is_dynamic ? get_character_sum(blocks->dynamic_ptr[i]) : get_character_sum(blocks->static_ptr[i]);
        if (abs(block_sum - sum) < diff) {
            diff = abs(block_sum - sum);
            block_id = i;
        }
    }
    if(block_id == -1)
        return 0;

    return block_id;
}

//---------------------------
void print_array(block_t* block){
    if(block == NULL)
        return;

    for(int i = 0; i < block->max_size; i++)
        printf("[%s] ", block->is_dynamic ? block->dynamic_ptr[i] : block->static_ptr[i]);
    printf("\n");
}






