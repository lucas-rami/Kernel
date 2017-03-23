/** @file dynamic_queue.h
 *  @brief This file declares the queue structure as well as functions to use
 * the generic queue.
 *  @author akanjani, lramire1
 */

#ifndef _DYNAMIC_QUEUE_H_
#define _DYNAMIC_QUEUE_H_

#include <generic_node.h>
#include <mutex.h>

/** @brief A structure that represents a generic queue object. It contains
 *   the head and tail pointers for the queue and the mutex used by it to
 *   make all of its updates atomic and mutually exclusive
 */
typedef struct queue {

  /** @brief A pointer to the generic_node_t type serving the head of this
   *   queue
   */
  generic_node_t *head;

  /** @brief A pointer to the generic_node_t type serving the tail of this
   *   queue
   */
  generic_node_t *tail;

  /** @brief A mutex for this queue to perform all operations atomic
   */
  mutex_t mp;

} generic_queue_t;

int queue_init(generic_queue_t *list);
int queue_insert_node(generic_queue_t *list, void *value);
void *queue_delete_node(generic_queue_t *list);
int is_queue_empty(generic_queue_t *list);

#endif /* _DYNAMIC_QUEUE_H_ */
