
int swexn(void *esp3, swexn_handler_t eip, void *arg, ureg_t *newureg) {

  if ((unsigned)eip < USER_MEM_START || (unsigned)esp3 < USER_MEM_START) {
    // Invalid args
    return -1;
  }

  // PRECHECKS
  if (newureg != NULL) {
    // Check if memory address is valid for the whole structure. If yes,
    // copy it to our stack
    // Otherwise, throw an error
  }

  // Store the current esp of the exception stack and the eip to restore in
  // case something goes bad
  if (esp3 == NULL || eip == NULL) {
    // Deregister the current handler
  } else {
    // Register this handler
  }

  if (newureg != NULL) {
    // The values on return should be that in the newureg
  }
  return 0;
} 
