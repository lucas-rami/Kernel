/** @file exception_handlers_asm.h
 *  @brief  This file contains the declarations for all the exception handlers
 *          wrappers
 *  @author akanjani, lramire1
 */

#ifndef _EXCEPTION_HANDLERS_ASM_H_
#define _EXCEPTION_HANDLERS_ASM_H_

/** @brief Exception handler wrapper for divide exception
 *  @return Does not return 
 */
void divide_handler();
/** @brief Exception handler wrapper for debug exception
 *  @return Does not return 
 */
void debug_handler();
/** @brief Exception handler wrapper for breakpoint exception
 *  @return Does not return 
 */
void breakpoint_handler();
/** @brief Exception handler wrapper for overflow exception
 *  @return Does not return 
 */
void overflow_handler();
/** @brief Exception handler wrapper for boundcheck exception
 *  @return Does not return 
 */
void boundcheck_handler();
/** @brief Exception handler wrapper for opcode exception
 *  @return Does not return 
 */
void opcode_handler();
/** @brief Exception handler wrapper for nofpu exception
 *  @return Does not return 
 */
void nofpu_handler();
/** @brief Exception handler wrapper for segfault exception
 *  @return Does not return 
 */
void segfault_handler();
/** @brief Exception handler wrapper for stackfault exception
 *  @return Does not return 
 */
void stackfault_handler();
/** @brief Exception handler wrapper for protfault exception
 *  @return Does not return 
 */
void protfault_handler();
/** @brief Exception handler wrapper for fpufault exception
 *  @return Does not return 
 */
void fpufault_handler();
/** @brief Exception handler wrapper for alignfault exception
 *  @return Does not return 
 */
void alignfault_handler();
/** @brief Exception handler wrapper for simdfault exception
 *  @return Does not return 
 */
void simdfault_handler();

#endif /* _EXCEPTION_HANDLERS_ASM_H_ */
