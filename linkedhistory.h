#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef struct Node * node_ptr;

struct Node
{
    int id;
    int val_count;
    char* val;
    node_ptr prev;
    node_ptr next;
};

struct LinkedHistory
{
    node_ptr head;
    node_ptr tail;
    node_ptr track;
    unsigned int list_sz;
    unsigned int list_max;
    unsigned int total_id_count;
};

node_ptr get_node(struct LinkedHistory *list, int position);
node_ptr find_id(struct LinkedHistory *list, int id);
void del_head(struct LinkedHistory *list);
void del_tail(struct LinkedHistory *List);
void append_node(struct LinkedHistory *list, const char *str, int id, bool reduce_size);
void remove_node(struct LinkedHistory *list, int position, bool id);
