/** @file stack_queue.c
 *
 *  @brief This file contains the implementation of funnctions used to modify a 
 *    stack_queue_t structure.
 *
 *  @author akanjani, lramire1
 */

#include <stack_queue.h>
#include <stdlib.h>
#include <assert.h>

/** @brief Initializes the queue
 *
 *  @param queue  The stack queue  
 *
 *  @return void 
 */
void stack_queue_init(stack_queue_t* queue) {

  assert(queue != NULL);

  queue->head = NULL;
  queue->tail = NULL;
  queue->init = QUEUE_INITIALIZED_TRUE;  

}

/** @brief Destroys the queue
 *
 *  @param queue  The stack queue  
 *
 *  @return void 
 */
void stack_queue_destroy(stack_queue_t* queue) {

  assert(queue != NULL && queue->init == QUEUE_INITIALIZED_TRUE);
  assert(queue->head == NULL && queue->tail == NULL);

  queue->init = QUEUE_INITIALIZED_FALSE;      

}

/** @brief Inserts a new element at the queue's tail 
 *
 *  @param queue      The stack queue  
 *  @param new_elem   A pointer to the new element to insert
 *
 *  @return void 
 */
void stack_queue_enqueue(stack_queue_t* queue, generic_node_t* new_elem) {

  assert(queue != NULL && queue->init == QUEUE_INITIALIZED_TRUE);
  assert(new_elem != NULL);

  if (queue->head == NULL) {
    // The queue is empty
    queue->head = queue->tail = new_elem; 
  } else {
    // The queue is non-empty
    queue->tail->next = new_elem;
    queue->tail = new_elem;
    queue->tail->next = NULL;
  }

}

/** @brief Dequeues an element at the queue's head 
 *
 *  @param queue      The stack queue  
 *
 *  @return The element at the queue's head if the queue was non-empty, NULL
 *    otherwise
 */
generic_node_t* stack_queue_dequeue(stack_queue_t* queue) {

  assert(queue != NULL && queue->init == QUEUE_INITIALIZED_TRUE);
  
  if (queue->head == NULL) {
    // The queue is empty
    return NULL;
  }

  // The queue is non-empty
  generic_node_t* ret = queue->head; 
  queue->head = queue->head->next;
  if (queue->head == NULL) {
    queue->tail = NULL;
  }

  return ret;
}

/** @brief Checks whether the queue is empty 
 *
 *  @param queue      The stack queue  
 *
 *  @return 1 if the queue is empty, 0 otherwise
 */
int is_stack_queue_empty(stack_queue_t* queue) {

  assert(queue != NULL && queue->init == QUEUE_INITIALIZED_TRUE);
  return (queue->head == NULL) ? 1 : 0;

}
