#define STR_EQUAL(x, y) strcmp(x, y) == 0
#define _DEFAULT_SOURCE
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>

static const int perms[] = {S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP, S_IWGRP, S_IXGRP, S_IROTH, S_IWOTH, S_IXOTH};
static const char keys[] = {'r','w','x','r','w','x','r','w','x'};

int compareData(time_t date1, time_t date2, const char* sign){
    double seconds = difftime(date1, date2);
    return seconds > 0 && STR_EQUAL(sign, ">") ? 1 : seconds < 0 && STR_EQUAL(sign, "<") ? 1 : seconds == 0 && STR_EQUAL(sign, "=") ? 1 : 0;
}

const char* getPermissions(const char* path, const struct stat *fstat){
    printf("%s", S_ISDIR(fstat->st_mode) ? "d" : "-");
    for(int i = 0; i < 9; i++){ printf("%c", (fstat->st_mode & perms[i]) ? keys[i] : '-'); }
}

void getFileInfo(const char* path, const struct stat *fstat){
    char buffer[PATH_MAX];
    char mtime[30];

    getPermissions(path, fstat);
    printf(" %d ", (int)fstat->st_size);
    strftime(mtime, 30, "%Y-%m-%d %H:%M:%S", localtime(&fstat->st_mtime));
    printf(" %s ", mtime);
    printf(" %s\n", realpath(path, buffer));
}

void listdir(const char *name, time_t date, const char* sign)
{
    DIR *dir;
    struct dirent *entry;
    struct stat fstat;

    if ((dir = opendir(name)) == NULL)
        return;

    while ((entry = readdir(dir)) != NULL) {
        char path[PATH_MAX];
        if (STR_EQUAL(entry->d_name, ".") || STR_EQUAL(entry->d_name, ".."))
            continue;
        snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
        if(lstat(path, &fstat) >= 0){
            if(S_ISDIR(fstat.st_mode) && compareData(fstat.st_mtime, date, sign)){
                if(!vfork()) {
                    getFileInfo(path, &fstat);
                    listdir(path, date, sign);
                    exit(0);
                }
            }
            else if(S_ISREG(fstat.st_mode))
                getFileInfo(path, &fstat);
        }
    }
    closedir(dir);
}

int main(int argc, char* argv[]) {
    if(argc != 4){
        printf("Wrong number of parameters.");
        exit(1);
    }

    char *date, *path, *sign;
    char datebuff[20];
    struct tm tm;
    path = argv[1];
    sign = argv[2];
    date = argv[3];

    strptime(date, "%d-%m-%Y", &tm);
    tm.tm_hour = 0;
    tm.tm_min = 0;
    tm.tm_sec = 0;
    tm.tm_isdst = -1;
    time_t time = mktime(&tm);
    listdir(".", time, sign);

    return 0;
}