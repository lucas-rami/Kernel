/** @file static_queue.c
 *  @brief This file contains the definitions of functions used to act on a
 *  static_queue_t data structure.
 *
 *  @author akanjani, lramire1
 */

#include <static_queue.h>
#include <stdlib.h>
#include <malloc.h>

#include <simics.h>

/* Boolean values for the queue's init field */
#define STATIC_QUEUE_INIT_FALSE 0
#define STATIC_QUEUE_INIT_TRUE 1

/** @brief Initialize the queue
 *
 *  The function sets various field of the static_queue_t structure to their
 *  initial value and allocates memory to store the queue elements.
 *  This function should be called once before using the static queue.
 *
 *  @param static_queue A static queue
 *  @param nb_elem      The maximum capacity of the queue
 *
 *  @return 0 on success, a negative number on error
 */
int static_queue_init(static_queue_t *static_queue, unsigned int nb_elem) {

  // Check validity of arguments
  if (static_queue == NULL || nb_elem <= 0) {
    lprintf("static_queue_init(): Invalid arguments");
    return -1;
  }

  // Check that the static queue is not already initialized
  if (static_queue->init == STATIC_QUEUE_INIT_TRUE) {
    lprintf("static_queue_init(): The queue is already initialized");
    return -1;
  }

  // Allocate the queue
  static_queue->queue = calloc(nb_elem, sizeof(void*));
  if (static_queue->queue == NULL) {
    lprintf("static_queue_init(): Impossible to allocate memory for the queue");
    return -1;
  }

  // Fill in the structure fields
  static_queue->init = STATIC_QUEUE_INIT_TRUE;
  static_queue->size = nb_elem;
  static_queue->nb_elem = 0;
  static_queue->head = 0;
  static_queue->tail = 0;

  return 0;
}

/** @brief Destroy the queue
 *
 *  The function free the memory used to store the queue elements.
 *  This function should be called once when one is done using the queue.
 *
 *  @param static_queue A static queue
 *
 *  @return 0 on success, a negative number on error
 */
int static_queue_destroy(static_queue_t* static_queue) {

  // Check validity of arguments
  if (static_queue == NULL || static_queue->queue == NULL) {
    lprintf("static_queue_destroy(): Invalid arguments");
    return -1;
  }

  // Check that the static queue is initialized
  if (static_queue->init == STATIC_QUEUE_INIT_FALSE) {
    lprintf("static_queue_destroy(): The queue is not initialized");
    return -1;
  }

  // Free the memory held by the queue
  free(static_queue->queue);
  static_queue->queue = NULL;

  // Set the queue to non-initialized
  static_queue->init = STATIC_QUEUE_INIT_FALSE;

  return 0;
}

/** @brief Enqueue a new element in the queue
 *
 *  If this function is called and the queue is full the element is not added
 *  at the queue's tail and a negative number is returned.
 *
 *  @param static_queue A static queue
 *  @param elem         The new element to enqueue
 *
 *  @return 0 on success, a negative number on error
 */
int static_queue_enqueue(static_queue_t* static_queue, void* elem) {

  // Check validity of arguments
  if (static_queue == NULL || static_queue->queue == NULL) {
    lprintf("static_queue_enqueue(): Invalid arguments");
    return -1;
  }

  // Check that the static queue is initialized
  if (static_queue->init == STATIC_QUEUE_INIT_FALSE) {
    lprintf("static_queue_enqueue(): The queue is not initialized");
    return -1;
  }

  // Check if the queue is full
  if (static_queue->nb_elem == static_queue->size) {
    lprintf("static_queue_enqueue(): The queue is already full");
    return -1;
  }

  // Enqueue the element
  static_queue->queue[static_queue->tail] = elem;
  static_queue->tail = (static_queue->tail + 1) % static_queue->size;

  // Update the total number of elements
  static_queue->nb_elem++;

  return 0;
}

/** @brief Dequeue the first element from the queue
 *
 *  NOTE: Due to the fact that the function returns NULL when the queue is empty
 *  or when an error occurs, it is impossible to determine from the perspective
 *  of the calling function whether an element with value NULL was dequeued from
 *  the queue or if one of the condition for the function returning NULL was
 *  met. This constraint is not an issue if the code using this queue makes sure
 *  that an element with value NULL is never enqueued.
 *
 *  @param static_queue A static queue
 *
 *  @return The dequeued element if the queue wasn't empty, NULL if the
 *  queue was empty or if an error occured.
 */
void* static_queue_dequeue(static_queue_t* static_queue) {

  // Check validity of arguments
  if (static_queue == NULL || static_queue->queue == NULL) {
    lprintf("static_queue_dequeue(): Invalid arguments");
    return NULL;
  }

  // Check that the static queue is initialized
  if (static_queue->init == STATIC_QUEUE_INIT_FALSE) {
    lprintf("static_queue_dequeue(): The queue is not initialized");
    return NULL;
  }

  // Check if the queue is empty
  if (static_queue->nb_elem == 0) {
    lprintf("static_queue_dequeue(): The queue is empty");
    return NULL;
  }

  // Dequeue the element
  void* ret = static_queue->queue[static_queue->head];
  static_queue->head = (static_queue->head + 1) % static_queue->size;

  // Update the total number of elements
  static_queue->nb_elem--;

  return ret;
}
