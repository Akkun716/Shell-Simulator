#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "util.h"

ssize_t lineread(int fd, char *buf, size_t sz) {
    size_t count_read = 0;
    while(count_read < sz) {
        char c;
        ssize_t read_sz = read(fd, &c, 1);
        if(read_sz == 0) {
    	    return count_read;
    	}
    	else if(read_sz == -1) {
	        return -1;
	    }

    	buf[count_read] = c;
    	count_read += read_sz;

    	if(c == '\n') {
	    	return count_read;
	    }
    }

    return count_read;
}

char *dynamic_lineread(int fd) {
	size_t buf_sz = 128;
	char* buf = NULL;
	size_t count_read = 0;

	while(true) {
		char *tmp_buf = realloc(buf, buf_sz);
		if(tmp_buf == NULL) {
			free(buf);
			return NULL;
		}
		buf = tmp_buf;

		ssize_t read_sz = lineread(fd, buf + count_read, buf_sz - count_read);
		if(read_sz == 0 || read_sz == -1) {
			free(buf);
			return NULL;
		}

		count_read += read_sz;
		size_t last_char = count_read - 1;
		if(count_read >= buf_sz || buf[last_char] == '\n') {
			buf[last_char] = '\0';
			return buf;
		}

		buf_sz *= 2;
	}

	return NULL;
}

char *next_token(char **str_ptr, const char *delim) {
    if (*str_ptr == NULL) {
        return NULL;
    }

    size_t tok_start = strspn(*str_ptr, delim);
    size_t tok_end = strcspn(*str_ptr + tok_start, delim);

    /* Zero length token. We must be finished. */
    if (tok_end  == 0) {
        *str_ptr = NULL;
        return NULL;
    }

    /* Take note of the start of the current token. We'll return it later. */
    char *current_ptr = *str_ptr + tok_start;

    /* Shift pointer forward (to the end of the current token) */
    *str_ptr += tok_start + tok_end;

    if (**str_ptr == '\0') {
        /* If the end of the current token is also the end of the string, we
         * must be at the last token. */
        *str_ptr = NULL;
    } else {
        /* Replace the matching delimiter with a NUL character to terminate the
         * token string. */
        **str_ptr = '\0';

        /* Shift forward one character over the newly-placed NUL so that
         * next_pointer now points at the first character of the next token. */
        (*str_ptr)++;
    }

    return current_ptr;
}

int tok_str(char *str, char **buf[], char *delim, bool comments) {
    char *str_iter = str;
    char *curr_tok = NULL;
    int arr_sz = 1;
    int i = 0;

    *buf = (char **) malloc(sizeof(char *));
    while((curr_tok = next_token(&str_iter, delim)) != NULL) {
        if(comments && strncmp("#", curr_tok, 1) == 0) {
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

char *str_to_lower(char *str) {
	size_t stringlen = strlen(str);
	char *copy = malloc(stringlen + 1);
	for(int i = 0; i < stringlen; i++) {
		*(copy + i) = tolower(*(str + i));
	}
	*(copy + stringlen) = '\0';
	return copy;
}

char *str_to_upper(char *str) {
	size_t stringlen = strlen(str);
	char *copy = malloc(stringlen + 1);
	for(int i = 0; i < stringlen; i++) {
		*(copy + i) = toupper(*(str + i));
	}
	*(copy + stringlen) = '\0';
	return copy;
}
