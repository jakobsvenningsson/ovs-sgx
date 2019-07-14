/*
 * Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013 Nicira, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "hotcall-list.h"
#include <string.h>
#include <assert.h>


/* Initializes 'list' as an empty list. */
void
hcall_list_init(struct hcall_list *list)
{
    list->next = list->prev = list;
}

/* Initializes 'list' with pointers that will (probably) cause segfaults if
 * dereferenced and, better yet, show up clearly in a debugger. */
void
hcall_list_poison(struct hcall_list *list)
{
    memset(list, 0xcc, sizeof *list);
}

/* Inserts 'elem' just before 'before'. */
void
hcall_list_insert(struct hcall_list *before, struct hcall_list *elem)
{
    elem->prev = before->prev;
    elem->next = before;
    before->prev->next = elem;
    before->prev = elem;
}

/* Removes elements 'first' though 'last' (exclusive) from their current list,
   then inserts them just before 'before'. */
void
hcall_list_splice(struct hcall_list *before, struct hcall_list *first, struct hcall_list *last)
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

/* Inserts 'elem' at the beginning of 'list', so that it becomes the front in
   'list'. */
void
hcall_list_push_front(struct hcall_list *list, struct hcall_list *elem)
{
    hcall_list_insert(list->next, elem);
}



/* Puts 'elem' in the position currently occupied by 'position'.
 * Afterward, 'position' is not part of a list. */
void
hcall_list_replace(struct hcall_list *element, const struct hcall_list *position)
{
    element->next = position->next;
    element->next->prev = element;
    element->prev = position->prev;
    element->prev->next = element;
}

/* Adjusts pointers around 'list' to compensate for 'list' having been moved
 * around in memory (e.g. as a consequence of realloc()).
 *
 * This always works if 'list' is a member of a list, or if 'list' is the head
 * of a non-empty list.  It fails badly, however, if 'list' is the head of an
 * empty list; just use list_init() in that case. */
void
hcall_list_moved(struct hcall_list *list)
{
    list->prev->next = list->next->prev = list;
}



/* Removes the front element from 'list' and returns it.  Undefined behavior if
   'list' is empty before removal. */
struct hcall_list *
hcall_list_pop_front(struct hcall_list *list)
{
    struct hcall_list *front = list->next;
    hcall_list_remove(front);
    return front;
}

/* Removes the back element from 'list' and returns it.
   Undefined behavior if 'list' is empty before removal. */
struct hcall_list *
hcall_list_pop_back(struct hcall_list *list)
{
    struct hcall_list *back = list->prev;
    hcall_list_remove(back);
    return back;
}

/* Returns the front element in 'list_'.
   Undefined behavior if 'list_' is empty. */
struct hcall_list *
hcall_list_front(const struct hcall_list *list_)
{
    struct hcall_list *list = CONST_CAST(struct hcall_list *, list_);

    return list->next;
}

/* Returns the back element in 'list_'.
   Undefined behavior if 'list_' is empty. */
struct hcall_list *
hcall_list_back(const struct hcall_list *list_)
{
    struct hcall_list *list = CONST_CAST(struct hcall_list *, list_);

    return list->prev;
}

/* Returns the number of elements in 'list'.
   Runs in O(n) in the number of elements. */
size_t
hcall_list_size(const struct hcall_list *list)
{
    const struct hcall_list *e;
    size_t cnt = 0;

    for (e = list->next; e != list; e = e->next) {
        cnt++;
    }
    return cnt;
}

/* Returns true if 'list' is empty, false otherwise. */
bool
hcall_list_is_empty(const struct hcall_list *list)
{
    return list->next == list;
}

/* Returns true if 'list' has exactly 1 element, false otherwise. */
bool
hcall_list_is_singleton(const struct hcall_list *list)
{
    return hcall_list_is_short(list) && !hcall_list_is_empty(list);
}

/* Returns true if 'list' has 0 or 1 elements, false otherwise. */
bool
hcall_list_is_short(const struct hcall_list *list)
{
    return list->next == list->prev;
}
