#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <getopt.h>
#include <sys/stat.h>
#include <ftw.h>
#include <sys/types.h>
#include <pwd.h>
#include <zconf.h>
#include <dirent.h>
#include <limits.h>

struct tm *searchedDate;
int compareType;
int accesRights[] = {S_IRUSR, S_IWUSR, S_IXUSR,
                     S_IRGRP, S_IWGRP, S_IXGRP,
                     S_IROTH, S_IWOTH, S_IXOTH};

char accesRigthSymbols[] = {'r', 'w', 'x',
                            'r', 'w', 'x',
                            'r', 'w', 'x'};



int compare(int y, int x){
    return x > y ? 1 : x == y ? 0 : -1;
}

int compareDate(struct tm *fileDate){

    return searchedDate->tm_year != (fileDate->tm_year + 1900) ? compare(searchedDate->tm_year, fileDate->tm_year + 1900) :
           searchedDate->tm_mon != (fileDate->tm_mon + 1)? compare(searchedDate->tm_mon, fileDate->tm_mon + 1) :
           searchedDate->tm_mday != fileDate->tm_mday ? compare(searchedDate->tm_mday, fileDate->tm_mday) :
           searchedDate->tm_hour != fileDate->tm_hour ? compare(searchedDate->tm_hour, fileDate->tm_hour) :
           searchedDate->tm_min != fileDate->tm_min ? compare(searchedDate->tm_min, fileDate->tm_min) :
           0;
}


int printFileInformation(const char *fpath, const struct stat *fileStat, int typeFlag, struct FTW *ftwbuf){
    struct tm *fileDate = calloc(1, sizeof(struct tm));
    localtime_r(&fileStat->st_mtime, fileDate);
    struct passwd *pw = getpwuid(fileStat->st_uid);
    char timeString[15];
    char pathBuffer[PATH_MAX];
    if (compareDate(fileDate) != compareType) return 0;

    //File type
    printf("%c", (typeFlag == FTW_D) ? 'd' :
                 (typeFlag == FTW_SL) ? 'l' : '-' );

    int mask = fileStat->st_mode;
    //Access Rights
    for (int i = 0; i < 9; ++i) {
        printf("%c", mask & accesRights[i] ? accesRigthSymbols[i] : '-' );
    }
    //Owner
    printf(" %s", pw->pw_name);
    //Size
    if (typeFlag == FTW_D) {
        printf(" %6c",'-');
    } else if (fileStat->st_size > 1000){
        printf(" %3d,%dK", (int) fileStat->st_size / 1000, ((int) fileStat->st_size % 1000) / 100);
    } else {
        printf(" %6d", (int) fileStat->st_size);
    }
    //Last modification date
    strftime(timeString, sizeof(timeString), "%m-%d %H:%M", fileDate);
    printf(" %s", timeString);

    //Real path
    char *realPath = realpath(fpath, pathBuffer);
    printf(" %s", realPath);

    printf("\n");
    return 0;
}


void lsRecursve(char *fPath) {
    DIR *direcotry = opendir(fPath);
    if (direcotry == NULL) {
        printf("No such directory was found\n");
        return; }

    struct dirent *dirEntry;
    struct stat fileStat;
    char *path = calloc(PATH_MAX, sizeof(char));
    strcpy(path, fPath);



    while ((dirEntry = readdir(direcotry)) != NULL) {
        strcpy(path, fPath);
        strcat(path, "/");
        strcat(path, dirEntry->d_name);

        if ((strcmp(dirEntry->d_name, ".") == 0 || strcmp(dirEntry->d_name, "..") == 0)) continue;

        if(lstat(path, &fileStat) >= 0) {
            if (S_ISDIR(fileStat.st_mode)) {
                printFileInformation(path, &fileStat, FTW_D, NULL);
                lsRecursve(path);
            } else if(S_ISREG(fileStat.st_mode)) {
                printFileInformation(path, &fileStat, FTW_F, NULL);
            } else if(S_ISLNK(fileStat.st_mode)){
                printFileInformation(path, &fileStat, FTW_SL, NULL);
            }
        }
    }

    closedir(direcotry);
}

int main(int argc, char **argv) {

    searchedDate = calloc(1, sizeof(struct tm));
    searchedDate->tm_year = 0;
    searchedDate->tm_mon = 0;
    searchedDate->tm_mday = 0;
    searchedDate->tm_hour = 0;
    searchedDate->tm_min = 0;
    searchedDate->tm_sec = 0;


    struct tm *tmpDate = calloc(1, sizeof(struct tm));
    time_t t = time(NULL);
    time_t t2 = mktime(searchedDate);


    searchedDate->tm_sec = 0;
    char *path = NULL;
    int optionIndex = 0;
    int shortOption = 0;


    while (1)
    {
        static struct option long_options[] =
                {
                        {"path",    required_argument, 0, 'p'},
                        {"compare", required_argument, 0, 'c'},
                        {"year",    required_argument, 0, 'y'},
                        {"month",   required_argument, 0, 'M'},
                        {"day",     required_argument, 0, 'd'},
                        {"hour",    required_argument, 0, 'h'},
                        {"min",     required_argument, 0, 'm'}
                };
        shortOption = getopt_long (argc, argv, "p:c:y:M:d:h:m:", long_options, &optionIndex);

        if (shortOption == -1)
            break;
        switch (shortOption){
            case 0:
                break;
            case 'p':
                path = optarg;
                break;
            case 'c':
                compareType = strcmp("<", optarg) == 0 ?  -1 :
                              strcmp("=", optarg) == 0 ?  0 :
                              strcmp(">", optarg) == 0 ?  1 :
                              303;
                if (compareType == 303) abort();
                break;
            case 'y':
                searchedDate->tm_year = (int) strtol(optarg, '\0', 10);
                break;
            case 'M':
                searchedDate->tm_mon = (int) strtol(optarg, '\0', 10);
                break;
            case 'd':
                searchedDate->tm_mday = (int) strtol(optarg, '\0', 10);
                break;
            case 'h':
                searchedDate->tm_hour = (int) strtol(optarg, '\0', 10);
                break;
            case 'm':
                searchedDate->tm_min = (int) strtol(optarg, '\0', 10);
                break;
            default:
                abort();
        }

    }

#ifdef NFTW
    nftw(path, printFileInformation, 10, FTW_PHYS);
#endif
#ifdef SYS
    lsRecursve(path);
#endif
    return 0;
}