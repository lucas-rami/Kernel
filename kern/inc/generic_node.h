/** @file generic_node.h
 *  @brief This file declares the generic_node_t structure.
 *  @author akanjani, lramire1
 */

#ifndef _GENERIC_NODE_H
#define _GENERIC_NODE_H

/** @brief A structure that represents a generic node in a linked list/queue
 */
typedef struct generic_node {

  /** @brief A void pointer type member which stores the data to be stored in
   *   every node
   */
  void *value;

  /** @brief A struct generic_node* type member which stores the address of
   *   the next node in a lnked list
   */
  struct generic_node *next;
} generic_node_t;

#endif /* _GENERIC_NODE_H */
