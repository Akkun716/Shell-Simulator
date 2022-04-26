#include <fcntl.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "history.h"
#include "logger.h"
#include "util.h"
#include "ui.h"

int exit_handler(char *args[]) {
    exit(EXIT_SUCCESS);
}

int cd_handler(char *args[]) {
    chdir(args[1]);
    return 0;
} 

int hist_handler(char *args[]) {
    hist_print();
    return 0;
}

struct builtin {
    char name[25];
    int (*function)(char *args[]);
};

struct builtin builtin_list[] = {
    {"cd", cd_handler},
    {"exit", exit_handler},
    {"history", hist_handler},
    //{"ls", ls_handler},
    //{"!", bang_handler},
};

int builtin_handler(char *args[]) {
    if(args[0] == NULL) {
        return -1;
    }

    for(int i = 0; i < (sizeof(builtin_list)/sizeof(struct builtin)); i++) {
        if(strncmp(builtin_list[i].name, args[0], strlen(builtin_list[i].name)) == 0) {
            return builtin_list[i].function(args);
        }
    }

    return -1;
}

int execute_cmd(char *command) { 

    ui_clear_prefix();

    if(strcmp(command, "") == 0) {
        return 0;
    }

    char *cmd_iter = command;
    char *curr_tok;
    char **cmd_args = (char **)malloc(sizeof(char *));
    int arr_sz = 1;
    int i = 0;

    hist_add(command);

    while((curr_tok = next_token(&cmd_iter, " \t\r\n")) != NULL) {
        if(i == arr_sz) {
            arr_sz *= 2;
            cmd_args = (char **)realloc(cmd_args, arr_sz * sizeof(char *));
        }
        cmd_args[i] = curr_tok;
        i++;
    }

    if(i == arr_sz) {
        arr_sz += 1;
        cmd_args = (char **)realloc(cmd_args, arr_sz * sizeof(char *));
    }
    cmd_args[i] = NULL;

    if(builtin_handler(cmd_args) == 0) {
        free(cmd_args);
        free(command);
        return 0;
    }

    pid_t child = fork();
    if (child == -1) {
        perror("fork");
    } else if (child == 0) {
        /* I am the child */
        if(execvp(cmd_args[0], command) == -1) {
            perror("exec");
            free(cmd_args);
            free(command);
            exit(EXIT_FAILURE);
        }
    } else {
        /* I am the parent */
        int status;
        wait(&status);
        LOG("Child exited with status code: %d\n", status);
    }

    free(cmd_args);
    free(command);
    return EXIT_SUCCESS;
}

void sig_handler(int signo) {
    switch(signo) {
        case SIGINT:
            break;
    }
}

void terminal_input() {
    /* This is the dynamic user entry version of the project */
    char *command;

    signal(SIGINT, sig_handler);

    while(true) {
        command = read_command();
        if (!strcasecmp(command, "exit")) {
            break;
        }

        LOG("Input command: %s\n", command);
        execute_cmd(command);
    }
}

void script_input() {
    /* This is the script version of the project */
    char *command;

    signal(SIGINT, sig_handler);

    while(true) {
        command = dynamic_lineread(fileno(stdin));
        if(command == NULL) {
            break;
        }
        
        LOG("Input command: %s\n", command);

        if (!strcasecmp(command, "exit")) {
            break;
        }

        if(execute_cmd(command) == -1) {
            exit(EXIT_FAILURE);
        }
    }
}

int main(void)
{
    init_ui();
    hist_init(10);

    if(isatty(STDIN_FILENO)) {
        terminal_input();
    }
    else {
        script_input();
    }

    hist_destroy();
    LOG("Thank you for using the %s!\nExiting shell...\n", "Frequently Inconsistant Shell");
    return 0;
}
