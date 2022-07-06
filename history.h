/**
 * @file
 *
 * Contains shell history data structures and retrieval functions.
 */

#ifndef _HISTORY_H_
#define _HISTORY_H_

void hist_init(unsigned int);
void hist_destroy(void);
void hist_add(const char *);
void hist_remove(int command_number);
void hist_print(void);
const char *hist_search_prefix(char *, int);
const char *hist_search_cnum(int);
const char *hist_track_val();
const char *hist_track_prev_val();
const char *hist_track_next_val();
unsigned int hist_oldest_cnum(void);
unsigned int hist_last_cnum(void);
unsigned int hist_track_cnum(void);

#endif
