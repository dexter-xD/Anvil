#include "anvil.h"

void trim(char *str) {
    char *start = str;
    char *end;
    while(isspace(*str)) str++;
    
    if(*str == 0) {
        *start = 0;
        return;
    }
    
    end = str + strlen(str) - 1;
    while(end > str && isspace(*end)) end--;
    *(end+1) = 0;
    
    if(str != start) {
        memmove(start, str, strlen(str) + 1);
    }
}

void parse_list(char *value, char dest[][128], int *count, int max) {
    char *token = strtok(value, " \t");
    *count = 0;
    while(token != NULL && *count < max) {
        strcpy(dest[*count], token);
        (*count)++;
        token = strtok(NULL, " \t");
    }
}