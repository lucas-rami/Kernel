/** @file atomic_ops.h
 *  @brief This file defines the prototypes for functions in atomic_ops.S
 *  @author akanjani, lramire1
 */

#ifndef _ATOMIC_OPS_H
#define _ATOMIC_OPS_H

/** @brief Exchanges the value at the address pointed to by the first parameter
 *   with the second parameter
 */
int atomic_exchange(int *mutex_lock, int val);

#endif /* _ATOMIC_OPS_H */
