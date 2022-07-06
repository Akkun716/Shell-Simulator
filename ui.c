#include <stdio.h>
#include <signal.h>
#include <readline/readline.h>
#include <locale.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>

#include "history.h"
#include "logger.h"
#include "ui.h"

static const char *good_str = "✅";
static const char *bad_str  = "🔥";
static int status = 0;
static char *prefix = NULL;
char *home = NULL;
int home_size = 0;
int delim_offset = 0;
char **path_complete = NULL;
int path_dir_ind = 0;
int dir_ind = 0;

static int readline_init(void);

void init_ui(void)
{
    LOGP("Initializing UI...\n");

    char *locale = setlocale(LC_ALL, "en_US.UTF-8");
    LOG("Setting locale: %s\n",
            (locale != NULL) ? locale : "could not set locale!");

    rl_startup_hook = readline_init;
}

void destroy_ui(void)
{
    if(path_complete != NULL) {
        free(path_complete);
        path_complete = NULL;
    }
    if(prefix != NULL) {
        ui_clear_prefix();
    }
}

char *prompt_line(void)
{
    const char *status = prompt_status() ? bad_str : good_str;

    char cmd_num[25];
    snprintf(cmd_num, 25, "%d", prompt_cmd_num());

    char *user = prompt_username();
    char *host = prompt_hostname();
    char *cwd = prompt_cwd();

    char *format_str = ">>-[%s]-[%s]-[%s@%s:%s]-> ";

    size_t prompt_sz
        = strlen(format_str)
        + strlen(status)
        + strlen(cmd_num)
        + strlen(user)
        + strlen(host)
        + strlen(cwd)
        + 1;

    char *prompt_str =  malloc(sizeof(char) * prompt_sz);

    snprintf(prompt_str, prompt_sz, format_str,
            status,
            cmd_num,
            user,
            host,
            cwd);

    free(cwd);
    free(host);
    return prompt_str;
}

char *prompt_username(void)
{
    return getenv("USER");
}

char *prompt_hostname(void)
{
    char *name = malloc(HOST_NAME_MAX * sizeof(char));
    gethostname(name, HOST_NAME_MAX);
    return name;
}

char *prompt_cwd(void)
{
    char *cwd = getcwd(NULL, 0);

    if(home_size == 0) {
        home = getenv("HOME");
        home_size = strlen(home);
    }

    if(strncmp(home, cwd, home_size) == 0) {
        int new_size = strlen(cwd) - home_size;
        if(new_size == 0) {
            //char new_cwd[2];
            char *new_cwd = malloc(2 * sizeof(char));
            new_cwd[0] = '~';
            new_cwd[1] = '\0';
            return new_cwd;
        } else {
            //char new_cwd[new_size + 2];
            char *new_cwd = malloc((new_size + 2) * sizeof(char));
            new_cwd[0] = '~';
            int new_ind = 1;
            for(int i = home_size; i < strlen(cwd); i++) {
                new_cwd[new_ind++] = cwd[i];
            }
            new_cwd[new_size + 1] = '\0';
            return new_cwd;
        }
    }
    return cwd;
}

int prompt_status(void)
{
    return status;
}

void good_status(void)
{
    status = 0;
}

void bad_status(void)
{
    status = -1;
}

unsigned int prompt_cmd_num(void)
{
    int cmd_num = hist_last_cnum();
    return cmd_num != -1
        ? cmd_num + 1
        : 1;
    return 1;
}

/*int sig_handler(int signo)
{
    switch(signo) {
        case SIGINT:
            printf("\n");
            fflush(stdout);
    }

    return 0;
}*/

char *read_command(void)
{
    char *prompt = NULL;
    char *command = NULL;
    //int interrupt = signal(SIGINT, sig_handler);

    prompt = prompt_line();
    command = readline(prompt);
    free(prompt);
    return command == NULL
        ? ""
        : command;
}

int readline_init(void)
{
    rl_bind_keyseq("\\e[A", key_up);
    rl_bind_keyseq("\\e[B", key_down);
    rl_variable_bind("show-all-if-ambiguous", "on");
    rl_variable_bind("colored-completion-prefix", "on");
    rl_attempted_completion_function = command_completion;
    rl_getc_function = getc;
    return 0;
}

