#include "anvil.h"
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

void add_watch_file(const char *path, WatchFile *watch_files, int *watch_count) {
    if(*watch_count >= MAX_WATCH_FILES) return;
    
    strcpy(watch_files[*watch_count].path, path);
    watch_files[*watch_count].mtime = get_mtime(path);
    (*watch_count)++;
}

void scan_directory_for_headers(const char *dir, WatchFile *watch_files, int *watch_count) {
    DIR *d = opendir(dir);
    if(!d) return;
    
    struct dirent *entry;
    while((entry = readdir(d)) != NULL) {
        if(entry->d_name[0] == '.') continue;
        
        char fullpath[256];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, entry->d_name);
        
        struct stat st;
        if(stat(fullpath, &st) == 0 && S_ISDIR(st.st_mode)) {
            scan_directory_for_headers(fullpath, watch_files, watch_count);
        } else if(is_header_file(entry->d_name)) {
            add_watch_file(fullpath, watch_files, watch_count);
        }
    }
    
    closedir(d);
}

void setup_watch_list(BuildConfig *cfg, WatchFile *watch_files, int *watch_count) {
    *watch_count = 0;
    
    // watch source files
    for(int i = 0; i < cfg->source_count; i++) {
        add_watch_file(cfg->sources[i], watch_files, watch_count);
    }
    
    // watch header files in include directories
    for(int i = 0; i < cfg->include_count; i++) {
        scan_directory_for_headers(cfg->includes[i], watch_files, watch_count);
    }
    
    printf("\n🔍 Watching %d files for changes...\n", *watch_count);
}

int check_for_changes(WatchFile *watch_files, int watch_count) {
    for(int i = 0; i < watch_count; i++) {
        time_t current_mtime = get_mtime(watch_files[i].path);
        if(current_mtime != watch_files[i].mtime) {
            watch_files[i].mtime = current_mtime;
            return 1;
        }
    }
    return 0;
}

void print_timestamp(void) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    printf("[%02d:%02d:%02d] ", t->tm_hour, t->tm_min, t->tm_sec);
}

int run_make(int run_after_build) {
    printf("\n");
    print_timestamp();
    printf("🔨 Building...\n");
    
    int result = system("cd build && make 2>&1");
    
    if(result == 0) {
        print_timestamp();
        printf("✅ Build successful!\n");
        
        if(run_after_build) {
            printf("\n");
            print_timestamp();
            printf("🚀 Running...\n");
            printf("════════════════════════════════════════\n");
            system("cd build && make run 2>&1");
            printf("════════════════════════════════════════\n");
        }
    } else {
        print_timestamp();
        printf("❌ Build failed!\n");
    }
    
    return result;
}

void watch_mode(BuildConfig *cfg, int run_after_build) {
    WatchFile watch_files[MAX_WATCH_FILES];
    int watch_count = 0;
    
    printf("\n╔════════════════════════════════════════╗\n");
    if(run_after_build) {
        printf("║   🔥 Anvil Watch & Run Mode Active 🔥  ║\n");
    } else {
        printf("║     🔥 Anvil Watch Mode Active 🔥      ║\n");
    }
    printf("╚════════════════════════════════════════╝\n");
    
    setup_watch_list(cfg, watch_files, &watch_count);
    
    printf("\n💡 Press Ctrl+C to stop watching\n");
    
    run_make(run_after_build);
    
    // watch loop
    while(1) {
        sleep(1);
        
        if(check_for_changes(watch_files, watch_count)) {
            print_timestamp();
            printf("📝 File change detected!\n");
            run_make(run_after_build);
            printf("\n🔍 Watching for changes...\n");
        }
    }
}