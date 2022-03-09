#include <stddef.h>

#include "history.h"

void hist_init(unsigned int limit)
{
    // TODO: set up history data structures, with 'limit' being the maximum
    // number of entries maintained.
}

void hist_destroy(void)
{
    // TODO: tear down any history-related data structures here
}

void hist_add(const char *cmd)
{
    // TODO: add a history entry
}

void hist_print(void)
{
    // TODO: print the history list in reverse chronological order
}

const char *hist_search_prefix(char *prefix)
{
    // TODO: Retrieves the most recent command starting with 'prefix', or NULL
    // if no match found.
    return NULL;
}

const char *hist_search_cnum(int command_number)
{
    // TODO: Retrieves a particular command number. Return NULL if no match
    // found.
    return NULL;
}

unsigned int hist_last_cnum(void)
{
    // TODO: Retrieve the most recent command number.
    return 0;
}
