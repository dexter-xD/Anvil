#include "anvil.h"
#include "colors.h"

void show_config_content(const char *filename) {
    FILE *f = fopen(filename, "r");
    if(!f) {
        printf(YELLOW "Error: Cannot read %s" RESET "\n", filename);
        return;
    }

    printf("\n" BRIGHT_CYAN "Current configuration in " BOLD "%s" RESET BRIGHT_CYAN ":" RESET "\n", filename);
    printf(CYAN "────────────────────────────────────────" RESET "\n");

    char line[MAX_LINE];

    while(fgets(line, MAX_LINE, f)) {
        trim(line);

        if(line[0] != 0 && line[0] != '#') {

            char *eq = strchr(line, '=');
            if(eq) {
                *eq = 0;
                char *key = line;
                char *value = eq + 1;
                trim(key);
                trim(value);
                printf("  " BRIGHT_BLUE "%s" RESET " = " BRIGHT_GREEN "%s" RESET "\n", key, value);
                *eq = '='; 
            } else {
                printf("  %s\n", line);
            }
        }
    }
    printf(CYAN "────────────────────────────────────────" RESET "\n");
    fclose(f);
}

void print_config(BuildConfig *cfg) {
    printf("Project: %s\n", cfg->project_name);
    printf("Target: %s\n", cfg->target_name);
    printf("Sources: %d files\n", cfg->source_count);

    for(int i = 0; i < cfg->source_count; i++) {
        printf("  - %s\n", cfg->sources[i]);
    }
    printf("Includes: %d directories\n", cfg->include_count);

    if(strcmp(cfg->output_dir, ".") == 0) {
        printf("Output: build/%s\n", cfg->target_name);
    } else {
        printf("Output: build/%s/%s\n", cfg->output_dir, cfg->target_name);
    }
    printf("\n");
}

int setup_build_dirs() {
    if(!create_directory("build")) return 0;
    if(!create_directory("build/obj")) return 0;
    return 1;
}

int main(int argc, char *argv[]) {
    int watch = 0;
    int run_after_build = 0;
    char *config_file = NULL;

    if(argc < 2) {
        printf("Usage: %s [-w|-wr] <buildfile>\n", argv[0]);
        printf("\nOptions:\n");
        printf("  -w     Watch mode (auto-rebuild on file changes)\n");
        printf("  -wr    Watch & Run mode (auto-rebuild and run on file changes)\n");
        printf("\nExample buildfile format:\n");
        printf("  project = MyProject\n");
        printf("  target = myapp\n");
        printf("  sources = src/*\n");
        printf("  includes = include\n");
        printf("  cflags = -Wall -O2\n");
        printf("  ldflags = -lm\n");
        printf("  output_dir = bin\n");
        return 1;
    }

    for(int i = 1; i < argc; i++) {
        if(strcmp(argv[i], "-w") == 0) {
            watch = 1;
        } else if(strcmp(argv[i], "-wr") == 0) {
            watch = 1;
            run_after_build = 1;
        } else {
            config_file = argv[i];
        }
    }

    if(!config_file) {
        fprintf(stderr, "Error: No config file specified\n");
        return 1;
    }

    show_config_content(config_file);

    BuildConfig cfg;
    if(!parse_buildfile(config_file, &cfg)) {
        return 1;
    }

    if(!setup_build_dirs()) return 1;

    printf("\n");
    generate_makefile(&cfg);
    printf(BRIGHT_GREEN "Generated build/Makefile successfully!" RESET "\n");

    if(watch) {
        watch_mode(&cfg, run_after_build);
    } else {
        printf("\n" BRIGHT_YELLOW "To build your project:" RESET "\n");
        printf("  " CYAN "cd build" RESET "\n");
        printf("  " CYAN "make" RESET "           " DIM "# compile" RESET "\n");
        printf("  " CYAN "make run" RESET "       " DIM "# compile and run" RESET "\n");
        printf("  " CYAN "make clean" RESET "     " DIM "# clean build artifacts" RESET "\n");

        if(strcmp(cfg.output_dir, ".") == 0) {
            printf("\n" BRIGHT_BLUE "Executable will be in: " BOLD "build/%s" RESET "\n", cfg.target_name);
        } else {
            printf("\n" BRIGHT_BLUE "Executable will be in: " BOLD "build/%s/%s" RESET "\n", cfg.output_dir, cfg.target_name);
        }

        printf("\n" BRIGHT_CYAN "💡 Tips:" RESET "\n");
        printf("  • Use " MAGENTA "'./anvil -w %s'" RESET " for auto-rebuild\n", config_file);
        printf("  • Use " MAGENTA "'./anvil -wr %s'" RESET " for auto-rebuild and run\n", config_file);
    }

    return 0;
}