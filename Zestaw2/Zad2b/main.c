#define _DEFAULT_SOURCE
#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <zconf.h>
#include <stdlib.h>
#include <memory.h>
#include <ftw.h>

#define STR_EQUAL(x, y) strcmp(x, y) == 0

char *sign;
time_t usrdate;

static const int perms[] = {S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP, S_IWGRP, S_IXGRP, S_IROTH, S_IWOTH, S_IXOTH};
static const char keys[] = "rwxrwxrwx";

int compareData(time_t date1, time_t date2, const char* sign){
    double seconds = difftime(date1, date2);
    if(seconds > 0 && STR_EQUAL(sign, ">"))
        return 1;
    if(seconds < 0 && STR_EQUAL(sign, "<"))
        return 1;
    if(seconds == 0 && STR_EQUAL(sign, "="))
        return 1;
    return 0;
}

void getPermissions(const char* path, const struct stat *fstat){
    printf("%s", S_ISDIR(fstat->st_mode) ? "d" : "-");
    for(int i = 0; i < 9; i++){ printf("%c", (fstat->st_mode & perms[i]) ? keys[i] : '-'); }
}

int getFileInfo(const char* path, const struct stat *fstat, int tflag, struct FTW *tftw){

    if(tflag == FTW_F && compareData(fstat->st_mtime, usrdate, sign)) {
        char buffer[PATH_MAX];
        char mtime[30];
        getPermissions(path, fstat);
        printf(" %d ", (int) fstat->st_size);
        strftime(mtime, 30, "%Y-%m-%d %H:%M:%S", localtime(&fstat->st_mtime));
        printf(" %s ", mtime);
        printf(" %s\n", realpath(path, buffer));
    }
    return 0;
}

int main(int argc, char* argv[]) {
    if(argc != 4){
        printf("Wrong number of parameters.\n");
        exit(1);
    }

    char *date;
    struct tm tm;
    sign = argv[2];
    date = argv[3];

    strptime(date, "%d-%m-%Y", &tm);
    tm.tm_hour = 0;
    tm.tm_min = 0;
    tm.tm_sec = 0;
    tm.tm_isdst = -1;
    usrdate = mktime(&tm);
    nftw(argv[1], getFileInfo, 100, FTW_PHYS);

    return 0;
}
