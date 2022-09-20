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
#define BG_LIMIT 10
#define CMD_DELIM " \t\r\n"

/* Holds the previous cd directory */
static char *prev_pwd = NULL;

/* Used for the forking status var */
static int status = 0;
//static int child_queue[BG_LIMIT] = {0};
//static int queue_ind = -1;

/**
 * Initializes the background list with a list max based on the passed limit
 *
 * @param limit the specified limit of max elements contained in list 
 */
void bg_init(unsigned int limit)
{
    LOG("Initializing background jobs list%s\n", "");
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
    int output = 0;
    char *temp = getcwd(NULL, 0);

    if(*buf != NULL) {
        if(*buf[1] == NULL) {
            output = chdir(getenv("HOME"));
        } else if(strcmp(*buf[1], "-") == 0 && prev_pwd != NULL){
	    LOG("Made it into - conditional%s\n", "");
	    output = chdir(prev_pwd);
	} else {
            output = chdir(*buf[1]);
        }
    } else {
        if(args[1] == NULL) {
            output = chdir(getenv("HOME"));
        } else if(strcmp(args[1], "-") == 0 && prev_pwd != NULL) {
	    LOG("Made it into - conditional%s\n", "");
	    output = chdir(prev_pwd);
        } else {
            output = chdir(args[1]);
        }
    }

    if(output == -1) {
        perror("chdir");
    } else {
	LOG("Checking if prev_pwd is empty...%s\n", "");
	if(prev_pwd != NULL) { free(prev_pwd); }
	LOG("Setting prev_pwd%s\n", "");
	prev_pwd = temp;
	LOG("prev_pwd is now %s\n", prev_pwd);
    }

    return output;
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
        *argc = tok_str(*buf_cmd, buf, CMD_DELIM, true);
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
            int id = -1;
            if((id = waitpid(-1, &status, WNOHANG)) > 0) {
                remove_node(bg_jobs, id, true);
            }
            LOG("The value from wait was %d\n", id);
            break;
    }
}

int next_null(char *sel_args[]) {
    int i = 0;
    while(sel_args[i] != NULL) { i += 1; }
    return i;
}

char **io_redir_handle(char **sel_args, int *redir_fd[], char *io_redir[]) {
    int next_ind = next_null(sel_args);
    if((next_ind == redir_fd[0][1] || next_ind == redir_fd[1][1]) &&
        (strcmp(sel_args[next_ind - 1], io_redir[0]) == 0 ||
        strcmp(sel_args[next_ind - 1], io_redir[1]) == 0)) {
        return sel_args + next_ind;
    }
    return sel_args;
}

int exec_pipe(char *sel_args[], int argc, int *redir_fd[], char *io_redir[]) {
    int i = 0;
    int child;
    while(sel_args[i] != NULL) { i += 1; }

    if(i == argc) {
        child = fork();
        if(child == -1) {
            perror("fork");
        } else if (child == 0) {
            if(redir_fd[0][0] || redir_fd[1][0]) {
                file_redir(redir_fd, io_redir);
            }
            execvp(sel_args[0], sel_args);
        } else {
            wait(&status);
            return status;
        }
    } else {
        /* Pipe vars */
        int fds[2];
        int stdin_fd;
        int stdout_fd;
        //if(pipe(fds) == -1) { perror("pipe"); }

        child = fork();
        if(child == -1) {
            perror("fork");
        } else if(child == 0) {
        }
    }
    return 0;
}

int file_redir(int redir_fd[], char *io_redir[]) {
    LOG("Checking redir%s\n", "");
    LOG("Vals at fd-10: %d\n", redir_fd[1]);
    if(redir_fd[0]) {
        LOG("Current input file should be: %s\n", io_redir[0]);
        redir_fd[2] = open(io_redir[0], O_RDONLY, 0666);
	    redir_fd[0] = dup2(redir_fd[2], STDIN_FILENO);
        close(redir_fd[2]);
    }

    if(redir_fd[1]) {
        LOG("Current output file should be: %s\n", io_redir[1]);
        if(redir_fd[1] == 2) {
            redir_fd[2] = open(io_redir[1], O_CREAT | O_WRONLY | O_APPEND, 0666);
        } else {
            redir_fd[2] = open(io_redir[1], O_CREAT | O_WRONLY, 0666);
        }    

        redir_fd[1] = dup2(redir_fd[2], STDOUT_FILENO);
        close(redir_fd[2]);
    }
    return 0;
}

