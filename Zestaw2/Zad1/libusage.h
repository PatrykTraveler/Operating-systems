#ifndef ZADANIE2_SYSLIB_H
#define ZADANIE2_SYSLIB_H

#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>

void libgenerate(const char* filename, int rnumber, size_t rsize);
void libsort(const char* filename, int rnumber, size_t rsize);
void libcopy(const char* from, const char* to, int rnumber, size_t rsize);

#endif //ZADANIE2_SYSLIB_H
