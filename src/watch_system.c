#include "anvil.h"
#include "colors.h"

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

        char fullpath[512];  
        int result = snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, entry->d_name);
        if(result >= (int)sizeof(fullpath)) continue;  

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

    for(int i = 0; i < cfg->source_count; i++) {
        add_watch_file(cfg->sources[i], watch_files, watch_count);
    }

    for(int i = 0; i < cfg->include_count; i++) {
        scan_directory_for_headers(cfg->includes[i], watch_files, watch_count);
    }

    printf("\n" BRIGHT_CYAN "🔍 Watching " BOLD "%d" RESET BRIGHT_CYAN " files for changes..." RESET "\n", *watch_count);
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
    printf(DIM "[%02d:%02d:%02d]" RESET " ", t->tm_hour, t->tm_min, t->tm_sec);
}

int run_make(int run_after_build) {
    printf("\n");
    print_timestamp();
    printf(BRIGHT_YELLOW "🔨 Building..." RESET "\n\n");

    int result = system("cd build && make 2>&1");

    if(result == 0) {
        printf("\n");
        print_timestamp();
        printf(BRIGHT_GREEN "✅ Build successful!" RESET "\n");

        if(run_after_build) {
            printf("\n");
            print_timestamp();
            printf(BRIGHT_MAGENTA "🚀 Running..." RESET "\n");
            printf(DIM "────────────────────────────────────────" RESET "\n");

            system("cd build && make run 2>&1");

            printf(DIM "────────────────────────────────────────" RESET "\n");
            print_timestamp();
            printf(BRIGHT_CYAN "🏁 Program finished" RESET "\n");
        }
    } else {
        printf("\n");
        print_timestamp();
        printf(BRIGHT_RED "❌ Build failed!" RESET "\n");
    }

    return result;
}

void watch_mode(BuildConfig *cfg, int run_after_build) {
    WatchFile watch_files[MAX_WATCH_FILES];
    int watch_count = 0;

    printf("\n" BRIGHT_YELLOW "╔════════════════════════════════════════╗" RESET "\n");
    if(run_after_build) {
        printf(BRIGHT_YELLOW "║" RESET "   " BRIGHT_RED "🔥" RESET " " BOLD "Anvil Watch & Run Mode Active" RESET " " BRIGHT_RED "🔥" RESET "  " BRIGHT_YELLOW "║" RESET "\n");
    } else {
        printf(BRIGHT_YELLOW "║" RESET "     " BRIGHT_RED "🔥" RESET " " BOLD "Anvil Watch Mode Active" RESET " " BRIGHT_RED "🔥" RESET "      " BRIGHT_YELLOW "║" RESET "\n");
    }
    printf(BRIGHT_YELLOW "╚════════════════════════════════════════╝" RESET "\n");

    setup_watch_list(cfg, watch_files, &watch_count);

    printf("\n" BRIGHT_BLUE "💡 Press " BOLD "Ctrl+C" RESET BRIGHT_BLUE " to stop watching" RESET "\n");

    run_make(run_after_build);

    while(1) {
        sleep(1);

        if(check_for_changes(watch_files, watch_count)) {
            printf("\n");
            print_timestamp();
            printf(BRIGHT_CYAN "📝 File change detected!" RESET "\n");
            run_make(run_after_build);
            printf("\n" BRIGHT_CYAN "🔍 Watching for changes..." RESET "\n");
        }
    }
}