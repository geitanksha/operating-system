// #include "rtc.h"

#ifndef _SCHED_H
#define _SCHED_H

 
#include "lib.h"
#include "i8259.h"
#include "scheduler.h"

#define PIT_WAVE_INIT   0x32
#define PIT_CMD_PORT    0x43
#define CHAN_0          0x40
#define CHAN_1          0x41
#define CHAN_2          0x42
#define MODE_CMD_REG    0x43 // [0: BCD/Binary mode] [1-3: Operating Mode] [4-5: Access mode][6-7:Select Channel]
#define PIT_IRQ         0
#define FREQUENCY       1193180 // 1193182?
#define FREQUENCY_RT    100
#define LOW_BYTE        0xFF
#define EIGHT_SHIFT     8
#define PREVPCB0        0x7fe000
#define PREVPCB1        0x7fc000
#define PREVPCB2        0x7fa000
#define PIT_INIT_CONST  1193180
#define PIT_INIT_FREQ   100
#define PIT_CMD_NUM     0x36

volatile int terminalrun = -1;
/* Defined Functions */
void pit_handler();
void pit_init();

void scheduler();

#endif
