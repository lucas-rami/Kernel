/** @file atomic_ops.h
 *  @brief This file contains the declaration for functions that perform
 *  different atomic operations
 *  @author akanjani, lramire1
 */

#ifndef _ATOMIC_OPS_H_
#define _ATOMIC_OPS_H_

/** @brief  Atomically add j to the variable pointed to by i
 *
 *  @param  i   The address of the variable that j is added to
 *  @param  j   The value added to the variable stored at address i
 *
 *  @return The old value stored at address i, before the addition was perfomed
 */
int atomic_add_and_update(int *i, int j);

/** @brief  Exchange the value of a variable with a new provided value
 *
 *  @param  addr  The address of the variable that is gonna take the new value
 *  @param  value The new value to exchange with the old one
 *
 *  @return The olf value stored at address addr, before the exchange was
 *          performed
 */
int atomic_exchange(void* addr, int value);

#endif /* _ATOMIC_OPS_H_ */
