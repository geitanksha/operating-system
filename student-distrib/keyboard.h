/* i8259.h - Defines used in interactions with the 8259 interrupt
 * controller
 * vim:ts=4 noexpandtab
 */

#include "lib.h"
#include "i8259.h"


#ifndef _KEYBOARD_H
#define _KEYBOARD_H


 /* Defined variables as used in the PIC functions */
#define KEYBOARD_IRQ                0x01
#define KEYBOARD_INPUT_RANGE        59

/* Defined variables as given by OsDev: https://wiki.osdev.org/PS/2_Keyboard */
#define KEYBOARD_SHIFT_DOWN         42
#define KEYBOARD_SHIFT_DOWN2        54
#define KEYBOARD_SHIFT_UP           170
#define KEYBOARD_SHIFT_UP2          182
#define KEYBOARD_CAPS_LOCK          58
#define KEYBOARD_ALT_DOWN           56
#define KEYBOARD_ALT_UP             184
#define KEYBOARD_CTRL_DOWN          29
#define KEYBOARD_CTRL_UP            158
#define KEYBOARD_ENTER              28
#define KEYBOARD_BACKSPACE          14
#define KEYBOARD_TAB                15
#define KEYBOARD_L_KEY_DOWN         38
#define KEYBOARD_F1_DOWN            59
#define KEYBOARD_F2_DOWN            60
#define KEYBOARD_F3_DOWN            61
#define BACKSPACE_CHAR_BUFF1        10
#define BACKSPACE_CHAR_BUFF2        24

/* Ports that each Keyboard sits on */
#define KEYBOARD_PORT               0x60

/* Miscellanious self-explanatory constants */
#define KEYBOARD_BUFFER_MAX_SIZE    128
#define TAB_SIZE                    4

/* Buffer for characters written to terminal */
uint8_t keyboardbuffer[128];
int keyboardbuffersize;
int charcount;
int capslock;
volatile int shiftflag;
volatile int ctrlflag;
volatile int altflag;
int enterflag;

/* Externally-visible functions */

/* Initialize both PICs */
void keyboard_init(void);
/* Keyboard's Interrupt Handler */
extern void keyboard_handler(void);

int32_t keyboard_open(const uint8_t* filename);
int32_t keyboard_close(int32_t fd);
int32_t keyboard_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t keyboard_read(int32_t fd, void* buf, int32_t nbytes);



#endif /* _KEYBOARD_H */
