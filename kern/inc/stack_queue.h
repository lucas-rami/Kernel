/** @file stack_queue.h
 *
 *  @brief This file specifies the stack_queue_t type, which represents a queue 
 *    of elements stored on the kernel stack of various threads. This queue is
 *    meant to be used when the enqueue operation is followed by a deschedule
 *    system call.
 *    The file also declares the functions used to manipulate the queue.
 *
 *  @author akanjani, lramire1
 */

#ifndef _STACK_QUEUE_H_
#define _STACK_QUEUE_H_

#define QUEUE_INITIALIZED_FALSE 0
#define QUEUE_INITIALIZED_TRUE 1

#include <generic_node.h>

/*  @brief Structure representing a queue whose elements are stored on the
 *    kernel stack of various threads */
typedef struct stack_queue {

  /*  @brief Pointer to the queue's head*/
  generic_node_t * head;
  
  /*  @brief Pointer to the queue's tail*/  
  generic_node_t * tail;

  /*  @breif Field indicating whether the queue is initialized*/
  int init;

} stack_queue_t;


void stack_queue_init(stack_queue_t* queue);
void stack_queue_destroy(stack_queue_t* queue);
void stack_queue_enqueue(stack_queue_t* queue, generic_node_t* new_elem);
generic_node_t* stack_queue_dequeue(stack_queue_t* queue);
int is_stack_queue_empty(stack_queue_t* queue);

#endif /* _STACK_QUEUE_H_ */
