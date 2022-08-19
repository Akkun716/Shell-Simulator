#include <stddef.h>
#include <string.h>

#include "history.h"
#include "logger.h"
#include "linkedhistory.c"

static struct LinkedHistory *history = NULL;

void hist_init(unsigned int limit)
{
    LOG("Initializing history%s\n", "");
    history = (struct LinkedHistory *) malloc(sizeof(struct LinkedHistory));
    history->list_max = limit;
    history->list_sz = 0;
    history->total_id_count = 0;
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
    append_node(history, cmd, -1, false);
    history->track = NULL;
}

void hist_remove(int command_number)
{
    LOG("Total num of ids in history is %d, id to remove is %d\n", hist_last_cnum(), command_number);
    LOG("Command %d to be removed is %s\n", command_number, hist_search_cnum(command_number));
    LOG("Value at tail was %s\n", hist_search_cnum(hist_last_cnum()));
    int offset = (history->total_id_count) - (history->list_sz);
    remove_node(history, command_number - offset - 1, false);
    LOG("Value at tail is now %s\n", hist_search_cnum(hist_last_cnum()));
    history->track = NULL;
}

void hist_print(void)
{
    node_ptr temp_node = history->head;
    while(temp_node != NULL){
        printf("%d %s\n", temp_node->id, temp_node->val);
        fflush(stdout);
        temp_node = temp_node->next;
    }
}

const char *hist_search_prefix(char *prefix, int newer)
{
    if(history->track == NULL) {
        if(newer == 0) {
            history->track = history->tail;
        }
    } else if(newer) {
        history->track = history->track->next;
    } else {
        if(history->track->prev != NULL) {
            history->track = history->track->prev;
        }
    }

    while(history->track != NULL) {
            if(strncmp(prefix, history->track->val, strlen(prefix)) == 0) {
                return history->track->val;
            }
            if(newer) {
                history->track = history->track->next;
            } else {
                if(history->track == history->head) {
                    break;
                }
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

void hist_track_clear() {
    history->track = NULL;
}

const char *hist_track_val() {
    return history->track != NULL
        ? history->track->val
        : NULL;
}

const char *hist_track_prev_val() {
    if(history->track != NULL && history->track->prev != NULL) {
        history->track = history->track->prev;
        return history->track->val;
    }
    return NULL;
}

const char *hist_track_next_val(){
    if(history->track != NULL && /*(history->track = */history->track->next/*)*/ != NULL) {
        history->track = history->track->next;
        return history->track->val;
    }
    return NULL;
}

unsigned int hist_oldest_cnum(void) {
    return history != NULL && history->head != NULL
        ? history->head->id
        : -1;
}

unsigned int hist_last_cnum(void)
{
    return history != NULL && history->tail != NULL
        ? history->tail->id
        : -1;
}

unsigned int hist_track_cnum(void) {
    return history != NULL && history->track != NULL
        ? history->track->id
        : -1;
}
