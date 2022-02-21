#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (!head)
        return NULL;
    head->next = head;
    head->prev = head;

    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (head == NULL)
        return;
    head->prev->next = NULL;
    head->prev = NULL;
    struct list_head *tmp, *l = head->next;
    while (l != NULL) {
        tmp = l->next;
        element_t *e = container_of(l, element_t, list);
        q_release_element(e);
        l = tmp;
    }
    free(head);
}

/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *new = malloc(sizeof(element_t));

    if (!new)
        return false;

    new->value = strdup(s);
    if (new->value == NULL) {
        free(new);
        return false;
    }
    struct list_head *tmp = head->next;

    tmp->prev = &(new->list);
    (new->list).next = tmp;

    head->next = &(new->list);
    (new->list).prev = head;
    // cppcheck-suppress memleak
    return true;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *new = malloc(sizeof(element_t));

    if (!new)
        return false;

    new->value = strdup(s);
    if (new->value == NULL) {
        free(new);
        return false;
    }
    struct list_head *tmp = head->prev;

    tmp->next = &(new->list);
    (new->list).prev = tmp;

    head->prev = &(new->list);
    (new->list).next = head;
    // cppcheck-suppress memleak
    return true;
}

/*
 * Attempt to remove element from head of queue.
 * Return target element.
 * Return NULL if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 *
 * NOTE: "remove" is different from "delete"
 * The space used by the list element and the string should not be freed.
 * The only thing "remove" need to do is unlink it.
 *
 * REF:
 * https://english.stackexchange.com/questions/52508/difference-between-delete-and-remove
 */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (q_size(head) == 0)
        return NULL;

    struct list_head *r = head->next;

    head->next = r->next;
    r->next->prev = head;

    element_t *tmp = container_of(r, element_t, list);

    if (sp) {
        memcpy(sp, tmp->value, bufsize);
        sp[bufsize - 1] = '\0';
    }

    return tmp;
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (q_size(head) == 0)
        return NULL;

    struct list_head *r = head->prev;

    r->prev->next = head;
    head->prev = r->prev;

    element_t *tmp = container_of(r, element_t, list);

    if (sp) {
        memcpy(sp, tmp->value, bufsize);
        sp[bufsize - 1] = '\0';
    }


    return tmp;
}

/*
 * WARN: This is for external usage, don't modify it
 * Attempt to release element.
 */
void q_release_element(element_t *e)
{
    free(e->value);
    free(e);
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(struct list_head *head)
{
    if (head == NULL || list_empty(head))
        return 0;

    int count = 0;
    struct list_head **pptr = &head->next;

    for (; *pptr != head; pptr = &(*pptr)->next, count++)
        ;

    return count;
}

/*
 * Delete the middle node in list.
 * The middle node of a linked list of size n is the
 * ⌊n / 2⌋th node from the start using 0-based indexing.
 * If there're six element, the third member should be return.
 * Return false if list is NULL or empty.
 */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/

    if (q_size(head) == 0)
        return false;

    struct list_head *slow = head->next, *fast = head->next->next;

    for (; fast != head && fast->next != head;
         slow = slow->next, fast = fast->next->next)
        ;

    slow->prev->next = slow->next;
    slow->next->prev = slow->prev;
    element_t *tmp = container_of(slow, element_t, list);

    q_release_element(tmp);

    return true;
}

/*
 * Delete all nodes that have duplicate string,
 * leaving only distinct strings from the original list.
 * Return true if successful.
 * Return false if list is NULL.
 *
 * Note: this function always be called after sorting, in other words,
 * list is guaranteed to be sorted in ascending order.
 */

bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/

    if (q_size(head) < 1)
        return false;

    struct list_head *tmp = head->next;
    while (tmp != head) {
        while (tmp->next != head &&
               strcmp(container_of(tmp, element_t, list)->value,
                      container_of(tmp->next, element_t, list)->value) == 0) {
            struct list_head *tmp2 = tmp->next;
            list_del(tmp->next);
            q_release_element(container_of(tmp2, element_t, list));
        }
        tmp = tmp->next;
    }
    return true;
}

/*
 * Attempt to swap every two adjacent nodes.
 */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/

    if (q_size(head) <= 1)
        return;

    struct list_head *node1 = head->next, *node2 = node1->next;
    while (node1 != head && node2 != head) {
        node1->prev->next = node2;
        node2->prev = node1->prev;

        node2->next->prev = node1;
        node1->next = node2->next;

        node1->prev = node2;
        node2->next = node1;

        node1 = node1->next;
        if (node1 == head)
            break;
        node2 = node1->next;
    }
    return;
}

/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(struct list_head *head)
{
    if (q_size(head) <= 1)
        return;

    struct list_head *ptr = head, *prev_node = ptr->prev,
                     *next_node = ptr->next;

    do {
        ptr->prev = next_node;
        ptr->next = prev_node;

        prev_node = ptr;
        ptr = ptr->prev;
        next_node = ptr->next;

    } while (ptr != head);
}

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */


struct list_head *merge(struct list_head *list1, struct list_head *list2)
{
    char *v1, *v2;
    if (!list1 || !list2)
        return (list2 == NULL) ? list1 : list2;

    struct list_head *newhead = NULL, *tail = NULL, **node = NULL;

    v1 = container_of(list1, element_t, list)->value;
    v2 = container_of(list2, element_t, list)->value;
    node = (strcmp(v1, v2) <= 0) ? &list1 : &list2;
    newhead = tail = *node;
    *node = (*node)->next;

    // cppcheck-suppress knownConditionTrueFalse
    while (list1 && list2) {
        v1 = container_of(list1, element_t, list)->value;
        v2 = container_of(list2, element_t, list)->value;
        node = (strcmp(v1, v2) <= 0) ? &list1 : &list2;

        (*node)->prev = tail;
        tail->next = *node;
        tail = tail->next;

        *node = (*node)->next;
    }

    // cppcheck-suppress knownConditionTrueFalse
    node = (list2 == NULL) ? &list1 : &list2;
    (*node)->prev = tail;
    tail->next = *node;

    return newhead;
}

// ptr2first point to first element of list
struct list_head *merge_sort(struct list_head *pptr2first)
{
    if (pptr2first == NULL || pptr2first->next == NULL)
        return pptr2first;
    struct list_head *slow = pptr2first, *fast = slow->next;

    for (; fast != NULL && fast->next != NULL;
         slow = slow->next, fast = fast->next->next)
        ;

    struct list_head *tmp = slow->next;
    tmp->prev = NULL;

    slow->next = NULL;

    struct list_head *l1 = merge_sort(pptr2first);
    struct list_head *l2 = merge_sort(tmp);

    return merge(l1, l2);
}

void q_sort(struct list_head *head)
{
    if (q_size(head) <= 1)
        return;
    struct list_head *tmp = head->next;

    head->prev->next = NULL;
    head->next->prev = NULL;
    head->prev = NULL;
    head->next = NULL;

    struct list_head *newhead = merge_sort(tmp);

    head->next = newhead;
    newhead->prev = head;

    struct list_head **pptr = &head;
    for (; (*pptr)->next != NULL; pptr = &(*pptr)->next)
        ;

    head->prev = *pptr;
    (*pptr)->next = head;
}
