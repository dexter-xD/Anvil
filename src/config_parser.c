#include "anvil.h"

int parse_buildfile(const char *filename, BuildConfig *cfg) {
    FILE *f = fopen(filename, "r");
    if(!f) {
        fprintf(stderr, "Error: Cannot open %s\n", filename);
        return 0;
    }

    memset(cfg, 0, sizeof(BuildConfig));
    strcpy(cfg->output_dir, "."); 

    char line[MAX_LINE];

    while(fgets(line, MAX_LINE, f)) {
        trim(line);
        if(line[0] == '#' || line[0] == 0) continue;

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

        if(strcmp(key, "project") == 0) {
            strcpy(cfg->project_name, value);
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

    fclose(f);
    return 1;
}