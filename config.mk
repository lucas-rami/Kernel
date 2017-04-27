###########################################################################
# This is the include file for the make file.
# You should have to edit only this file to get things to build.
###########################################################################

###########################################################################
# Tab stops
###########################################################################
# If you use tabstops set to something other than the international
# standard of eight characters, this is your opportunity to inform
# our print scripts.
TABSTOP = 8

###########################################################################
# The method for acquiring project updates.
###########################################################################
# This should be "afs" for any Andrew machine, "web" for non-andrew machines
# and "offline" for machines with no network access.
#
# "offline" is strongly not recommended as you may miss important project
# updates.
#
UPDATE_METHOD = afs

###########################################################################
# WARNING: When we test your code, the two TESTS variables below will be
# blanked.  Your kernel MUST BOOT AND RUN if 410TESTS and STUDENTTESTS
# are blank.  It would be wise for you to test that this works.
###########################################################################

###########################################################################
# Test programs provided by course staff you wish to run
###########################################################################
# A list of the test programs you want compiled in from the 410user/progs
# directory.
#
410TESTS = cat mem_permissions readline_basic print_basic exec_basic exec_basic_helper knife peon merchant coolness fork_bomb new_pages remove_pages_test1 remove_pages_test2 swexn_basic_test swexn_cookie_monster swexn_dispatch swexn_regs swexn_stands_for_swextensible swexn_uninstall_test stack_test1 actual_wait fork_wait_bomb fork_wait wait_getpid minclone_mem sleep_test1 fork_exit_bomb make_crash make_crash_helper cho cho2 cho_variant wild_test1 yield_desc_mkrun loader_test1 loader_test2 getpid_test1

###########################################################################
# Test programs you have written which you wish to run
###########################################################################
# A list of the test programs you want compiled in from the user/progs
# directory.
#
STUDENTTESTS = pages_alloc_test

###########################################################################
# Data files provided by course staff to build into the RAM disk
###########################################################################
# A list of the data files you want built in from the 410user/files
# directory.
#
410FILES =

###########################################################################
# Data files you have created which you wish to build into the RAM disk
###########################################################################
# A list of the data files you want built in from the user/files
# directory.
#
STUDENTFILES =

###########################################################################
# Object files for your thread library
###########################################################################
THREAD_OBJS = malloc.o panic.o mutex.o atomic_ops.o cond_var.o queue.o linked_list.o hash_table.o thr_create.o thread_fork.o thr_init.o thr_exit.o thr_join.o tcb.o get_esp.o thr_getid.o thr_yield.o sem.o rwlock.o rwlock_helper.o mutex_asm.o

# Thread Group Library Support.
#
# Since libthrgrp.a depends on your thread library, the "buildable blank
# P3" we give you can't build libthrgrp.a.  Once you install your thread
# library and fix THREAD_OBJS above, uncomment this line to enable building
# libthrgrp.a:
410USER_LIBS_EARLY += libthrgrp.a

###########################################################################
# Object files for your syscall wrappers
###########################################################################
SYSCALL_OBJS = vanish.o set_status.o print.o deschedule.o exec.o fork.o getchar.o gettid.o make_runnable.o readline.o sleep.o swexn.o wait.o yield.o set_term_color.o get_cursor_pos.o set_cursor_pos.o halt.o readfile.o task_vanish.o new_pages.o remove_pages.o get_ticks.o misbehave.o

###########################################################################
# Object files for your automatic stack handling
###########################################################################
AUTOSTACK_OBJS = autostack.o page_fault_handler.o

###########################################################################
# Parts of your kernel
###########################################################################
#
# Kernel object files you want included from 410kern/
#
410KERNEL_OBJS = load_helper.o
#
# Kernel object files you provide in from kern/
#
KERNEL_OBJS = stack_queue.o virtual_memory_helper.o virtual_memory_asm.o kernel_state.o hash_table.o linked_list.o mutex.o kernel.o loader.o malloc_wrappers.o interrupts.o queue.o page_fault_asm.o page_fault_handler.o virtual_memory.o bitmap.o idt_syscall.o task_create.o context_switch_asm.o context_switch.o static_queue.o scheduler.o cond_var.o dynamic_queue.o atomic_ops.o sw_exception.o exception_handlers.o exception_handlers_asm.o

# Files in drivers/
KERNEL_OBJS += drivers/console.o drivers/keyboard.o drivers/keyboard_asm.o drivers/prechecks.o drivers/timer.o drivers/timer_asm.o

# Files in syscalls/
KERNEL_OBJS += syscalls/readfile.o syscalls/set_status.o syscalls/get_ticks.o syscalls/sleep.o syscalls/gettid.o syscalls/scheduling_calls.o syscalls/fork.o syscalls/exec.o syscalls/pages.o syscalls/console_io.o syscalls/vanish.o syscalls/wait.o syscalls/swexn.o

# Files in syscalls/wrappers/
KERNEL_OBJS += syscalls/wrappers/readfile.o syscalls/wrappers/set_status.o syscalls/wrappers/get_ticks.o syscalls/wrappers/halt.o syscalls/wrappers/sleep.o syscalls/wrappers/gettid.o syscalls/wrappers/scheduling_calls.o syscalls/wrappers/fork.o syscalls/wrappers/syscalls_helper.o syscalls/wrappers/exec.o syscalls/wrappers/pages.o syscalls/wrappers/console_io.o syscalls/wrappers/vanish.o syscalls/wrappers/wait.o syscalls/wrappers/exec.o syscalls/wrappers/swexn.o

###########################################################################
# WARNING: Do not put **test** programs into the REQPROGS variables.  Your
#          kernel will probably not build in the test harness and you will
#          lose points.
###########################################################################

###########################################################################
# Mandatory programs whose source is provided by course staff
###########################################################################
# A list of the programs in 410user/progs which are provided in source
# form and NECESSARY FOR THE KERNEL TO RUN.
#
# The shell is a really good thing to keep here.  Don't delete idle
# or init unless you are writing your own, and don't do that unless
# you have a really good reason to do so.
#
410REQPROGS = idle init shell

###########################################################################
# Mandatory programs whose source is provided by you
###########################################################################
# A list of the programs in user/progs which are provided in source
# form and NECESSARY FOR THE KERNEL TO RUN.
#
# Leave this blank unless you are writing custom init/idle/shell programs
# (not generally recommended).  If you use STUDENTREQPROGS so you can
# temporarily run a special debugging version of init/idle/shell, you
# need to be very sure you blank STUDENTREQPROGS before turning your
# kernel in, or else your tweaked version will run and the test harness
# won't.
#
STUDENTREQPROGS =
