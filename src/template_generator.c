#include "anvil.h"
#include "colors.h"
#include <sys/stat.h>
#include <libgen.h>

typedef struct {
    char project_name[128];
    char version[64];
    int need_src_folder;
    int need_include_folder;
    int multiple_executables;
    int use_current_dir;
} ProjectTemplate;

void get_current_directory_name(char *dir_name, size_t size) {
    char cwd[256];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        char *base = basename(cwd);
        strncpy(dir_name, base, size - 1);
        dir_name[size - 1] = '\0';
    } else {
        strcpy(dir_name, "myproject");
    }
}

int prompt_yes_no(const char *question) {
    char response[10];
    printf(BRIGHT_CYAN "%s" RESET " (y/n): ", question);
    fflush(stdout);

    if (fgets(response, sizeof(response), stdin) == NULL) {
        return 0;
    }

    trim(response);
    return (response[0] == 'y' || response[0] == 'Y');
}

void prompt_string(const char *question, char *buffer, size_t size, const char *default_value) {
    printf(BRIGHT_CYAN "%s" RESET, question);
    if (default_value && strlen(default_value) > 0) {
        printf(" [%s]", default_value);
    }
    printf(": ");
    fflush(stdout);

    if (fgets(buffer, size, stdin) == NULL) {
        if (default_value) {
            strcpy(buffer, default_value);
        } else {
            buffer[0] = '\0';
        }
        return;
    }

    trim(buffer);
    if (strlen(buffer) == 0 && default_value) {
        strcpy(buffer, default_value);
    }
}

void create_hello_world_main(const char *filepath, const char *version) {
    FILE *f = fopen(filepath, "w");
    if (!f) {
        printf(RED "Error: Cannot create %s" RESET "\n", filepath);
        return;
    }

    fprintf(f, "#include <stdio.h>\n\n");
    fprintf(f, "int main() {\n");
    fprintf(f, "    printf(\"Hello, World! v%%s\\n\", VERSION);\n");
    fprintf(f, "    return 0;\n");
    fprintf(f, "}\n");

    fclose(f);
    printf(GREEN "Created: %s" RESET "\n", filepath);
}

void create_single_target_config(const ProjectTemplate *tmpl) {
    FILE *f = fopen("build.conf", "w");
    if (!f) {
        printf(RED "Error: Cannot create build.conf" RESET "\n");
        return;
    }

    fprintf(f, "project = %s\n", tmpl->project_name);
    fprintf(f, "version = %s\n", tmpl->version);
    fprintf(f, "target = %s\n", tmpl->project_name);

    if (tmpl->need_src_folder) {
        fprintf(f, "sources = src/*\n");
    } else {
        fprintf(f, "sources = main.c\n");
    }

    if (tmpl->need_include_folder) {
        fprintf(f, "includes = include\n");
    }

    fprintf(f, "cflags = -Wall -O2 -std=c99\n");
    fprintf(f, "ldflags = -lm\n");
    fprintf(f, "output_dir = bin\n");

    fclose(f);
    printf(GREEN "Created: build.conf" RESET "\n");
}

void create_multi_target_config(const ProjectTemplate *tmpl) {
    FILE *f = fopen("build.conf", "w");
    if (!f) {
        printf(RED "Error: Cannot create build.conf" RESET "\n");
        return;
    }

    fprintf(f, "# Multi-target build configuration\n");
    fprintf(f, "project = %s\n", tmpl->project_name);
    fprintf(f, "version = %s\n", tmpl->version);
    fprintf(f, "cflags = -Wall -O2 -std=c99\n");

    if (tmpl->need_include_folder) {
        fprintf(f, "includes = include\n");
    }

    fprintf(f, "output_dir = bin\n\n");

    fprintf(f, "# Main executable\n");
    fprintf(f, "[target:main]\n");
    if (tmpl->need_src_folder) {
        fprintf(f, "sources = src/main.c\n");
    } else {
        fprintf(f, "sources = main.c\n");
    }
    fprintf(f, "ldflags = -lm\n");
    fprintf(f, "[/target]\n\n");

    fprintf(f, "# Additional executable (example)\n");
    fprintf(f, "[target:app2]\n");
    if (tmpl->need_src_folder) {
        fprintf(f, "sources = src/app2.c\n");
    } else {
        fprintf(f, "sources = app2.c\n");
    }
    fprintf(f, "ldflags = -lm\n");
    fprintf(f, "[/target]\n");

    fclose(f);
    printf(GREEN "Created: build.conf (multi-target)" RESET "\n");
}

