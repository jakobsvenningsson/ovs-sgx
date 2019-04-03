/*
 * list.c
 *
 *  Created on: Apr 23, 2018
 *      Author: jorge
 */

#include "list.h"

/* Initializes 'list' as an empty list. */
void
list_init(struct list *list)
{
    list->next = list->prev = list;
}


/* Removes 'elem' from its list and returns the element that followed it.
   Undefined behavior if 'elem' is not in a list. */
struct list *
list_remove(struct list *elem)
{
    elem->prev->next = elem->next;
    elem->next->prev = elem->prev;
    return elem->next;
}


//lines 34
/* Inserts 'elem' just before 'before'. */
void
list_insert(struct list *before, struct list *elem)
{
    elem->prev = before->prev;
    elem->next = before;
    before->prev->next = elem;
    before->prev = elem;
}

/* Removes elements 'first' though 'last' (exclusive) from their current list,
   then inserts them just before 'before'. */
void
list_splice(struct list *before, struct list *first, struct list *last)
{
    if (first == last) {
        return;
    }
    last = last->prev;

    /* Cleanly remove 'first'...'last' from its current list. */
    first->prev->next = last->next;
    last->next->prev = first->prev;

    /* Splice 'first'...'last' into new list. */
    first->prev = before->prev;
    last->next = before;
    before->prev->next = first;
    before->prev = last;
}
//lines 64





//lines 73
/* Inserts 'elem' at the end of 'list', so that it becomes the back in
 * 'list'. */
void
list_push_back(struct list *list, struct list *elem)
{
    list_insert(list, elem);
}

/* Puts 'elem' in the position currently occupied by 'position'.
 * Afterward, 'position' is not part of a list. */
void
list_replace(struct list *element, const struct list *position)
{
    element->next = position->next;
    element->next->prev = element;
    element->prev = position->prev;
    element->prev->next = element;
}
//lines 90

//lines 170
/* Returns true if 'list' is empty, false otherwise. */
bool
list_is_empty(const struct list *list)
{
    return list->next == list;
}







