/** @file sleep.c
 *  @brief This file contains the definition for the sleep() system call and
 *    functions handling sleeping threads
 *  @author akanjani, lramire1
 */

#include <kernel_state.h>
#include <scheduler.h>
#include <generic_node.h>
#include <stdlib.h>
#include <asm.h>

/* Debugging */
#include <simics.h>

/** @brief  Data structure representing a sleeping thread */
typedef struct sleeper {

  /** @brief  Number of remaining ticks to sleep */
  int ticks;

  /** @brief  Sleeper's TCB */  
  tcb_t* tcb;
  
} sleeper_t;

/* File variables */
static int ticks_buffer = 0;
static int ticks_next_update = 0;
static generic_double_node_t* head = NULL;
static generic_double_node_t* tail = NULL;

/** @brief  Deschedules the calling thread until at least ticks timer interrupts
 *          have occurred after the call.
*
 *  @param  ticks   The number of timer interrupts before waiking up the thead 
 *
 *  @return 0 on success (after sleeping or if ticks is 0), a negative number 
 *          if ticks is negative
 */
int kern_sleep(int ticks) {
  
  // Check validity of arguments
  if (ticks == 0) {
    return 0;
  } else if (ticks < 0) {
    return -1;
  }

  sleeper_t new_sleeper = {ticks, kernel.current_thread};
  generic_double_node_t new_node = {&new_sleeper, NULL, NULL};

  // We don't want any timer interrupt during this operation
  disable_interrupts();

  generic_double_node_t* it = head;
  if (head == NULL) {
    // Queue is empty

    head = (tail = &new_node);
    ticks_next_update = ticks;
  } else {
    // Queue is non-empty

    new_sleeper.ticks += ticks_buffer;

    // Find the good position in the queue
    while(it != NULL && ((sleeper_t*)it->value)->ticks < new_sleeper.ticks) {
      it = it->next;
    }

    // Insert the new node at the correct position
    if (it == NULL) {
      // Insert new element at queue's tail

      new_node.prev = tail;
      tail->next = &new_node;
      tail = &new_node;
    } else {
      // Insert new element between two nodes
      new_node.next = it;
      
      if (it->prev != NULL) {
        // The new element 
        new_node.prev = it->prev;
        it->prev->next = &new_node;
      } else {
        // The new element is the new head
        head = &new_node;
        // Update the time to next update
        ticks_next_update = ticks;
      }

      it->prev = &new_node;
    }

  }

  // Block the thread and context switch
  // (interrupts will be enabled after context switch)
  block_and_switch(HOLDING_MUTEX_FALSE, NULL);

  return 0;
}

/** @brief  Callback function for timer interrupt handler, wakes up all threads
 *          that have been sleeping for the right amount of ticks
 *
 *  @brief  ticks   The total number of ticks since system boot
 *
 *  @return void
 */
void wake_up_threads(unsigned int ticks) {

  // If no one is sleeping, return immediately
  if (ticks_next_update == 0) {
    return;
  }

  ++ticks_buffer;
  --ticks_next_update;

  if (ticks_next_update == 0) {
    // There must be at least one element in the queue

    generic_double_node_t* node = head;
    sleeper_t* sleeper = head->value;
  
    while (node != NULL && sleeper->ticks == ticks_buffer) { 
      add_runnable_thread(sleeper->tcb); 
      node = node->next;
      sleeper = (node == NULL) ? NULL : node->value;
    }

    if (node == NULL) {
      // The queue is now empty
      head = (tail = NULL);
      ticks_next_update = 0;
      ticks_buffer = 0;
    } else {
      // There are still elements in the queue
      head = node;
      node->prev = NULL;
      ticks_next_update = sleeper->ticks - ticks_buffer;

    }

  }

}
