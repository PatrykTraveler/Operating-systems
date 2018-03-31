#include <zconf.h>
#include <fcntl.h>
#include "sysusage.h"

void sysgenerate(const char* filename, int rnumber, size_t rsize){
    int f = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if(f < 0){
        printf("Error while opening %s\n", filename);
        exit(-1);
    }

    unsigned char* buffer = calloc(rnumber*rsize, sizeof(unsigned char));
    for(int i = 0; i < rnumber*rsize; i++){ buffer[i]=(unsigned char)(rand() % 25 + 97); }
    write(f, buffer, rsize*rnumber);
    free(buffer);
    close(f);
}

void syssort(const char* filename, int rnumber, size_t rsize){
    int f = open(filename, O_RDWR);
    if(f < 0){
        printf("Error while opening %s\n", filename);
        exit(-1);
    }

    unsigned char* key = calloc(rsize, sizeof(unsigned char));
    unsigned char* buff = calloc(rsize, sizeof(unsigned char));

    for(int i = 1; i < rnumber; i++){
        lseek(f, i*rsize, SEEK_SET);
        if(read(f, key, rsize) != rsize){ return; };
        lseek(f, (-2)*rsize, SEEK_CUR);
        if(read(f, buff, rsize) != rsize){ return; };
        int j = i;
        while(j > 1 && buff[0] > key[0]){
            if(write(f, buff, rsize) != rsize){ return; };
            lseek(f, (-3)*rsize, SEEK_CUR);
            if(read(f, buff, rsize) != rsize){ return; };
            j = j - 1;
        }
        if(buff[0] > key[0] && j == 1){
            if(write(f, buff, rsize) != rsize){ return; }
            lseek(f, (-2)*rsize, SEEK_CUR);
        }
        if(write(f, key, rsize) != rsize){ return; };
    }

    free(key);
    free(buff);
    close(f);
}

void syscopy(const char* from, const char* to, int rnumber, size_t rsize){
    int from_ptr = open(from, O_RDONLY);
    int to_ptr = open(to, O_CREAT | O_WRONLY, S_IRUSR|S_IWUSR);
    if(from_ptr < 0 || to_ptr < 0){
        printf("Error while opening %s or %s\n", from, to);
        exit(-1);
    }

    unsigned char* buffer = calloc(rsize, sizeof(unsigned char));
    for(int i = 0; i < rnumber; i++){
        if(read(from_ptr, buffer, rsize) != rsize){return;};
        if(write(to_ptr, buffer, rsize) != rsize){return;};
    }

    free(buffer);
    close(from_ptr);
    close(to_ptr);
}