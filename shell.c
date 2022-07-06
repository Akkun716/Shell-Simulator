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
#include "linkedhistory.h"
#include "logger.h"
#include "util.h"
#include "ui.h"

/* Serves as the background jobs list */
static struct LinkedHistory *bg_jobs = NULL;
/* Used for the forking status var */
static int status = 0;

/**
 * Initializes the background list with a list max based on the passed limit
 *
 * @param limit the specified limit of max elements contained in list 
 */
void bg_init(unsigned int limit)
{
    bg_jobs = (struct LinkedHistory *) malloc(sizeof(struct LinkedHistory));
    bg_jobs->list_max = limit;
    bg_jobs->list_sz = 0;
    bg_jobs->total_id_count = 0;
    bg_jobs->head = NULL;
    bg_jobs->tail = NULL;
    bg_jobs->track = NULL;
}

/**
 * Frees each element of the background list before freeing the pointer itself
 */
void bg_destroy(void)
{
    while(bg_jobs->head != NULL) {
        del_head(bg_jobs);
    }
    free(bg_jobs);
}

/**
 * Tokenizes the passed string
 *
 * @param command the command string to be tokenized
 * @param buf the string array to hold string tokens
 * @return the num of token elements in the array
 */
int tok_cmd(char *command, char **buf[])
{
    char *cmd_iter = command;
    char *curr_tok = NULL;
    int arr_sz = 1;
    int i = 0;

    *buf = (char **) malloc(sizeof(char *));
    while((curr_tok = next_token(&cmd_iter, " \t\r\n")) != NULL) {
        if(strncmp("#", curr_tok, 1) == 0) {
            break;
        } else {
            if(i == arr_sz) {
                arr_sz *= 2;
                *buf = (char **)realloc(*buf, arr_sz * sizeof(char *));
            }
            (*buf)[i] = curr_tok;
            i++;
        }
    }
    
    /* If the array is completely filled, add space for NULL terminator */
    if(i == arr_sz) {
        arr_sz += 1;
        *buf = (char **)realloc(*buf, arr_sz * sizeof(char *));
    }
    (*buf)[i] = NULL;
    return i;
}

/* All builtin functions use the same arguments. Refer to builtin_handler() for arg explanations */

/**
 * Exits the program
 */
int exit_handler(char *args[], int *argc, char **buf[], char **buf_cmd, char *old_cmd) {
    exit(EXIT_SUCCESS);
}

/**
 * Initializes the background list with a list max based on the passed limit
 */
int cd_handler(char *args[], int *argc, char **buf[], char **buf_cmd, char *old_cmd) {
    if(*buf != NULL) {
        chdir(*buf[1]);
    } else {
        chdir(args[1]);
    }
    return 0;
} 

/**
 * Prints the history list
 */
int hist_handler(char *args[], int *argc, char **buf[], char **buf_cmd, char *old_cmd) {
    hist_print();
    return 0;
}

/**
 * Attempts to retrieve the specified command from the bang command
 */
int bang_handler(char *args[], int *argc, char **buf[], char **buf_cmd, char *old_cmd) {
    /* If the first arg contains a bang, if 1) that is the only
     * arg, exit with 0, executing nothing. Else 2) if there are
     * other args, exit with -1 for command to be executed normally
     */
    int bang_num = 0;
    char *bang_cmd = NULL;
    LOG("Bang handler executed!%s\n", "");

    if(*argc == 1 && strcmp(args[0], "!") == 0) {
        LOG("Bang handler special finish!%s\n", "");
        return 0;
    }

    if(strcmp(args[0], "!!") == 0) {
        bang_cmd = hist_search_cnum(hist_last_cnum() - 1);
    } else if((bang_num = atoi(args[0] + 1)) != NULL) {
        /* Checks if the oldest command num is one prior the current oldest */
        if(bang_num == hist_oldest_cnum() - 1 && old_cmd != NULL) {
            bang_cmd = old_cmd;
        } else {
            bang_cmd = hist_search_cnum(bang_num);
        }
    } else {
        bang_cmd = hist_search_prefix(args[0] + 1, 0);
        
        /* If no prefix found, check if the oldest command satisfies the requirement */
        if(bang_cmd == NULL && strncmp(args[0] + 1, old_cmd, strlen(args[0] + 1)) == 0) {
            bang_cmd = old_cmd;
        }
    } 
   
   
    LOG("Bang_cmd currently %s\n", bang_cmd);
    hist_remove(hist_last_cnum());
    if(bang_cmd != NULL) {
        /*if(*buf != NULL || *buf_cmd != NULL) {
            free(*buf);
            free(*buf_cmd);
        }*/
    
        LOG("Bang cmd added to history! Bang command receive: %s\n", bang_cmd);
        *buf_cmd = strdup(bang_cmd);
        hist_add(*buf_cmd);
        *argc = tok_cmd(*buf_cmd, buf);
    }
    LOG("Bang handler default finish!%s\n", "");
    return -1;
}

