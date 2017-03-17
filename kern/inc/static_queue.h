/** @file static_queue.h
 *  @brief This file contains the declaration of the static_queue_t structure,
 *  as well as the functions to act on and modify this data structure
 *
 *  @author akanjani, lramire1
 */

#ifndef _STATIC_QUEUE_H_
#define _STATIC_QUEUE_H_

typedef struct static_queue {

  /** @brief Actual queue: contains the elements composing the queue */
  void** queue;

  /** @brief Hold the queue's size (maximum number of elements the queue
   *  may contain) */
  unsigned int size;

  /** @brief The number of elements in the queue at a particular time*/
  unsigned int nb_elem;

  /** @brief Index of the queue's head */
  unsigned int head;

  /** @brief Index of the queue's tail */
  unsigned int tail;

  /** @brief Indicate whether the static_queue is initialized or not.
    * The queue should be initialized by calling static_queue_init().
    * Similarly, the queue is destroyed using static_queue_destroy() */
  char init;

} static_queue_t;

int static_queue_init(static_queue_t* static_queue, unsigned int nb_elem);
int static_queue_destroy(static_queue_t* static_queue);

int static_queue_enqueue(static_queue_t* static_queue, void* elem);
void* static_queue_dequeue(static_queue_t* static_queue);

int static_queue_remove(static_queue_t* static_queue, void* elem);

#endif /* _STATIC_QUEUE_H_ */
