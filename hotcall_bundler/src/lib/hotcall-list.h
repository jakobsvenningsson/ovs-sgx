/*
 * Copyright (c) 2008, 2009, 2010, 2011 Nicira, Inc.
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
#ifndef _LIST_HOTCALL_TRUSTED_H
#define _LIST_HOTCALL_TRUSTED_H 1


#include <stddef.h>
#include <stdbool.h>

#ifdef  __cplusplus
extern "C" {
#endif


/* Doubly linked list. */
#define CONST_CAST(TYPE, POINTER)                               \
    ((void) sizeof ((int) ((POINTER) == (TYPE) (POINTER))),     \
     (TYPE) (POINTER))

/* Doubly linked list head or element. */
struct hcall_list {
    struct hcall_list *prev;     /* Previous list element. */
    struct hcall_list *next;     /* Next list element. */
};

#define LIST_INITIALIZER(LIST) { LIST, LIST }

void hcall_list_init(struct hcall_list *);
void hcall_list_poison(struct hcall_list *);

/* List insertion. */
void hcall_list_insert(struct hcall_list *, struct hcall_list *);
void hcall_list_splice(struct hcall_list *before, struct hcall_list *first, struct hcall_list *last);
void hcall_list_push_front(struct hcall_list *, struct hcall_list *);
void hcall_list_replace(struct hcall_list *, const struct hcall_list *);
void hcall_list_moved(struct hcall_list *);

/* List removal. */
struct hcall_list *hcall_list_pop_front(struct hcall_list *);
struct hcall_list *hcall_list_pop_back(struct hcall_list *);

/* List elements. */
struct hcall_list *hcall_list_front(const struct hcall_list *);
struct hcall_list *hcall_list_back(const struct hcall_list *);

/* List properties. */
size_t hcall_list_size(const struct hcall_list *);
bool hcall_list_is_empty(const struct hcall_list *);
bool hcall_list_is_singleton(const struct hcall_list *);
bool hcall_list_is_short(const struct hcall_list *);

#define LIST_FOR_EACH(ITER, MEMBER, LIST)                               \
    for (ASSIGN_CONTAINER(ITER, (LIST)->next, MEMBER);                  \
         &(ITER)->MEMBER != (LIST);                                     \
         ASSIGN_CONTAINER(ITER, (ITER)->MEMBER.next, MEMBER))
#define LIST_FOR_EACH_CONTINUE(ITER, MEMBER, LIST)                      \
    for (ASSIGN_CONTAINER(ITER, (ITER)->MEMBER.next, MEMBER);           \
         &(ITER)->MEMBER != (LIST);                                     \
         ASSIGN_CONTAINER(ITER, (ITER)->MEMBER.next, MEMBER))
#define LIST_FOR_EACH_REVERSE(ITER, MEMBER, LIST)                       \
    for (ASSIGN_CONTAINER(ITER, (LIST)->prev, MEMBER);                  \
         &(ITER)->MEMBER != (LIST);                                     \
         ASSIGN_CONTAINER(ITER, (ITER)->MEMBER.prev, MEMBER))
#define LIST_FOR_EACH_REVERSE_CONTINUE(ITER, MEMBER, LIST)              \
    for (ASSIGN_CONTAINER(ITER, (ITER)->MEMBER.prev, MEMBER);           \
         &(ITER)->MEMBER != (LIST);                                     \
         ASSIGN_CONTAINER(ITER, (ITER)->MEMBER.prev, MEMBER))
#define LIST_FOR_EACH_SAFE(ITER, NEXT, MEMBER, LIST)            \
    for (ASSIGN_CONTAINER(ITER, (LIST)->next, MEMBER);          \
         (&(ITER)->MEMBER != (LIST)                             \
          ? ASSIGN_CONTAINER(NEXT, (ITER)->MEMBER.next, MEMBER) \
          : 0);                                                 \
         (ITER) = (NEXT))


 /* Removes 'elem' from its list and returns the element that followed it.
    Undefined behavior if 'elem' is not in a list. */
 static inline struct hcall_list *
 hcall_list_remove(struct hcall_list *elem)
 {
     elem->prev->next = elem->next;
     elem->next->prev = elem->prev;
     return elem->next;
 }


 /* Inserts 'elem' at the end of 'list', so that it becomes the back in
  * 'list'. */
static inline void
hcall_list_push_back(struct hcall_list *list, struct hcall_list *elem)
{
    hcall_list_insert(list, elem);
}


 #ifdef  __cplusplus
 }
 #endif

#endif /* list.h */
