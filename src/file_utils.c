#include "anvil.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

int is_c_file(const char *filename) {
    size_t len = strlen(filename);
    return len > 2 && strcmp(filename + len - 2, ".c") == 0;
}

void expand_glob(const char *pattern, char dest[][128], int *count, int max) {
    char path[256], wildcard[128];
    strcpy(path, pattern);

    char *star = strchr(path, '*');
    if(!star) {

        if(*count < max) {
            strcpy(dest[*count], pattern);
            (*count)++;
        }
        return;
    }

    char *last_slash = strrchr(path, '/');
    if(last_slash) {
        *last_slash = 0;
        strcpy(wildcard, last_slash + 1);
    } else {
        strcpy(path, ".");
        strcpy(wildcard, pattern);
    }

    DIR *dir = opendir(path);
    if(!dir) {
        fprintf(stderr, "Warning: Cannot open directory %s\n", path);
        return;
    }

    struct dirent *entry;
    while((entry = readdir(dir)) != NULL && *count < max) {
        if(entry->d_name[0] == '.') continue;

        if(!is_c_file(entry->d_name)) continue;

        char fullpath[256];
        if(strcmp(path, ".") == 0) {
            snprintf(fullpath, sizeof(fullpath), "%s", entry->d_name);
        } else {
            snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);
        }

        strcpy(dest[*count], fullpath);
        (*count)++;
    }

    closedir(dir);
}

int create_directory(const char *path) {
    struct stat st = {0};
    if(stat(path, &st) == -1) {
        #ifdef _WIN32
            if(mkdir(path) != 0) {
        #else
            if(mkdir(path, 0755) != 0) {
        #endif
            fprintf(stderr, "Error: Cannot create directory %s\n", path);
            return 0;
        }
        printf("Created directory: %s\n", path);
    } else {
        printf("Directory exists: %s\n", path);
    }
    return 1;
}

int create_directory_recursive(const char *path) {
    char tmp[256];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp), "%s", path);
    len = strlen(tmp);
    if(tmp[len - 1] == '/')
        tmp[len - 1] = 0;

    for(p = tmp + 1; *p; p++) {
        if(*p == '/') {
            *p = 0;
            #ifdef _WIN32
                mkdir(tmp);
            #else
                mkdir(tmp, 0755);
            #endif
            *p = '/';
        }
    }

    #ifdef _WIN32
        return mkdir(tmp) == 0 || errno == EEXIST;
    #else
        return mkdir(tmp, 0755) == 0 || errno == EEXIST;
    #endif
}