#include "anvil.h"

void generate_makefile(BuildConfig *cfg) {
    FILE *f = fopen("build/Makefile", "w");
    if(!f) {
        fprintf(stderr, "Error: Cannot create build/Makefile\n");
        return;
    }

    fprintf(f, "# Generated Makefile for %s\n\n", cfg->project_name);
    fprintf(f, "CC = gcc\n");
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
    fprintf(f, "\n\n");

    /* Generate variables and rules for each target */
    fprintf(f, "# Targets\n");
    fprintf(f, "TARGETS =");
    for(int t = 0; t < cfg->target_count; t++) {
        if(use_bin_dir) {
            fprintf(f, " $(BIN_DIR)/%s", cfg->targets[t].name);
        } else {
            fprintf(f, " %s", cfg->targets[t].name);
        }
    }
    fprintf(f, "\n\n");

    fprintf(f, "all: $(TARGETS)\n\n");

    /* generate rules for each target */
    for(int t = 0; t < cfg->target_count; t++) {
        Target *target = &cfg->targets[t];
        
        fprintf(f, "# Target: %s\n", target->name);
        fprintf(f, "%s_SOURCES =", target->name);
        for(int i = 0; i < target->source_count; i++)
            fprintf(f, " $(SRC_DIR)/%s", target->sources[i]);
        fprintf(f, "\n");

        fprintf(f, "%s_OBJECTS =", target->name);
        for(int i = 0; i < target->source_count; i++) {
            char basename[128];
            strcpy(basename, target->sources[i]);
            char *last_slash = strrchr(basename, '/');
            char *name = last_slash ? last_slash + 1 : basename;
            char *dot = strrchr(name, '.');
            if(dot) *dot = 0;
            fprintf(f, " $(OBJ_DIR)/%s_%s.o", target->name, name);
        }
        fprintf(f, "\n");

        fprintf(f, "%s_LDFLAGS =", target->name);
        for(int i = 0; i < target->ldflag_count; i++)
            fprintf(f, " %s", target->ldflags[i]);
        fprintf(f, "\n\n");

        if(use_bin_dir) {
            fprintf(f, "$(BIN_DIR)/%s: $(%s_OBJECTS) | $(BIN_DIR)\n", target->name, target->name);
            fprintf(f, "\t$(CC) $(%s_OBJECTS) -o $(BIN_DIR)/%s $(%s_LDFLAGS)\n", target->name, target->name, target->name);
            fprintf(f, "\t@echo \"Build complete: $(BIN_DIR)/%s\"\n\n", target->name);
        } else {
            fprintf(f, "%s: $(%s_OBJECTS)\n", target->name, target->name);
            fprintf(f, "\t$(CC) $(%s_OBJECTS) -o %s $(%s_LDFLAGS)\n", target->name, target->name, target->name);
            fprintf(f, "\t@echo \"Build complete: %s\"\n\n", target->name);
        }

        for(int i = 0; i < target->source_count; i++) {
            char basename[128];
            strcpy(basename, target->sources[i]);
            char *last_slash = strrchr(basename, '/');
            char *name = last_slash ? last_slash + 1 : basename;
            char *dot = strrchr(name, '.');
            if(dot) *dot = 0;

            fprintf(f, "$(OBJ_DIR)/%s_%s.o: $(SRC_DIR)/%s | $(OBJ_DIR)\n", target->name, name, target->sources[i]);
            fprintf(f, "\t$(CC) $(CFLAGS) -c $< -o $@\n\n");
        }
    }

    fprintf(f, "$(OBJ_DIR):\n");
    fprintf(f, "\tmkdir -p $(OBJ_DIR)\n\n");

    if(use_bin_dir) {
        fprintf(f, "$(BIN_DIR):\n");
        fprintf(f, "\tmkdir -p $(BIN_DIR)\n\n");
    }

    /* generate run targets for each executable */
    for(int t = 0; t < cfg->target_count; t++) {
        Target *target = &cfg->targets[t];
        fprintf(f, "run-%s: ", target->name);
        if(use_bin_dir) {
            fprintf(f, "$(BIN_DIR)/%s\n", target->name);
        } else {
            fprintf(f, "%s\n", target->name);
        }
        fprintf(f, "\t@echo \"Running %s...\"\n", target->name);
        fprintf(f, "\t@echo \"\"\n");
        fprintf(f, "\t@");
        if(use_bin_dir) {
            fprintf(f, "$(BIN_DIR)/%s", target->name);
        } else {
            fprintf(f, "./%s", target->name);
        }
        fprintf(f, "\n\n");
    }

    /* backward compatibility: if only one target, create 'run' alias */
    if(cfg->target_count == 1) {
        fprintf(f, "run: run-%s\n\n", cfg->targets[0].name);
    }

    fprintf(f, "clean:\n");
    fprintf(f, "\trm -rf $(OBJ_DIR) $(TARGETS)\n");
    fprintf(f, "\t@echo \"Clean complete\"\n\n");

    fprintf(f, ".PHONY: all clean");
    for(int t = 0; t < cfg->target_count; t++) {
        fprintf(f, " run-%s", cfg->targets[t].name);
    }
    if(cfg->target_count == 1) {
        fprintf(f, " run");
    }
    fprintf(f, "\n");

    fclose(f);
}