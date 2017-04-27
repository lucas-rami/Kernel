/** @file   loader.c
 *  @brief  This file contains the definition for the get_bytes() function,
 *          which allows to copy data from a file into a buffer.
 *  @author akanjani, lramire1
 */

#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <exec2obj.h>
#include <loader.h>
#include <elf_410.h>


/** @brief  Copies data from a file into a provided buffer
 * 
 *  The call will fail if at least one of this condition is met:
 *  - the size and/or offset arguments are lesser than 0
 *  - the offset is bigger than the file's size
 *  - no file exists with the given filename
 *
 *  The function assumes that the buffer validity has been checked before by the
 *  invoking thread. The buffer should exist within the current task's address
 *  space and be big enough to contain size bytes.
 *
 *  @param  filename   The name of the file to copy data from
 *  @param  offset     The location in the file to begin copying from
 *  @param  size       The number of bytes to be copied
 *  @param  buf        The buffer to copy the data into
 *
 * @return  Returns the number of bytes copied on succes, -1 on failure
 */
int getbytes( const char *filename, int offset, int size, char *buf ) {

  // Check that the size and offset arguments are positive
  if (size < 0 || offset < 0) {
    return -1;
  }

  int i;
  for (i = 0; i < MAX_NUM_APP_ENTRIES; i++) {
    if (!strcmp(exec2obj_userapp_TOC[i].execname, filename)) {

      // If offset is greater than the file's size, return an error
      if (offset > exec2obj_userapp_TOC[i].execlen) {
        return -1;
      }

      // Compute the amount of bytes to copy from the file
      int len = (exec2obj_userapp_TOC[i].execlen - offset < size) ?
                exec2obj_userapp_TOC[i].execlen - offset : size;

      // Copy file content into buffer
      memcpy(buf, exec2obj_userapp_TOC[i].execbytes + offset, len);

      return len;
    }
  }

  // No file exists with the given filename
  return -1;
}