/**
 * Attempts to execute the inputted command. First tokenizes, then 
 */
int execute_cmd(char *command)
{
    ui_clear_prefix();
    status = 0;

    if(strcmp(command, "") == 0) {
        return 0;
    }

    /* Input command vars */
    char **cmd_args = NULL;
    char **buf_args = NULL;
    char **sel_args = NULL;
    char *old_cmd = NULL;
    char *buf_cmd = NULL;
    char *full_cmd = strdup(command);
    int argc = 0;

    if(hist_oldest_cnum() != -1) {
        old_cmd = strdup(hist_search_cnum(hist_oldest_cnum()));
    }
    
    hist_add(full_cmd);
    argc = tok_str(command, &cmd_args, CMD_DELIM, true);

    if(builtin_handler(cmd_args, &argc, &buf_args, &buf_cmd, old_cmd) == 0) {
        LOG("Builtin handled!%s\n", "");
        good_status();
        free(cmd_args);
        free(command);
        free(buf_args);
        free(buf_cmd);
        free(old_cmd);
        free(full_cmd);
        return EXIT_SUCCESS;
    }
    
    free(old_cmd);

    /* Checks for bang handle execution */
    if(buf_args != NULL) {
        sel_args = buf_args;
    } else {
        sel_args = cmd_args;
    }

    /* IO Redirection vars */
    char *io_redir[2] = {0};
    int redir_fd[3] = {0};
    bool pipe_exec = false;
    
    signal(SIGCHLD, sig_handler);
    LOG("Value of argc is %d\n", argc);
    
    for(int ind = 0; ind < argc; ind++) {
        if(argc != 1 && strncmp("|", sel_args[ind], 1) == 0) {
            /* Pipe check */
            LOG("Pipe found at arg %d\n", ind);
            pipe_exec = true;
        } else if(ind != 0 && ind != argc - 1) {
            /* IO Redirection check */
   	        if(strcmp("<", sel_args[ind]) == 0) {
                sel_args[ind] = NULL;
	            io_redir[0] = sel_args[ind - 1];
                redir_fd[0] = 1;
                LOG("Val of io_redir[0] is '%s'\n", io_redir[0]);
            } else if(strcmp(">", sel_args[ind]) == 0) {
                sel_args[ind] = NULL;
                io_redir[1] = sel_args[ind + 1];
                redir_fd[1] = 1;
            } else if(strcmp(">>", sel_args[ind]) == 0) { 
                sel_args[ind] = NULL;
                io_redir[1] = sel_args[ind + 1];
                redir_fd[1] = 2;
            }

            if(redir_fd[1]) {
                LOG("Val of redir_fd[1] is %d\n", redir_fd[1]);
                LOG("Val of io_redir[1] is '%s'\n", io_redir[1]);
            }
        }
    }
 
    LOG("DONE CHECKING ARGS %s\n", "");
    
    if(pipe_exec) {

    } else {
        pid_t child = fork();
        if (child == -1) {
            perror("fork");
        } else if (child == 0) {
            /* I am the child */
            LOG("CHILD PID IS: %d\n", getpid());

            LOG("First arg (file location) is: %s\n", sel_args[0]);
            if(strcmp("&", sel_args[argc - 1]) == 0) {
                sel_args[argc - 1] = NULL;
            }
            
            
            file_redir(redir_fd, io_redir);

            LOG("Current val at sel_args[0]: %s\n", sel_args[0]);
            //sel_args = io_redir_handle(sel_args, redir_fd, io_redir);
            LOG("Current val at new sel_args[0]: %s\n", sel_args[0]); 

            if(execvp(sel_args[0], sel_args) == -1) {
                perror("exec");
                free(cmd_args);
                free(command);
                free(buf_args);
                free(buf_cmd);
                free(full_cmd);
                exit(EXIT_FAILURE);
            }
        } else {
            /* I am the parent */
            if(argc != 0 && strcmp("&", sel_args[argc - 1]) == 0) {
                append_node(bg_jobs, full_cmd, child, true);
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
    }
   
    free(cmd_args);
    free(command);
    free(buf_args);
    free(buf_cmd);
    free(full_cmd);
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
    if(prev_pwd != NULL) { free(prev_pwd); }
    LOG("Thank you for using the %s!\nExiting shell...\n", "Frequently Inconsistant Shell");
    return 0;
}