/**
 * Prints the list of currently active background jobs
 */
int jobs_handler(char *args[], int *argc, char **buf[], char **buf_cmd, char *old_cmd)
{ 
    node_ptr temp_node = bg_jobs->head;
    while(temp_node != NULL){
        printf("%d %s\n", temp_node->id, temp_node->val);
        fflush(stdout);
        temp_node = temp_node->next;
    }
    return 0; 
}

/* Struct for builtin functions, including name and to be specified function */
struct builtin {
    char name[25];
    int (*function)(char *args[], int *argc, char **buf[], char **buf_cmd, char *old_cmd);
};

/* List for all supported builtin functions */
struct builtin builtin_list[] = {
    {"!", bang_handler},
    {"cd", cd_handler},
    {"exit", exit_handler},
    {"history", hist_handler},
    {"jobs", jobs_handler},
};

/**
 * Handler function to check for builtin functions. 
 * 
 * @param args array of tokens from originally entered command
 * @param argc total num of argument tokens
 * @param buf pointer to array for arg tokens of a new command
 * @param buf_cmd pointer to new command string
 * @param old_cmd string of the "oldest" command (to handle bang of oldest command num);
 * @return 0 when builtin function finishes or -1 if specified otherwise
 */
int builtin_handler(char *args[], int *argc, char **buf[], char **buf_cmd, char *old_cmd)
{
    if(args[0] == NULL) {
        return -1;
    }

    int return_stat = -1;

    for(int i = 0; i < (sizeof(builtin_list)/sizeof(struct builtin)); i++) {
        if(*buf != NULL) {
            if(strncmp(builtin_list[i].name, *buf[0], strlen(builtin_list[i].name)) == 0) {
                return_stat = builtin_list[i].function(args, argc, buf, buf_cmd, old_cmd);
            }
        } else {
            if(strncmp(builtin_list[i].name, args[0], strlen(builtin_list[i].name)) == 0) {
                return_stat = builtin_list[i].function(args, argc, buf, buf_cmd, old_cmd);
            }
        }
    }

    return return_stat;
}

/**
 * Signal handler for interrupt and child process finish.
 */
void sig_handler(int signo) {
    switch(signo) {
        case SIGINT:
            fflush(stdout);
            break;
        case SIGCHLD:
            waitpid(-1, &status, WNOHANG);
            break;
    }
}

/**
 * Attempts to execute the inputted command. First tokenizes, then 
 */
