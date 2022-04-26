#include <stddef.h>
#include <string.h>

#include "history.h"
#include "linkedhistory.c"

static struct LinkedHistory *history = NULL;

void hist_init(unsigned int limit)
{
    history = (struct LinkedHistory *) malloc(sizeof(struct LinkedHistory));
    history->list_max = limit;
    history->list_sz = 0;
    history->total_id_count = 0;
    history->start_invert = false;
    history->head = NULL;
    history->tail = NULL;
    history->track = NULL;
}

void hist_destroy(void)
{
    while(history->head != NULL) {
        del_head(history);
    }
    free(history);
}

void hist_add(const char *cmd)
{
    append_node(cmd, history);
    history->track = NULL;
}

void hist_print(void)
{
    node_ptr temp_node = history->head;
    while(temp_node != NULL){
        printf("[%d] %s\n", temp_node->id, temp_node->val);
        fflush(stdout);
        temp_node = temp_node->next;
    }
}

const char *hist_search_prefix(char *prefix, int newer)
{
    // TODO: Retrieves the most recent command starting with 'prefix', or NULL
    // if no match found.

    bool not_found = true;

    if(history->track == NULL) {
        history->track = history->tail;
    } else if(newer) {
        history->track = history->track->next;
    } else {
        history->track = history->track->prev;
    }

    while(history->track != NULL) {
            if(strncmp(prefix, history->track->val, strlen(prefix)) == 0) {
                return history->track->val;
            }
            if(newer) {
                history->track = history->track->next;
            } else {
                history->track = history->track->prev;
            }
    }
    return NULL;
}

const char *hist_search_cnum(int command_number)
{
    /* If the command_number is between the id of head and tail */
    if(history->head != NULL) {
        if(command_number <= history->tail->id &&
            command_number >= history->head->id) {
            /* Calculate index of node in list with offset */
            int offset = (history->total_id_count) - (history->list_sz);
            /* -1 to turn command_number into index num */
            return get_node(history, command_number - offset - 1)->val;
        }
    }
    return NULL;
}

const char *hist_track_val() {
    return history->track != NULL
        ? history->track->val
        : NULL;
}

const char *hist_track_prev_val() {
    if(history->track != NULL && (history->track = history->track->prev) != NULL) {
       return history->track->val;
    }
    return NULL;
}

const char *hist_track_next_val(){
    if(history->track != NULL && (history->track = history->track->next) != NULL) {
        return history->track->val;
    }
    return NULL;
}

unsigned int hist_oldest_cnum(void) {
    return history->head != NULL
        ? history->head->id
        : -1;
}

unsigned int hist_last_cnum(void)
{
    return history->tail != NULL
        ? history->tail->id
        : -1;
}

unsigned int hist_track_cnum(void) {
    return history->track != NULL
        ? history->track->id
        : -1;
}
