#ifndef ANVIL_H
#define ANVIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 512
#define MAX_SOURCES 256
#define MAX_FLAGS 32
#define MAX_INCLUDES 16

typedef struct {
    char project_name[128];
    char target_name[128];
    char sources[MAX_SOURCES][128];
    int source_count;
    char includes[MAX_INCLUDES][128];
    int include_count;
    char cflags[MAX_FLAGS][128];
    int cflag_count;
    char ldflags[MAX_FLAGS][128];
    int ldflag_count;
    char output_dir[128];
} BuildConfig;


int parse_buildfile(const char* filename, BuildConfig* cfg);
int create_directory(const char* path);
int create_directory_recursive(const char* path);
int is_c_file(const char* filename);
void expand_glob(const char* pattern, char dest[][128], int* count, int max);
void generate_makefile(BuildConfig* cfg);
void trim(char* str);
void parse_list(char* value, char dest[][128], int* count, int max);

#endif