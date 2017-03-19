/** @file thr_internals.h
 *
 *  @brief This file may be used to define things
 *         internal to the thread library.
 *
 *  @author akanjani, lramire1
 */

#ifndef THR_INTERNALS_H
#define THR_INTERNALS_H

#include <mutex.h>
#include <global_state.h>
#include <ureg.h>

/** @brief An assembly wrapper for the thread_fork system call
 *
 *  @param child_esp A pointer to the child's stack
 *
 *  @return 0 on success, a negative error code otherwise
 */
int thread_fork(void *child_esp);

/** @brief An assembly function to get the stack pointer of the current thread
 *
 *  @return unsigned int The stack pointer of the current thread
 */
unsigned int get_esp(void);

void stub(void *(*func)(void *), void *arg, void* addr_exception_stack);
tcb_t* get_tcb(void);
int find_tcb(void* tcb, void* tid);
unsigned int hash_function_tcb(void* tcb, unsigned int nb_buckets);

int thr_get_kernel_id(int library_tid);
int thr_get_my_kernel_id();

#endif /* THR_INTERNALS_H */
