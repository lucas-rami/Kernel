/** @file atomic_ops.S
 *  @brief This file contains functions that perform actions atomically
 *  @author akanjani, lramire1
 */

.global atomic_exchange

atomic_exchange:
  movl  4(%esp), %edx   // Move &mutex_lock (first argument) into edx
  movl  8(%esp), %eax   // Move val (second argument) into eax
  xchg  (%edx),  %eax   // Exchange *mutex_lock and val atomically
  ret                   // Return from procedure (eax contains the old value)
