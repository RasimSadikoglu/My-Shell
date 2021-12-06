#include "../include/alias.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static char ***list = NULL;
static int list_index = 0;

void alias_setup() {
    list = malloc(sizeof(char*));
    list[list_index] = NULL;
} 

void alias_free(void) {

    for (char ***alias_it = list; *alias_it != NULL; alias_it++) {

        for (char **arg_it = *alias_it; *arg_it != NULL; arg_it++) {
            free(*arg_it);
        }

        free(*alias_it);
    }

    free(list);
}

int alias_add_new_entry(char *args[]) {

    int argc = 0; for (char **arg_it = args + 1; *arg_it != NULL; arg_it++, argc++);

    list[list_index] = malloc(sizeof(char*) * (argc + 1));
    list[list_index][argc] = NULL;

    list[list_index][0] = malloc(strlen(args[argc]) + 1);
    strcpy(list[list_index][0], args[argc]);

    // Remove "
    args[1] = args[1] + 1;
    args[argc - 1][strlen(args[argc - 1]) - 1] = '\0';

    for (int i = 1; i < argc; i++) {
        list[list_index][i] = malloc(strlen(args[i]) + 1);
        strcpy(list[list_index][i], args[i]);
    }

    list_index++;
    list = realloc(list, sizeof(char*) * (list_index + 1));
    list[list_index] = NULL;

    return 0;
}

int alias_find(char *args[]) {
    for (int i = 0; i < list_index; i++) {

        if (strcmp(list[i][0], args[0])) continue;

        int index = 0;
        for (char **arg_it = list[i] + 1; *arg_it != NULL; arg_it++) {

            args[index++] = *arg_it;
        }

        args[index] = NULL;
        
        break;
    }

    return 1;
}

int alias_remove_entry(char *args) {
    return 0;
}

int alias_list() {

    for (char ***alias_it = list; *alias_it != NULL; alias_it++) {

        printf("%s \"", (*alias_it)[0]);

        for (char **arg_it = *alias_it + 1; *arg_it != NULL; arg_it++) {

            printf("%s ", *arg_it);
        }

        printf("\"\n");
    }

    return 0;

}

int alias_handler(char *args[]) {
    if (list == NULL) {
        alias_setup();
        atexit(alias_free);
    }

    if (!strcmp(args[0], "alias")) {
        if (!strcmp(args[1], "-l")) return alias_list();

        return alias_add_new_entry(args);
    }

    if (!strcmp(args[0], "unalias")) {
        return alias_remove_entry(args[1]);
    }

    return alias_find(args);
}