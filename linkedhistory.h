#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef struct Node * node_ptr;

struct Node
{
    int id;
    char* val;
    node_ptr prev;
    node_ptr next;
};

struct LinkedHistory
{
    node_ptr head;
    node_ptr tail;
    node_ptr track;
    bool start_invert;
    unsigned int list_sz;
    unsigned int list_max;
    unsigned int total_id_count;
};

node_ptr get_node(struct LinkedHistory *list, int position);
void del_head(struct LinkedHistory *list);
void del_tail(struct LinkedHistory *List);
void append_node(const char *str, struct LinkedHistory *list);
