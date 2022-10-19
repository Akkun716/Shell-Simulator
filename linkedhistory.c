#include "linkedhistory.h"
#include "logger.h"

void set_node(node_ptr target, node_ptr placed, bool set_before)
{
    /* Sets node before the already placed node  */
    if(set_before) {
        target->prev = placed->prev;
        target->next = placed;
        if(placed->prev != NULL) {
            placed->prev->next = target;
        }
        placed->prev = target;
    } else {
        /* Sets node after the already placed node */
        target->prev = placed;
        target->next = placed->next;
        if(placed->next != NULL) {
            placed->next->prev = target;
        }
        placed->next = target;
    }
}

node_ptr get_node(struct LinkedHistory *list, int position)
{
    if(list->list_sz < 1) {
        /* The list is empty */
        perror("[The list was empty]\n");
        return NULL;
    } else if(position < 0 || position >= list->list_sz) {
        /* The position entered is invalid */
        perror("Position input out of bounds");
        return NULL;
    }
    
    
    /* Position is between head and tail, search linearly */
    int mid = (list->list_sz - 1) / 2;

    if(position > mid){
        /* If position is closer to tail than head, start from tail */
        list->track = list->tail;
        for(int i = list->list_sz - 1; i > mid; i--) {
            if(i == position) {
                break;
            } else {
                list->track = list->track->prev;
            }
        }

    } else {
        /* Else, start from head */
        list->track = list->head;
        for(int i = 0; i <= mid; i++) {
            if(i == position) {
                break; 
            } else {
                list->track = list->track->next;
            }
        }    
    }

    return list->track;
}

node_ptr find_id(struct LinkedHistory *list, int id)
{
    if(list->list_sz < 1) {
        /* The list is empty */
        perror("[The list was empty]\n");
        return NULL;
    }

    list->track = list->head;
    for(int i = 0; i < list->list_sz; i++) {
        if(list->track->id == id) {
            break; 
        } else {
            list->track = list->track->next;
        }
    }    
    return list->track;
}

void del_head(struct LinkedHistory *list)
{
    node_ptr del_node = list->head;
    /* Set list head to the next node */
    list->head = del_node->next;
    if(list->head != NULL) {
        /* Set new head's prev to NULL */
        list->head->prev = NULL;
    } else {
        /* History is empty, so also set tail to NULL */
        list->tail = NULL;
    }

    free(del_node->val);
    free(del_node);

    return;
}

void del_tail(struct LinkedHistory *list)
{
    node_ptr del_node = list->tail;
    /* Set list tail to the prev node */
    list->tail = del_node->prev;
    if(list->tail != NULL) {
        /* Set new tail's next to NULL */
        list->tail->next = NULL;
    } else {
        /* History is empty, so also set head to NULL */
        list->head = NULL;
    }
   
    free(del_node->val);
    free(del_node);

    return;
}


void append_node(struct LinkedHistory *list, const char *str, int id, bool reduce_size)
{
    node_ptr new_node = (node_ptr) malloc(sizeof(struct Node));

   if(new_node == NULL) {
        /* No memory could be alocated */
        perror("No available memory");
        exit(EXIT_FAILURE);
    } else {
        new_node->val = strdup(str);
        list->total_id_count += 1;
        if(id != -1) {
            new_node->id = id;
        } else {
            new_node->id = list->total_id_count;
        }

        if(list->list_sz == 0) {
            /* Position is at head */
            new_node->prev = NULL;
            new_node->next = NULL;
            /* Label new node as tail and head */
            list->tail = new_node;
            list->head = new_node;
        } else {
            /* Add node to list tail */
            set_node(new_node, list->tail, false);
            list->tail = new_node;
        }
        
        if(list->list_sz < list->list_max) {
            LOG("List size increased!%s\n", "");
            list->list_sz += 1;
        } else {
            LOG("List max reached! Deleting oldest history entry %d...\n", list->head->id);
            if(reduce_size) {
                remove_node(list, 0, false);
                list->list_sz += 1;
            } else {
                del_head(list);
            }
        }
    }

    return;
}

void remove_node(struct LinkedHistory *list, int position, bool id) {
    node_ptr del_node;
    if(id) {
        del_node = find_id(list, position);
    } else {
        del_node = get_node(list, position);
    }

    if(del_node != NULL) {
        if(del_node->prev != NULL) {
            del_node->prev->next = del_node->next;
        }
        if(del_node->next != NULL) {
            del_node->next->prev = del_node->prev;
        }

        if(list->tail == del_node) {
            list->tail = del_node->prev;
        }
        if(list->head == del_node) {
            list->head = del_node->next; 
        }

        list->total_id_count -= 1;
        list->list_sz -= 1;

        if(!id) {
            node_ptr temp_node = del_node->next;

            while(temp_node != NULL) {
                temp_node->id -= 1;
                temp_node = temp_node->next;
            }
        }

        free(del_node->val);
        free(del_node);
        LOG("Node found and removed! Current id total and list size is %u and %u\n", list->total_id_count, list->list_sz);
    }

    return;
}