int execute_cmd(char *command)
{ 
    ui_clear_prefix();

    if(strcmp(command, "") == 0) {
        return 0;
    }

    char **cmd_args = NULL;
    char **buf_args = NULL;
    char **sel_args = NULL;
    char *old_cmd = NULL;
    char *buf_cmd = NULL;
    int argc = 0;

    if(hist_oldest_cnum() != -1) {
        old_cmd = strdup(hist_search_cnum(hist_oldest_cnum()));
    }
    
    hist_add(command);
    argc = tok_cmd(command, &cmd_args);

    if(builtin_handler(cmd_args, &argc, &buf_args, &buf_cmd, old_cmd) == 0) {
        LOG("Builtin handled!%s\n", "");
        good_status();
        free(cmd_args);
        free(command);
        free(buf_args);
        free(buf_cmd);
        free(old_cmd);
        return EXIT_SUCCESS;
    }
    
    free(old_cmd);

    if(buf_args != NULL) {
        sel_args = buf_args;
    } else {
        sel_args = cmd_args;
    }

    //char **pipe_args = NULL;
    //char **temp_args = sel_args;
    bool pipe_exec = false;
    int start = 0;
    int end = argc - 1;
    
    int fd[4];
    if(pipe(fd) == -1) {
        perror("pipe");
    }

    for(int ind = 0; ind < argc; ind++) {
        if(argc != 1 && strncmp("|", sel_args[ind], 1) == 0) {
            sel_args[ind] = NULL;
            end = ind;
            pipe_exec = true;

            LOG("Pipe found as arg %d\n", ind);
        } else if(ind == argc - 1 && start != 0) {
            LOG("Last ind reached!%s\n", "");
            end = ind;
            pipe_exec = true;
        }

        if(ind == argc - 1 || pipe_exec) {
            pid_t child = fork();
            if (child == -1) {
                perror("fork");
                close(fd[0]);
                close(fd[1]);
                close(fd[2]);
                close(fd[3]);
            } else if (child == 0) {
                /* I am the child */
                if(pipe_exec) {
                    if(start == 0) {
                        close(fd[0]);
                        dup2(fd[1], STDOUT_FILENO);
                        close(fd[1]);
                        close(fd[2]);
                    } else if(end == argc - 1) {
                        close(fd[1]);
                        dup2(fd[0], STDIN_FILENO);
                        close(fd[0]);
                        close(fd[2]);
                        LOG("End of parse reached at %d\n", ind);
                    } else {
                        dup2(fd[1], STDOUT_FILENO);
                        dup2(fd[0], STDIN_FILENO);
                        close(fd[0]);
                        close(fd[1]);
                        close(fd[2]);
                    }
                } else {
                    close(fd[0]);
                    close(fd[1]);
                    close(fd[2]);
                }
                         
                LOG("First arg (file location) is: %s\n", sel_args[start]);
                if(strcmp("&", sel_args[argc - 1]) == 0) {
                    sel_args[argc - 1] = NULL;
                    write(fd[3], getpid(), sizeof(getpid()));
                }
                close(fd[3]);

                if(execvp(sel_args[start], sel_args + start) == -1) {
                    perror("exec");
                    free(cmd_args);
                    free(command);
                    free(buf_args);
                    free(buf_cmd);
                    exit(EXIT_FAILURE);
                }
            } else {
                /* I am the parent */
                LOG("Value of argc is %d\n", argc);
                //int status;
                close(fd[1]);

                signal(SIGCHLD, sig_handler);
                if(argc != 0 && strcmp("&", sel_args[argc - 1]) == 0) {
                    
                    if(buf_args != NULL) {
                        append_node(bg_jobs, buf_cmd, -1, true);
                    } else {
                        append_node(bg_jobs, command, -1, true);
                    }
                } else {
                    wait(&status);
                }

                if(status != 0) {
                    bad_status();
                } else {
                    good_status();
                }


                LOG("Child exited with status code: %d\n", status);
            }
            
            if(pipe_exec) {
                start = end + 1;
                pipe_exec = false;
            }
        }
    }

    close(fd[0]);
    
    free(cmd_args);
    free(command);
    free(buf_args);
    free(buf_cmd);
    LOG("Final frees executed%s\n", "");
    return EXIT_SUCCESS;
}

void terminal_input() {
    /* This is the dynamic user entry version of the project */
    char *command;

    while(true) {
        LOG("New loop executed!%s\n", "");
        command = read_command();

        if(!strcasecmp(command, "exit")) {
            break;
        }

        execute_cmd(command);
        LOG("Command execution complete! Checking for next loop...%s\n", "");
    }
    LOG("Program run complete! Proceeding to exit terminal read...%s\n", "");
}

void script_input() {
    /* This is the script version of the project */
    char *command;
    
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
    bg_init(10);
    hist_init(100);

    signal(SIGINT, sig_handler);

    if(isatty(STDIN_FILENO)) {
        terminal_input();
    }
    else {
        script_input();
    }

    bg_destroy();
    hist_destroy();
    destroy_ui();
    LOG("Thank you for using the %s!\nExiting shell...\n", "Frequently Inconsistant Shell");
    return 0;
}
