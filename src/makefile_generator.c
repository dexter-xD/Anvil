#include "anvil.h"

void generate_makefile(BuildConfig *cfg) {
    FILE *f = fopen("build/Makefile", "w");
    if(!f) {
        fprintf(stderr, "Error: Cannot create build/Makefile\n");
        return;
    }

    fprintf(f, "# Generated Makefile for %s\n\n", cfg->project_name);
    fprintf(f, "CC = gcc\n");
    fprintf(f, "TARGET = %s\n", cfg->target_name);
    fprintf(f, "OBJ_DIR = obj\n");

    int use_bin_dir = strcmp(cfg->output_dir, ".") != 0;
    if(use_bin_dir) {
        fprintf(f, "BIN_DIR = %s\n", cfg->output_dir);
    }
    fprintf(f, "SRC_DIR = ..\n\n");

    fprintf(f, "CFLAGS =");
    for(int i = 0; i < cfg->cflag_count; i++)
        fprintf(f, " %s", cfg->cflags[i]);

    for(int i = 0; i < cfg->include_count; i++)
        fprintf(f, " -I$(SRC_DIR)/%s", cfg->includes[i]);
    fprintf(f, "\n");

    fprintf(f, "LDFLAGS =");
    for(int i = 0; i < cfg->ldflag_count; i++)
        fprintf(f, " %s", cfg->ldflags[i]);
    fprintf(f, "\n\n");

    fprintf(f, "SOURCES =");
    for(int i = 0; i < cfg->source_count; i++)
        fprintf(f, " $(SRC_DIR)/%s", cfg->sources[i]);
    fprintf(f, "\n\n");

    fprintf(f, "OBJECTS =");
    for(int i = 0; i < cfg->source_count; i++) {
        char basename[128];
        strcpy(basename, cfg->sources[i]);
        char *last_slash = strrchr(basename, '/');
        char *name = last_slash ? last_slash + 1 : basename;
        char *dot = strrchr(name, '.');
        if(dot) *dot = 0;
        fprintf(f, " $(OBJ_DIR)/%s.o", name);
    }
    fprintf(f, "\n\n");

    if(use_bin_dir) {
        fprintf(f, "TARGET_PATH = $(BIN_DIR)/$(TARGET)\n\n");
        fprintf(f, "all: $(TARGET_PATH)\n\n");
        fprintf(f, "$(TARGET_PATH): $(OBJECTS) | $(BIN_DIR)\n");
    } else {
        fprintf(f, "all: $(TARGET)\n\n");
        fprintf(f, "$(TARGET): $(OBJECTS)\n");
    }

    fprintf(f, "\t$(CC) $(OBJECTS) -o ");
    if(use_bin_dir) {
        fprintf(f, "$(TARGET_PATH)");
    } else {
        fprintf(f, "$(TARGET)");
    }
    fprintf(f, " $(LDFLAGS)\n");
    fprintf(f, "\t@echo \"Build complete: ");
    if(use_bin_dir) {
        fprintf(f, "$(TARGET_PATH)");
    } else {
        fprintf(f, "$(TARGET)");
    }
    fprintf(f, "\"\n\n");

    for(int i = 0; i < cfg->source_count; i++) {
        char basename[128];
        strcpy(basename, cfg->sources[i]);
        char *last_slash = strrchr(basename, '/');
        char *name = last_slash ? last_slash + 1 : basename;
        char *dot = strrchr(name, '.');
        if(dot) *dot = 0;

        fprintf(f, "$(OBJ_DIR)/%s.o: $(SRC_DIR)/%s | $(OBJ_DIR)\n", name, cfg->sources[i]);
        fprintf(f, "\t$(CC) $(CFLAGS) -c $< -o $@\n\n");
    }

    fprintf(f, "$(OBJ_DIR):\n");
    fprintf(f, "\tmkdir -p $(OBJ_DIR)\n\n");

    if(use_bin_dir) {
        fprintf(f, "$(BIN_DIR):\n");
        fprintf(f, "\tmkdir -p $(BIN_DIR)\n\n");
    }

    fprintf(f, "run: ");
    if(use_bin_dir) {
        fprintf(f, "$(TARGET_PATH)\n");
    } else {
        fprintf(f, "$(TARGET)\n");
    }
    fprintf(f, "\t@echo \"Running $(TARGET)...\"\n");
    fprintf(f, "\t@echo \"\"\n");
    fprintf(f, "\t@");
    if(use_bin_dir) {
        fprintf(f, "$(TARGET_PATH)");
    } else {
        fprintf(f, "./$(TARGET)");
    }
    fprintf(f, "\n\n");

    fprintf(f, "clean:\n");
    fprintf(f, "\trm -rf $(OBJ_DIR) $(TARGET)");
    if(use_bin_dir) {
        fprintf(f, " $(BIN_DIR)");
    }
    fprintf(f, "\n");
    fprintf(f, "\t@echo \"Clean complete\"\n\n");

    fprintf(f, ".PHONY: all clean run\n");

    fclose(f);
    printf("Generated build/Makefile successfully!\n");
}