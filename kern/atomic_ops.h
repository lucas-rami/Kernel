/** @file atomic_ops.h
 *  @brief This file contains the declaration for functions that perform
 *  different atomic operations
 *  @author akanjani, lramire1
 */

#ifndef _ATOMIC_OPS_H_
#define _ATOMIC_OPS_H_

#include <stdint.h>

/** @brief  Atomically adds j to the variable pointed to by i
 *
 *  @param  i   The address of the variable that j is added to
 *  @param  j   The value added to the variable stored at address i
 *
 *  @return The old value stored at address i, before the addition was perfomed
 */
int atomic_add_and_update(void *i, uint32_t j);

/** @brief  Atomically exchanges the value of a variable with a new 
 *          provided value
 *
 *  @param  addr  The address of the variable that is gonna take the new value
 *  @param  value The new value to exchange with the old one
 *
 *  @return The olf value stored at address addr, before the exchange was
 *          performed
 */
int atomic_exchange(void* addr, uint32_t value);

/** @brief  Atomically modifies the value pointed to by addr with new_val if
 *          the value stored at addr is old_val   
 *
 *  @param  addr      The variable's address
 *  @param  old_val   The value that we expect at addr
 *  @param  new_val   The value to replace old_val with
 *
 *  @return 1 if the value stored at addr was old_val (thus the value at addr
 *          has been changed with new_val), 0 otherwise (nothing changed)
 */
int atomic_compare_and_exchange_32(void* addr, uint32_t old_val, 
                                    uint32_t new_val);

#endif /* _ATOMIC_OPS_H_ */
