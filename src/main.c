#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>

#include "../include/alias.h"

#define IFEXIT(CONDITION, MSG, CODE) do { if ((CONDITION)) { perror((MSG)); exit((CODE)); } } while (0)

#define STYLE "\033[1;96m"
#define RESET "\033[0m"
 
#define MAX_LINE 200 /* 80 chars per line, per command, should be enough. */
 
/* The setup function below will not return any value, but it will just: read
in the next command line; separate it into distinct arguments (using blanks as
delimiters), and set the args array entries to point to the beginning of what
will become null-terminated, C-style strings. */

void setup(char inputBuffer[], char *args[], int *background) {
    int length, /* # of characters in the command line */
        i,      /* loop index for accessing inputBuffer array */
        start,  /* index where beginning of next command parameter is */
        ct;     /* index of where to place the next parameter into args[] */
    
    ct = 0;
        
    /* read what the user enters on the command line */
    length = read(STDIN_FILENO, inputBuffer, MAX_LINE);  

    /* 0 is the system predefined file descriptor for stdin (standard input),
       which is the user's screen in this case. inputBuffer by itself is the
       same as &inputBuffer[0], i.e. the starting address of where to store
       the command that is read, and length holds the number of characters
       read in. inputBuffer is not a null terminated C-string. */

    start = -1;
    if (length == 0) {
        printf("\nProcess Terminated!\n");
        exit(0); /* ^d was entered, end of user command stream */
    }

    /* the signal interrupted the read system call */
    /* if the process is in the read() system call, read returns -1
    However, if this occurs, errno is set to EINTR. We can check this  value
    and disregard the -1 value */
    IFEXIT((length < 0) && (errno != EINTR), "Error reading the command\n", -1);

    for (i = 0; i < length; i++) { /* examine every character in the inputBuffer */
        switch (inputBuffer[i]) {
	        case ' ':
	        case '\t' :               /* argument separators */
		        if(start != -1) {
                    args[ct] = &inputBuffer[start];    /* set up pointer */
		            ct++;
		        }
                inputBuffer[i] = '\0'; /* add a null char; make a C string */
		        start = -1;
		        break;
            case '\n':                 /* should be the final char examined */
		        if (start != -1) {
                    args[ct] = &inputBuffer[start];     
		            ct++;
		        }
                inputBuffer[i] = '\0';
                args[ct] = NULL; /* no more arguments to this command */
		        break;
	        default :             /* some other character */
		        if (start == -1) start = i;
                if (inputBuffer[i] == '&') {
		            *background  = 1;
                    inputBuffer[i-1] = '\0';
		        }
	    } /* end of switch */
    }    /* end of for */
    args[ct] = NULL; /* just in case the input line was > 80 */
    if (*background) args[ct - 1] = NULL;
} /* end of setup routine */

int path_find(char arg[], char path[]) {

    const char *paths_address = getenv("PATH");

    char paths[strlen(paths_address) + 1];
    strcpy(paths, paths_address);

    char *current_path = paths, *next_path;

    while (strtok_r(current_path, ":", &next_path) != NULL) {

        char location[200];
        sprintf(location, "%s/%s", current_path, arg);

        if (!access(location, F_OK)) {
            strcpy(path, location);
            return 1;
        }

        current_path = next_path;
    }

    strcpy(path, arg);
    
    return 1;
}

int redirect(char *args[]) {

    for (char **it = args; *it != NULL; it++) {

        if (!strcmp(*it, "<")) {
            int fd = open(*(it + 1), O_RDONLY);

            IFEXIT(dup2(fd, STDIN_FILENO) == -1, "Failed to redirect\n", EXIT_FAILURE);

            close(fd);

            *it = NULL;
        } else if (!strcmp(*it, ">") || !strcmp(*it, ">>")) {
            int flags = O_WRONLY | O_CREAT;

            if ((*it)[1] == '>') flags |= O_APPEND;

            int fd = open(*(it + 1), flags, 0755);

            IFEXIT(dup2(fd, STDIN_FILENO) == -1, "Failed to redirect\n", EXIT_FAILURE);

            close(fd);

            *it = NULL;
        }

    }

    return 0;
}

int backgroud_processes(int add) {
    static int process_count = 0;

    process_count += add;

    return process_count;
}

pid_t foreground_process(pid_t new_pid) {
    static pid_t pid = -1;

    pid_t old_pid = pid;

    if (new_pid == 0) return old_pid;

    pid = new_pid;

    return old_pid;
}

void sigchld_handler(int sig) {
    pid_t child_pid = waitpid(-1, NULL, WNOHANG);

    if (child_pid <= 0) return;

    backgroud_processes(-1);
}

void sigtstp_handler(int sig) {
    pid_t child_pid = foreground_process(-1);
    if (child_pid == -1) return;
    kill(child_pid, SIGKILL);
}

int main(void) {
    signal(SIGCHLD, sigchld_handler);
    signal(SIGTSTP, sigtstp_handler);

    char inputBuffer[MAX_LINE]; /*buffer to hold command entered */
    int background; /* equals 1 if a command is followed by '&' */
    char *args[MAX_LINE/2 + 1]; /*command line arguments */
    for (;;) {
        background = 0;

        printf(STYLE "myshell$ " RESET);
        fflush(stdout);

        /*setup() calls exit() when Control-D is entered */
        setup(inputBuffer, args, &background);
        
        if (args[0] == NULL) continue;

        if (!strcmp(args[0], "exit")) {
            if (backgroud_processes(0) > 0) {
                printf("There are background processes that are still running!\n");
                fflush(stdout);
                continue;
            }

            printf("Process Terminated!\n");
            exit(0);
        }

        if (!alias_handler(args)) continue;

        pid_t pid;
        IFEXIT((pid = fork()) == -1, "Fork failed!\n", EXIT_FAILURE);

        if (pid) { /* Parent */
            if (!background) {
                foreground_process(pid);
                waitpid(pid, NULL, 0);
            }
            else backgroud_processes(1);
        } else { /* Child */

            char path[MAX_LINE];
            path_find(args[0], path);

            redirect(args);

            execv(path, args);
            
            IFEXIT(1, "Child process encountered an error!", EXIT_FAILURE);
        }
    }
}
