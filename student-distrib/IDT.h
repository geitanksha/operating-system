/* IDT.h - Defines used in interactions with the IDT
 */

// #ifndef _IDT_H
// #define _IDT_H

#define KEYBOARD_IDT_ENTRY          0x21
#define RTC_IDT_ENTRY               0x28
#define PIT_IDT_ENTRY               0x20
#define TOTAL_IDT_ENTRIES           256
#define EXCEPTION_CALLED            256
#define SYSTEM_CALL_IDT_ENTRY       0x80
#define USER_DPL                    3

/* Function Definitions for the IDT Functions.  */
/* Externally-visible functions */

int (*funcs[TOTAL_IDT_ENTRIES])();
int generic_interrupt();
int divide_error();
int RESERVED();
int NMI();
int breakpoint();
int overflow();
int bound();
int InvalidOpcode();
int WAIT();
int DoubleFalt();
int overrun();
int TSS();
int segment();
int stackSegment();
int protect();
int pageFault();
int RESERVED2();
int FPU();
int allign();
int machine();
int SIMD();
// int system_call_placeholder();
extern void call_handler();
void init_IDT();





// #endif /* _IDT_H */
