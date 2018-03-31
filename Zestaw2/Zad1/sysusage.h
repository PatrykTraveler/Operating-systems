//
// Created by traveler on 18.03.18.
//

#ifndef ZADANIE2_SYSUSAGE_H
#define ZADANIE2_SYSUSAGE_H

#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>

void sysgenerate(const char* filename, int rnumber, size_t rsize);
void syssort(const char* filename, int rnumber, size_t rsize);
void syscopy(const char* from, const char* to, int rnumber, size_t rsize);

#endif //ZADANIE2_SYSUSAGE_H
