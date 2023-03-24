
 #ifndef _TERMINAL_H
 #define _TERMINAL_H

#include "keyboard.h"
#include "lib.h"
#include "i8259.h"
#include "syscall.h"

#include "filesys.h"
#include "paging.h"

#define MAX_TERMINALS           3
#define TERMINAL_1_NUM          0
#define TERMINAL_2_NUM          1
#define TERMINAL_3_NUM          2


int32_t terminal_open(const uint8_t *filename,int32_t fd);
int32_t terminal_close(int32_t fd);
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t terminal_switch(int32_t newTerminal);
void terminal_init();

/* Terminal Struct stuff */
/* Struct for the Termial. */
typedef struct terminalStruct{
   volatile uint32_t currRTC;            // it is virtualized
    uint32_t savedt_esp;
    uint32_t savedt_ebp;
    uint8_t terminalbuffer[KEYBOARD_BUFFER_MAX_SIZE];
    int currkey;
    uint8_t cursor_x;
    uint8_t cursor_y;
    uint32_t vidmemloc; //ba,bb,bc
    uint8_t currprocessid;
    pcb_t*  cur_PCB;
}__attribute__ ((packed)) terminalStruct_t;

volatile uint8_t currTerminal;

terminalStruct_t terminalArray[MAX_TERMINALS];

#endif /* _TERMINAL_H */
