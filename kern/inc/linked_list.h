/** @file linked_list.h
 *  @brief This file declares the linked list structure as well as functions to
 *   use the generic linked list.
 *  @author akanjani, lramire1
 */

#ifndef _LINKED_LIST_H
#define _LINKED_LIST_H

#include <generic_node.h>
#include <eff_mutex.h>

/** @brief A structure of a linked list
 */
typedef struct linked_list {

  /** @brief A pointer to the generic_node_t type serving the head of this
   *   linked list
   */
  generic_node_t *head;

  /** @brief A pointer to the generic_node_t type serving the tail of this
   *   linked list
   */
  generic_node_t *tail;

  /** @brief A function pointer to the function which can be used to find an
   *   element in this linked list
   */
  int (*find)(void *elem, void *value);

  /** @brief A mutex to guarantee atomic access to the linked list
   */
  eff_mutex_t mp;

} generic_linked_list_t;

int linked_list_init(generic_linked_list_t *list, int (*find)(void *, void *));
int linked_list_insert_node(generic_linked_list_t *list, void *value);
void *linked_list_delete_node(generic_linked_list_t *list, void *value);
void *linked_list_get_node(generic_linked_list_t *list, void *value);
void linked_list_delete_list(generic_linked_list_t *list);

#endif /* _LINKED_LIST_H */
