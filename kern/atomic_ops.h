/** @file atomic_ops.h
 *  @brief This file contains the declaration for functions that perform
 *  different atomic operations
 *  @author akanjani, lramire1
 */

#ifndef _ATOMIC_OPS_H_
#define _ATOMIC_OPS_H_

// TODO: doc
int atomic_add_and_update(int *i, int j);
int atomic_exchange(void* addr, int value);

#endif /* _ATOMIC_OPS_H_ */
