#include <stdio.h>
#include <readline/readline.h>
#include <locale.h>
#include <stdlib.h>
#include <unistd.h>

#include "history.h"
#include "logger.h"
#include "ui.h"

static const char *good_str = "✅";
static const char *bad_str  = "❗";
static int status = 0;
static char *prefix = NULL;

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
    // TODO cleanup code, if necessary
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

    return prompt_str;
}

char *prompt_username(void)
{
    static char user[_SC_LOGIN_NAME_MAX] = {0};
    getlogin_r(user, _SC_LOGIN_NAME_MAX);
    return user;
}

char *prompt_hostname(void)
{
   static char name[_SC_HOST_NAME_MAX] = {0};
   gethostname(name, _SC_HOST_NAME_MAX);
   return name;
}

char *prompt_cwd(void)
{
   char *cwd = getcwd(NULL, 0);
   return cwd;
}

int prompt_status(void)
{
    return status;
}

unsigned int prompt_cmd_num(void)
{
    int cmd_num = hist_last_cnum();
    return cmd_num != -1
        ? cmd_num + 1
        : 1;
}

char *read_command(void)
{
    char *prompt = prompt_line();
    char *command = readline(prompt);
    free(prompt);
    return command;
}

int readline_init(void)
{
    rl_bind_keyseq("\\e[A", key_up);
    rl_bind_keyseq("\\e[B", key_down);
    rl_variable_bind("show-all-if-ambiguous", "on");
    rl_variable_bind("colored-completion-prefix", "on");
    rl_attempted_completion_function = command_completion;
    return 0;
}

int key_up(int count, int key)
{
    char *output_str = NULL;
    
    /* If text was entered and there was no prefix stored
     * OR
     * If a prefix is stored and its length is different than
     * user entered command */
    
    char *track_val = hist_track_val();
    if(track_val == NULL) {
        track_val = "";
    }

    if(rl_mark != rl_point && strcmp(rl_line_buffer, track_val) != 0) {
        prefix = strdup(rl_line_buffer);
        LOG("prefix updated to: %s\n", prefix);
    } else {
        //LOG("prefix was NOT updated %s\n", "");
    }
    

    if(prefix != NULL) {
        //LOG("Prefix search was executed!%s\n", "");
        output_str = hist_search_prefix(prefix, 0);
        if(output_str == NULL) {
            output_str = prefix;
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

    char *track_val = hist_track_val();
    if(track_val == NULL) {
        track_val = "";
    }

    if(rl_mark != rl_point && strcmp(rl_line_buffer, track_val) != 0) {
        prefix = strdup(rl_line_buffer);
        //LOG("prefix updated to: %s\n", prefix);
    } else {
        //LOG("prefix was NOT updated %s\n", "");
    }
    

    if(prefix != NULL) {
        //LOG("Prefix search was executed!%s\n", "");
        output_str = hist_search_prefix(prefix, 1);
        if(output_str == NULL) {
            output_str = prefix;
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

    // TODO: step forward through the history (assuming we have stepped back
    // previously). Going past the most recent history command blanks out the
    // command line to allow the user to type a new command.

    return 0;
}

void ui_clear_prefix() {
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
