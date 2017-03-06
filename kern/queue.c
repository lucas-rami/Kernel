/** @file queue.c
 *  @brief Implementation of the enqueue and dequeue functions used to put
 *   and extract characters from the static buffer "queue" of all the 
 *   characters typed on the keyboard maintained by the kernel.
 *
 *  @author Anirudh Kanjani
 */

#include <queue.h>

/* The size of the cyclic static buffer storing the characters entered
 * on the keyboard. The size is chosen to be big enough so that the buffer
 * does not run out of space every time the user puts in a lot of data
 * before reading it and small enough to not take up a lot of space in 
 * memory all the time.
 */
#define QUEUE_SIZE 2048

/* The static buffer cyclic queue storing the characters entered on the 
 * keyboard.
 */
uint8_t queue[QUEUE_SIZE];

/* The front and rear of the cyclic queue buffer */
int front = 0, rear = 0;

/** @brief The function to add the character to the cyclic buffer
 *
 *   Tries to add the character passed as the parameter to the cyclic
 *   buffer. Throws an error when the queue is full. 
 *
 *  @param ch The character from the keyboard to be enqued
 *   
 *  @return A negative error code on error, or 0 on success
 **/
int enqueue( uint8_t ch )
{
	int new_rear = ( rear + 1 ) % QUEUE_SIZE;
	if ( new_rear == front ) {
		// queue is full
		return -1;
	}
	queue[rear] = ch;
	rear = new_rear;
	return 0;
}

/** @brief The function to remove the character from the cyclic buffer
 *
 *   Tries to remove the character from the cyclic queue buffer. 
 *   Throws an error when the queue is empty. 
 *
 *  @param void
 *   
 *  @return A negative error code on error, or 0 on sucess
 **/
int dequeue()
{
	if ( front == rear ) {
		// empty qeueue
		return -1;
	}
	int ret = ( int )queue[front];
	front = ( front + 1 ) % QUEUE_SIZE;
	return ret;
}
