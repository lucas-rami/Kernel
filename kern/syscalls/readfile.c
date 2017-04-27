/** @file readfile.c
 *  @brief This file contains the definition for the readfile() system call.
 *  @author akanjani, lramire1
 */

#include <loader.h>
#include <common_kern.h>
#include <virtual_memory.h>

/** @brief  Attempts to fill the user-specified buffer buf with count bytes 
 *          starting offset bytes from the beginning of the RAM disk file 
 *          specified by filename
 *
 *  The call will fail if at least one one of this condition is met:
 *  - no file with the given name exists
 *  - count is negative
 *  - offset is less than zero or greater than the size of the file
 *  - buf is not a valid buffer large enough to store count bytes
 *  In this case, the contents of buf are undefines.
 *
 *  @param  filename   The name of the file to copy data from
 *  @param  buf        The buffer to copy the data into
 *  @param  count       The number of bytes to be copied
 *  @param  offset     The location in the file to begin copying from
 *
 *  @return The number of bytes stored in the buffer on success, a negative
 *          number on failure
 */
int kern_readfile(char *filename, char *buf, int count, int offset) {

  // Check that the count and offset arguments are positive
  if (count < 0 || offset < 0) {
    return -1;
  }

  // Check that the buffer is valid
  if ((unsigned int)buf < USER_MEM_START ||
      is_buffer_valid((unsigned int)buf, count) < 0) {
    return -1;
  }

  // Get the bytes from the file and return
  return getbytes(filename, offset, count, buf);
}
