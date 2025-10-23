#ifndef ANVIL_H
#define ANVIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <ctype.h>

#define MAX_LINE 512
#define MAX_SOURCES 256
#define MAX_FLAGS 32
#define MAX_INCLUDES 16
#define MAX_WATCH_FILES 512

typedef struct {
    char name[128];
    char sources[MAX_SOURCES][128];
    int source_count;
    char ldflags[MAX_FLAGS][128];
    int ldflag_count;
} Target;

typedef struct {
    char project_name[128];
    char version[64];  // Project version from build.conf
    char target_name[128];  // Keep for backward compatibility
    char sources[MAX_SOURCES][128];  // Keep for backward compatibility
    int source_count;  // Keep for backward compatibility
    Target targets[16];  // Support up to 16 targets
    int target_count;
    char includes[MAX_INCLUDES][128];
    int include_count;
    char cflags[MAX_FLAGS][128];
    int cflag_count;
    char ldflags[MAX_FLAGS][128];  // Keep for backward compatibility
    int ldflag_count;  // Keep for backward compatibility
    char output_dir[128];
} BuildConfig;

typedef struct {
    char path[256];
    time_t mtime;
} WatchFile;


int parse_buildfile(const char *filename, BuildConfig *cfg);

/* file utils */ 
int create_directory(const char *path);
int create_directory_recursive(const char *path);
int is_c_file(const char *filename);
int is_header_file(const char *filename);
void expand_glob(const char *pattern, char dest[][128], int *count, int max);
time_t get_mtime(const char *path);

void generate_makefile(BuildConfig *cfg);

/* string utils */
void trim(char *str);
void parse_list(char *value, char dest[][128], int *count, int max);

/* watch system */ 
void add_watch_file(const char *path, WatchFile *watch_files, int *watch_count);
void scan_directory_for_headers(const char *dir, WatchFile *watch_files, int *watch_count);
void setup_watch_list(BuildConfig *cfg, WatchFile *watch_files, int *watch_count);
int check_for_changes(WatchFile *watch_files, int watch_count);
void print_timestamp(void);
int run_make(int run_after_build);
void watch_mode(BuildConfig *cfg, int run_after_build);

#endif