int create_project_template(void) {
    ProjectTemplate tmpl = {0};
    char current_dir[128];

    printf(BRIGHT_YELLOW "=== Anvil Project Template Generator ===" RESET "\n\n");

    get_current_directory_name(current_dir, sizeof(current_dir));
    prompt_string("Enter project name (or '.' for current directory)", tmpl.project_name, sizeof(tmpl.project_name), "");

    if (strcmp(tmpl.project_name, ".") == 0 || strlen(tmpl.project_name) == 0) {
        tmpl.use_current_dir = 1;
        if (strlen(tmpl.project_name) == 0) {

            strcpy(tmpl.project_name, "project");
        } else {

            strcpy(tmpl.project_name, current_dir);
        }
    }

    prompt_string("Enter project version", tmpl.version, sizeof(tmpl.version), "1.0.0");

    tmpl.need_src_folder = prompt_yes_no("Do you need a src folder?");
    tmpl.need_include_folder = prompt_yes_no("Do you need an include folder?");
    tmpl.multiple_executables = prompt_yes_no("Does the project need multiple executables?");

    printf("\n" BRIGHT_CYAN "Creating project structure..." RESET "\n");

    if (!tmpl.use_current_dir) {
        if (create_directory(tmpl.project_name)) {
            printf(GREEN "Created directory: %s" RESET "\n", tmpl.project_name);
            if (chdir(tmpl.project_name) != 0) {
                printf(RED "Error: Cannot change to project directory" RESET "\n");
                return 0;
            }
        } else {
            printf(RED "Error: Cannot create project directory" RESET "\n");
            return 0;
        }
    }

    if (tmpl.need_src_folder) {
        if (create_directory("src")) {
            printf(GREEN "Created directory: src" RESET "\n");
        }
        create_hello_world_main("src/main.c", tmpl.version);

        if (tmpl.multiple_executables) {
            create_hello_world_main("src/app2.c", tmpl.version);
        }
    } else {

        create_hello_world_main("main.c", tmpl.version);

        if (tmpl.multiple_executables) {
            create_hello_world_main("app2.c", tmpl.version);
        }
    }

    if (tmpl.need_include_folder) {
        if (create_directory("include")) {
            printf(GREEN "Created directory: include" RESET "\n");
        }

        FILE *header = fopen("include/common.h", "w");
        if (header) {
            fprintf(header, "#ifndef COMMON_H\n");
            fprintf(header, "#define COMMON_H\n\n");
            fprintf(header, "#include <stdio.h>\n");
            fprintf(header, "#include <stdlib.h>\n\n");
            fprintf(header, "// Add your function declarations here\n\n");
            fprintf(header, "#endif // COMMON_H\n");
            fclose(header);
            printf(GREEN "Created: include/common.h" RESET "\n");
        }
    }

    if (tmpl.multiple_executables) {
        create_multi_target_config(&tmpl);
    } else {
        create_single_target_config(&tmpl);
    }

    printf("\n" BRIGHT_GREEN "Project template created successfully!" RESET "\n");
    printf(BRIGHT_YELLOW "Next steps:" RESET "\n");
    printf("  1. " CYAN "anvil build.conf" RESET "     # Generate Makefile\n");
    printf("  2. " CYAN "cd build" RESET "             # Enter build directory\n");
    printf("  3. " CYAN "make" RESET "                 # Compile your project\n");
    printf("  4. " CYAN "make run" RESET "             # Run your project\n");

    if (tmpl.multiple_executables) {
        printf("\n" BRIGHT_YELLOW "Multi-target commands:" RESET "\n");
        printf("  " CYAN "make main" RESET "            # Build main executable\n");
        printf("  " CYAN "make app2" RESET "            # Build app2 executable\n");
        printf("  " CYAN "make run-main" RESET "        # Run main executable\n");
        printf("  " CYAN "make run-app2" RESET "        # Run app2 executable\n");
    }

    return 1;
}