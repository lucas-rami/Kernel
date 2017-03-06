/** @file timer.h
 *  @brief The file which contains the helper functions needed by the
 *   timer driver.
 *
 *  @author Anirudh Kanjani
 */

#ifndef __TIMER_H_
#define __TIMER_H_

void timer_c_handler();

int timer_init( void ( *tickback )( unsigned int ) );

#endif
