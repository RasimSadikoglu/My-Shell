#include "../include/process_handler.h"

#include <stdlib.h>

typedef struct process_tree {
    pid_t pid;
    struct process_tree *next;
    struct process_tree *child;
} process_tree;

static process_tree *head = NULL;

void free_all(process_tree *root) {

    if (root->next != NULL) free_all(root->next);
    if (root->child != NULL) free_all(root->child);

    free(root);
}

void free_process_tree() { free_all(head); }

pid_t register_main(pid_t pid) {
    atexit(free_process_tree);

    head = calloc(sizeof(process_tree), 1);
    head->pid = pid;
    return pid;
}

pid_t register_child(pid_t pid, int bg) {

    process_tree *process = calloc(sizeof(process_tree), 1);
    process->pid = pid;

    if (head == NULL) return -1;

    if (!bg) {
        free(head->next);
        head->next = process;
    } else {
        process->next = head->child;
        head->child = process;
    }

    return pid;
}

pid_t unregister_child(pid_t pid) {

    if (head->child == NULL) return -1;

    if (head->child->pid == pid) {
        process_tree *next = head->child->next;
        free(head->child);
        head->child = next;
        return pid;
    }

    for (process_tree *it = head->child; it->next != NULL; it = it->next) {
        if (it->next->pid == pid) {
            process_tree *next = it->next->next;
            free(it->next);
            it->next = next;
            return pid;
        }
    }

    return -1;

}

int background_process_count() {
    
    return head->child != NULL;

}

void sigchld_handler(int sig) {

    pid_t child_pid = waitpid(-1, NULL, WNOHANG);

    if (child_pid <= 0) return;

    unregister_child(child_pid);
}

void sigtstp_handler(int sig) {

    for (process_tree *it = head->child; it != NULL; it = it->next) {
        kill(it->pid, SIGCONT);
    }

    if (head->next == NULL) return;

    pid_t child_pid = head->next->pid;

    free(head->next);
    head->next = NULL;
    
    kill(child_pid, SIGKILL);

}