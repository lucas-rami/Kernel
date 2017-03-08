/** @file kernel_state.h
 *  @brief This file contains the declaration for the kernel_state_t data
 *  structure.
 *  @author akanjani, lramire1
 */


typedef struct kernel {

  /* Holds the TID of the thread currently running,
   * only works for kernel running on uniprocessor */
  int current_tid;

} kernel_t;

kernel_t kernel;
