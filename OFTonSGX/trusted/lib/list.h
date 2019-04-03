/*
 * list.h
 *
 *  Created on: Apr 23, 2018
 *      Author: jorge
 */

#ifndef ENCLAVE_MYENCLAVE_TRUSTED_LIB_LIST_H_
#define ENCLAVE_MYENCLAVE_TRUSTED_LIB_LIST_H_

/* Doubly linked list. */

#include <stdbool.h>
#include <stddef.h>
#include "util.h"

/* Doubly linked list head or element. */
struct list {
    struct list *prev;     /* Previous list element. */
    struct list *next;     /* Next list element. */
};

#define LIST_INITIALIZER(LIST) { LIST, LIST }


void list_init(struct list *);

/* List insertion. */
void list_insert(struct list *, struct list *);
void list_splice(struct list *before, struct list *first, struct list *last);
void list_replace(struct list *, const struct list *);
void list_push_back(struct list *, struct list *);

/* List removal. */
struct list *list_remove(struct list *);

/* List properties. */
bool list_is_empty(const struct list *);

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







#endif /* ENCLAVE_MYENCLAVE_TRUSTED_LIB_LIST_H_ */
