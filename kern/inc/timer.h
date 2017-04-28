/** @file timer.h
 *  @brief The file which contains the helper functions needed by the
 *   timer driver.
 *
 *  @author Anirudh Kanjani
 */

#ifndef _TIMER_H_
#define _TIMER_H_

void timer_c_handler();

int timer_init( void ( *tickback )( unsigned int ) );
unsigned int get_global_counter();

#endif /* _TIMER_H_ */
