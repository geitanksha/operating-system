# interruptlinkage.S - Set up a macro for the interrupt linkage - convers normal handlers/system calls to ASM wrapped handlers/system calls.
# vim:ts=4 noexpandtab

#define ASM     1
#include "x86_desc.h"

/* Interrupt Handlers */
#define INTR_LINK(name, func)    \
.GLOBL name                     ;\
name:                           ;\
PUSHAL	                        ;\
pushfl                          ;\
call func                       ;\
popfl                           ;\
POPAL                           ;\
IRET

INTR_LINK(rtc_handler_linkage, rtc_handler);

INTR_LINK(keyboard_handler_function, keyboard_handler); 

INTR_LINK(pit_handler_linkage, pit_handler); // interrlink.S

/*
System Call Functions 
#define SYSCALL_LINK(name, func) \
.GLOBL name                     ;\
name:                           ;\
PUSHAL	                        ;\
pushfl                          ;\
pushl %edx                      ;\  # argument 3
pushl %ecx                      ;\  # argument 2
pushl %ebx                      ;\  # argument 1
call func                       ;\
popl %ebx                       ;\
popl %ecx                       ;\
popl %edx                       ;\
popfl                           ;\
POPAL                           ;\
IRET

SYSCALL_LINK(halt_linkage, halt);

SYSCALL_LINK(execute_linkage, execute);

SYSCALL_LINK(read_linkage, read);

SYSCALL_LINK(write_linkage, open);

SYSCALL_LINK(open_linkage, open);

SYSCALL_LINK(close_linkage, close);

SYSCALL_LINK(getargs_linkage, getargs);

SYSCALL_LINK(vidmap_linkage, vidmap);

SYSCALL_LINK(set_handler_linkage, set_handler);

SYSCALL_LINK(sigreturn_linkage, sigreturn);
*/
