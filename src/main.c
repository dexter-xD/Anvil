#include "anvil.h"

int main(int argc, char *argv[]) {
    
    BuildConfig cfg;
    if(!parse_buildfile(argv[1], &cfg)) {
        return 1;
    }
    
    printf("Project: %s\n", cfg.project_name);
    printf("Target: %s\n", cfg.target_name);
    printf("Sources: %d files\n", cfg.source_count);
    for(int i = 0; i < cfg.source_count; i++) {
        printf("  - %s\n", cfg.sources[i]);
    }
    printf("Includes: %d directories\n", cfg.include_count);
    
    if(strcmp(cfg.output_dir, ".") == 0) {
        printf("Output: build/%s\n", cfg.target_name);
    } else {
        printf("Output: build/%s/%s\n", cfg.output_dir, cfg.target_name);
    }
    printf("\n");
    
    // Create build directory structure
    if(!create_directory("build")) return 1;
    if(!create_directory("build/obj")) return 1;
    
    printf("\n");
    generate_makefile(&cfg);
    
    printf("\nTo build your project:\n");
    printf("  cd build\n");
    printf("  make\n");
    printf("  make run    # to build and run\n");
    printf("  make clean  # to clean\n");
    
    if(strcmp(cfg.output_dir, ".") == 0) {
        printf("\nExecutable will be in: build/%s\n", cfg.target_name);
    } else {
        printf("\nExecutable will be in: build/%s/%s\n", cfg.output_dir, cfg.target_name);
    }
    
    return 0;
}