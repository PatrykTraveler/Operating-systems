#include "libusage.h"

void libgenerate(const char* filename, int rnumber, size_t rsize){
    FILE* f = fopen(filename, "w");
    if(f == NULL){
        printf("Error while opening %s\n", filename);
        exit(-1);
    }


    unsigned char* buffer = calloc(rnumber*rsize, sizeof(unsigned char));
    for(int i = 0; i < rnumber*rsize; i++){ buffer[i]=(unsigned char)(rand() % 25 + 97); }
    fwrite(buffer, sizeof(unsigned char), rnumber*rsize, f);
    free(buffer);
    fclose(f);
}

void libsort(const char* filename, int rnumber, size_t rsize){
    FILE* f = fopen(filename, "r+");
    if(f == NULL){
        printf("Error while opening %s\n", filename);
        exit(-1);
    }

    unsigned char* key = calloc(rsize, sizeof(unsigned char));
    unsigned char* buff = calloc(rsize, sizeof(unsigned char));

    for(int i = 1; i < rnumber; i++){
        fseek(f, i*rsize, 0);
        fread(key, sizeof(unsigned char), rsize, f);
        fseek(f, (-2)*rsize, 1);
        fread(buff, sizeof(unsigned char), rsize, f);
        int j = i;
        while(j > 1 && buff[0] > key[0]){
            if(fwrite(buff, sizeof(unsigned char), rsize, f) != rsize){return;};
            fseek(f, (-3)*rsize, 1);
            if(fread(buff, sizeof(unsigned char), rsize, f) != rsize){return;};
            j=j-1;
        }
        if(j == 1 && buff[0] > key[0]){
            if(fwrite(buff, sizeof(unsigned char), rsize, f) != rsize){return;}
            fseek(f, (-2)*rsize, 1);
        }
        if(fwrite(key, sizeof(unsigned char), rsize, f) != rsize){return;};
    }

    free(key);
    free(buff);
    fclose(f);
}

void libcopy(const char* from, const char* to, int rnumber, size_t rsize){
    FILE* from_ptr = fopen(from, "r");
    FILE* to_ptr = fopen(to, "w");
    if(from_ptr == NULL || to_ptr == NULL)
        return;

    unsigned char* buffer = calloc(rsize, sizeof(unsigned char));
    for(int i = 0; i < rnumber; i++){
        fread(buffer, sizeof(unsigned char), rsize, from_ptr);
        fwrite(buffer, sizeof(unsigned char), rsize, to_ptr);
    }

    free(buffer);
    fclose(from_ptr);
    fclose(to_ptr);
}