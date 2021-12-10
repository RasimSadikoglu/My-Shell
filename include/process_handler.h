#ifndef PROCESS_HANDLER_H
#define PROCESS_HANDLER_H

#include <unistd.h>
#include <sys/wait.h>

pid_t register_child(pid_t pid, int bg);

int background_process_count();

void sigchld_handler(int sig);
void sigtstp_handler(int sig);

#endif // PROCESS_HANDLER_H