#include "anvil.h"

int parse_buildfile(const char *filename, BuildConfig *cfg) {
    FILE *f = fopen(filename, "r");
    if(!f) {
        fprintf(stderr, "Error: Cannot open %s\n", filename);
        return 0;
    }

    memset(cfg, 0, sizeof(BuildConfig));
    strcpy(cfg->output_dir, ".");
    strcpy(cfg->version, "1.0.0");  // Default version 

    char line[MAX_LINE];
    int in_target_block = 0;
    Target *current_target = NULL;

    while(fgets(line, MAX_LINE, f)) {
        trim(line);
        if(line[0] == '#' || line[0] == 0) continue;

        /* check for target block start */
        if(strncmp(line, "[target:", 8) == 0) {
            char *end = strchr(line + 8, ']');
            if(end && cfg->target_count < 16) {
                *end = 0;
                current_target = &cfg->targets[cfg->target_count];
                strcpy(current_target->name, line + 8);
                current_target->source_count = 0;
                current_target->ldflag_count = 0;
                cfg->target_count++;
                in_target_block = 1;
            }
            continue;
        }

        /* check for end of target block */
        if(strcmp(line, "[/target]") == 0) {
            in_target_block = 0;
            current_target = NULL;
            continue;
        }

        char *eq = strchr(line, '=');
        if(!eq) continue;

        *eq = 0;
        char *key = line;
        char *value = eq + 1;
        trim(key);
        trim(value);

        char *comment = strchr(value, '#');
        if(comment) {
            *comment = 0;
            trim(value);
        }

        /* handle target-specific properties */
        if(in_target_block && current_target) {
            if(strcmp(key, "sources") == 0) {
                char value_copy[MAX_LINE];
                strcpy(value_copy, value);
                char *token = strtok(value_copy, " \t");
                while(token != NULL && current_target->source_count < MAX_SOURCES) {
                    if(strchr(token, '*')) {
                        expand_glob(token, current_target->sources, &current_target->source_count, MAX_SOURCES);
                    } else {
                        strcpy(current_target->sources[current_target->source_count], token);
                        current_target->source_count++;
                    }
                    token = strtok(NULL, " \t");
                }
            } else if(strcmp(key, "ldflags") == 0) {
                parse_list(value, current_target->ldflags, &current_target->ldflag_count, MAX_FLAGS);
            }
        } else {
            if(strcmp(key, "project") == 0) {
                strcpy(cfg->project_name, value);
            } else if(strcmp(key, "version") == 0) {
                strcpy(cfg->version, value);
            } else if(strcmp(key, "target") == 0) {
                strcpy(cfg->target_name, value);
            } else if(strcmp(key, "sources") == 0) {
                char value_copy[MAX_LINE];
                strcpy(value_copy, value);
                char *token = strtok(value_copy, " \t");
                while(token != NULL && cfg->source_count < MAX_SOURCES) {
                    if(strchr(token, '*')) {
                        expand_glob(token, cfg->sources, &cfg->source_count, MAX_SOURCES);
                    } else {
                        strcpy(cfg->sources[cfg->source_count], token);
                        cfg->source_count++;
                    }
                    token = strtok(NULL, " \t");
                }
            } else if(strcmp(key, "includes") == 0) {
                parse_list(value, cfg->includes, &cfg->include_count, MAX_INCLUDES);
            } else if(strcmp(key, "cflags") == 0) {
                parse_list(value, cfg->cflags, &cfg->cflag_count, MAX_FLAGS);
            } else if(strcmp(key, "ldflags") == 0) {
                parse_list(value, cfg->ldflags, &cfg->ldflag_count, MAX_FLAGS);
            } else if(strcmp(key, "output_dir") == 0) {
                strcpy(cfg->output_dir, value);
            }
        }
    }

    fclose(f);

    /* if no targets defined but old-style config exists, create a single target for backward compatibility */
    if(cfg->target_count == 0 && strlen(cfg->target_name) > 0) {
        Target *compat_target = &cfg->targets[0];
        strcpy(compat_target->name, cfg->target_name);
        for(int i = 0; i < cfg->source_count; i++) {
            strcpy(compat_target->sources[i], cfg->sources[i]);
        }
        compat_target->source_count = cfg->source_count;
        for(int i = 0; i < cfg->ldflag_count; i++) {
            strcpy(compat_target->ldflags[i], cfg->ldflags[i]);
        }
        compat_target->ldflag_count = cfg->ldflag_count;
        cfg->target_count = 1;
    }

    return 1;
}