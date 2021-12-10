#include "../include/process_handler.h"

static int background_process = 0;
static pid_t foreground_process = -1;

pid_t register_child(pid_t pid, int bg) {

    if (bg) background_process++;
    else foreground_process = pid;

    return pid;

}

void sigchld_handler(int sig) {

    pid_t child_pid = waitpid(-1, NULL, WNOHANG);

    if (child_pid <= 0) return;

    background_process--;
}

void sigtstp_handler(int sig) {

    if (foreground_process == -1) return;

    kill(foreground_process, SIGKILL);

    foreground_process = -1;
}

int background_process_count() {
    return background_process;
}