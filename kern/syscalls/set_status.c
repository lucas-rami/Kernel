#include <kernel_state.h>

void kern_set_status(int status) {
  kernel.current_thread->task->return_status = status;
}
