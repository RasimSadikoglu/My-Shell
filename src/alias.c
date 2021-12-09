#include "../include/alias.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define CHECK_USAGE(CONDITION) do { if (!(CONDITION)) { fprintf(stderr, "Wrong usage of alias!\n"); return 0; } } while (0)

enum find_options {OFIND, REPLACE};

static char ***list = NULL;
static int list_index = 0;

void alias_setup() {
    list = malloc(sizeof(char*));
    list[list_index] = NULL;
} 

void alias_free_all(void) {

    for (int i = 0; i < list_index; i++) {

        if (list[i] == NULL) continue;

        for (char **arg_it = list[i]; *arg_it != NULL; arg_it++) {
            free(*arg_it);
        }

        free(list[i]);
    }

    free(list);
}

void alias_free(int index) {

    if (list[index] == NULL) return;

    for (char **args_it = list[index]; *args_it != NULL; args_it++) {
        free(*args_it);
        *args_it = NULL;
    }

    free(list[index]);
    list[index] = NULL;

}

int alias_find(char *args[], int opt, int index) {
    for (int i = 0; i < list_index; i++) {

        if (list[i] == NULL) continue;

        if (strcmp(list[i][0], args[index])) continue;

        if (opt == OFIND) return i;

        int index = 0;
        for (char **arg_it = list[i] + 1; *arg_it != NULL; arg_it++) {

            args[index++] = *arg_it;
        }

        args[index] = NULL;
        
        break;
    }

    return opt == OFIND ? list_index : 1;
}

int alias_add_new_entry(char *args[]) {

    int argc = 0; for (char **arg_it = args + 1; *arg_it != NULL; arg_it++, argc++);

    CHECK_USAGE(argc >= 2);
    CHECK_USAGE(args[1][0] == '"');
    CHECK_USAGE(args[argc - 1][strlen(args[argc - 1]) - 1] == '"');

    int entry_index = alias_find(args, OFIND, argc);
    alias_free(entry_index);

    char **alias_args = malloc(sizeof(char*) * 2);
    int alias_index = 1;

    alias_args[0] = malloc(strlen(args[argc]) + 1);
    strcpy(alias_args[0], args[argc]);

    args[1] = args[1] + 1;
    args[argc - 1][strlen(args[argc - 1]) - 1] = '\0';

    if (args[1][0] == '\0') args[1] = NULL;
    if (args[argc - 1][0] == '\0') args[argc - 1] = NULL;

    for (int i = 1; i < argc; i++) {
        if (args[i] == NULL) continue;

        alias_args[alias_index] = malloc(strlen(args[i]) + 1);
        strcpy(alias_args[alias_index], args[i]);

        alias_index++;
        alias_args = realloc(alias_args, sizeof(char*) * (alias_index + 1));
        alias_args[alias_index] = NULL;
    }

    list[entry_index] = alias_args;

    if (entry_index == list_index) {
        list_index++;
        list = realloc(list, sizeof(char*) * (list_index + 1));
        list[list_index] = NULL;
    }
    
    return 0;
}

int alias_remove_entry(char *args[]) {
    
    int entry_index = alias_find(args, OFIND, 1);
    alias_free(entry_index);
    return 0;

}

int alias_list() {

    for (int i = 0; i < list_index; i++) {

        if (list[i] == NULL) continue;

        printf("%s \"", list[i][0]);

        for (char **arg_it = list[i] + 1; *arg_it != NULL; arg_it++) {

            printf("%s ", *arg_it);
        }

        printf("\b\"\n");
    }

    return 0;

}

int alias_handler(char *args[]) {
    if (list == NULL) {
        alias_setup();
        atexit(alias_free_all);
    }

    if (!strcmp(args[0], "alias")) {
        if (args[1] != NULL && !strcmp(args[1], "-l")) return alias_list();

        return alias_add_new_entry(args);
    }

    if (!strcmp(args[0], "unalias")) {
        return alias_remove_entry(args);
    }
    
    return alias_find(args, REPLACE, 0);
}