int key_up(int count, int key)
{
    char *output_str = NULL;

    //LOG("Current value of val is %s\n", hist_track_val());
    char *track_val = hist_track_val();
    if(track_val == NULL) {
        track_val = "";
    }

    /* If text was entered and there was no prefix stored
     * OR
     * If a prefix is stored and its length is different than
     * user entered command
     *
     * This verifies the user changed the entry in history */

    if (strcmp(rl_line_buffer, "") == 0) {
        ui_clear_prefix();
    } else if(strcmp(rl_line_buffer, track_val) != 0) {
        if(prefix == NULL || strncmp(prefix, rl_line_buffer, strlen(prefix)) != 0) {
            prefix = strdup(rl_line_buffer);
            hist_track_clear();
            LOG("prefix updated to: %s\n", prefix);
        }
    } else {
        //LOG("prefix was NOT updated %s\n", "");
    }
    

    if(prefix != NULL) {
        /* Checks if there is a valid prefix */
        //LOG("Prefix search was executed!%s\n", "");
        output_str = hist_search_prefix(prefix, 0);
        
        /*while(output_str != NULL && strcmp(output_str, rl_line_buffer) == 0) {
            output_str = hist_search_prefix(prefix, 0);
        }*/
        
        if(output_str == NULL) {
            output_str = rl_line_buffer;
        }
    } else {
        if(hist_track_cnum() == -1) {
            /* If first upkey press && line is blank*/
            output_str = hist_search_cnum(hist_last_cnum()); 
        } else if(hist_track_cnum() == hist_oldest_cnum()) {
            /* If the last up press returned the oldest hist item */
            output_str = hist_search_cnum(hist_oldest_cnum()); 
        } else {
            output_str = hist_track_prev_val();
        }
    }

    if(output_str == NULL) {
        output_str = "";
    }

    rl_replace_line(output_str, 1);

    /* Move the cursor to the end of the line: */
    rl_point = rl_end;
    //rl_mark = rl_end;

    return 0;
}

int key_down(int count, int key)
{
    char *output_str = NULL;
    /* Modify the command entry text: */

    //LOG("Current value of val is %s\n", hist_track_val());
    char *track_val = hist_track_val();
    if(track_val == NULL) {
        track_val = "";
    }

    if (strcmp(rl_line_buffer, "") == 0) {
        ui_clear_prefix();
    } else if(strcmp(rl_line_buffer, track_val) != 0) {
        if(prefix == NULL || strncmp(prefix, rl_line_buffer, strlen(prefix)) != 0) {
            prefix = strdup(rl_line_buffer);
            LOG("prefix updated to: %s\n", prefix);
        }
    } else {
        //LOG("prefix was NOT updated %s\n", "");
    }
    

    if(prefix != NULL) {
        //LOG("Prefix search was executed!%s\n", "");
        output_str = hist_search_prefix(prefix, 1);
        while(output_str != NULL && strcmp(output_str, rl_line_buffer) == 0) {
            output_str = hist_search_prefix(prefix, 1);
        }
    } else {
        output_str = hist_track_next_val();
    }

    if(output_str == NULL) {
        output_str = "";
    }

    rl_replace_line(output_str, 1);
    /* Move the cursor to the end of the line: */
    rl_point = rl_end;
    //rl_mark = rl_end;
    return 0;
}

void ui_clear_prefix()
{
    free(prefix);
    prefix = NULL;
}

char **command_completion(const char *text, int start, int end)
{
    /* Tell readline that if we don't find a suitable completion, it should fall
     * back on its built-in filename completion. */
    rl_attempted_completion_over = 0;

    return rl_completion_matches(text, command_generator);
}

/**
 * This function is called repeatedly by the readline library to build a list of
 * possible completions. It returns one match per function call. Once there are
 * no more completions available, it returns NULL.
 */
char *command_generator(const char *text, int state)
{
    // TODO: find potential matching completions for 'text.' If you need to
    // initialize any data structures, state will be set to '0' the first time
    // this function is called. You will likely need to maintain static/global
    // variables to track where you are in the search so that you don't start
    // over from the beginning.

    return NULL;
}
