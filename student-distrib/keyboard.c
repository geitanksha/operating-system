/** Keyboard Driver
 * rtc.c
*/
#include "keyboard.h"
#include "terminal.h"
#include "lib.h"
#include "i8259.h"
#include "filesys.h"


int8_t term_2_flag = 0;
int8_t term_3_flag = 0;

// used code from here https://wiki.osdev.org/PS/2_Keyboard

/* A map that can directly print the character relative to the scan code. 
   This one handles expected outputs when shift is not held down and caps lock is off. */
uint8_t scancode_map_normal[KEYBOARD_INPUT_RANGE] = {
    '\0', '\0', '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '0', '-', '=', '\0', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
    'o', 'p', '[', ']', '\n', '\0', 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
    '\'', '`', '\0', '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', '\0', '*',
    '\0', ' ', '\0'
};

/* A map that can directly print the character relative to the scan code. 
   This one handles expected outputs when shift is held down.           */
 uint8_t scancode_map_shift[KEYBOARD_INPUT_RANGE] = {
    '\0', '\0', '!', '@', '#', '$', '%', '^',
    '&', '*', '(', ')', '_', '+', ' ', '\t',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',
    'O', 'P', '{', '}', '\n', '\0', 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
    '\"', '~', '\0', '|', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', '<', '>', '?', '\0', '*',
    '\0', ' ', '\0'
 };

/* A map that can directly print the character relative to the scan code. 
   This one handles expected outputs when caps lock is on.              */
 uint8_t scancode_map_caps_lock[KEYBOARD_INPUT_RANGE] = {
    '\0', '\0', '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '0', '-', '=', '\0', '\t',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',
    'O', 'P', '[', ']', '\n', '\0', 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ';',
    '\'', '`', '\0', '\\', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', ',', '.', '/', '\0', '*',
    '\0', ' ', '\0'
};

/* A map that can directly print the character relative to the scan code.
   This one handles expected outputs when shift is held down with caps lock on. */
uint8_t scancode_map_shift_and_caps_lock[KEYBOARD_INPUT_RANGE] = {
    '\0', '\0', '!', '@', '#', '$', '%', '^',
    '&', '*', '(', ')', '_', '+', '\0', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
    'o', 'p', '{', '}', '\n', '\0', 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ':',
    '\"', '~', '\0', '|', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', '<', '>', '?', '\0', '*',
    '\0', ' ', '\0'
};

/*
void keyboard_init(void)
Description: Initialize the Keyboard and relevant variables
Inputs: none
Outputs: none
Side Effects: Enables interripts on IRQ 1.
*/
void keyboard_init(void) {
    capslock = 0;
    shiftflag = 0;
    ctrlflag = 0;
    altflag = 0;
    terminalArray[currTerminal].currkey = 0;
    charcount = 0;
    enterflag = 0;
    enable_irq(KEYBOARD_IRQ);
    return;
}

/*
void keyboard_handler(void)
Description: Keyboard's Interrupt Handler
Inputs: none
Outputs: none
Side Effects: Prints what was typed on the keyboard.
*/
extern void keyboard_handler(void) {
    uint32_t keycode = inb(KEYBOARD_PORT);

    /* This chunk of logic sets the flags for function keys */
    if((keycode == KEYBOARD_SHIFT_DOWN) || (keycode == KEYBOARD_SHIFT_DOWN2)){
        shiftflag = 1;
    }
    else if((keycode == KEYBOARD_SHIFT_UP) || (keycode == KEYBOARD_SHIFT_UP2)){
        shiftflag = 0;
    }
    else if(keycode == KEYBOARD_CAPS_LOCK){
        if(capslock){
            capslock = 0;
        }
        else{
            capslock = 1;
        }     
    }
    else if(keycode == KEYBOARD_ALT_DOWN){
        altflag = 1;
    }
    else if(keycode == KEYBOARD_ALT_UP){
        altflag = 0;
    }
    else if(keycode == KEYBOARD_CTRL_DOWN){
        ctrlflag = 1;
    }
    else if(keycode == KEYBOARD_CTRL_UP){
        ctrlflag = 0;
    }

    /* This logic handles our Ctrl+L clear screen functionality */
    if((ctrlflag) && (keycode == KEYBOARD_L_KEY_DOWN)){
        int i;
        clear();
        terminalArray[currTerminal].currkey = 0;
        ctrlflag = 0;
        for(i=0; i<KEYBOARD_BUFFER_MAX_SIZE; i++){
            keyboardbuffer[i] = '\0';
        }
        terminalArray[currTerminal].cursor_x = 0;
        terminalArray[currTerminal].cursor_y = 0;
        update_cursor(terminalArray[currTerminal].cursor_x, terminalArray[currTerminal].cursor_y);
        send_eoi(KEYBOARD_IRQ);
        return;
    }

    if((altflag) && (keycode == KEYBOARD_F1_DOWN)){
        if(currTerminal != 0){
            terminal_switch(TERMINAL_1_NUM);
            send_eoi(KEYBOARD_IRQ);
            return;
        }
    }
    else if((altflag) && (keycode == KEYBOARD_F2_DOWN)){
        if(currTerminal != 1){
            terminal_switch(TERMINAL_2_NUM);
            send_eoi(KEYBOARD_IRQ);
            return;
        }
    }
    else if((altflag) && (keycode == KEYBOARD_F3_DOWN)){
        if(currTerminal != 2){
            terminal_switch(TERMINAL_3_NUM);
            send_eoi(KEYBOARD_IRQ);
            return;
        }
    }
    
    /* If keycode is out of range or is one of the function keys already processed above, we simply send an EOI and leave. */
    if ((keycode < 0) || (keycode >= KEYBOARD_INPUT_RANGE) || (keycode == KEYBOARD_CAPS_LOCK) || (keycode == KEYBOARD_CTRL_DOWN) || (keycode == KEYBOARD_ALT_DOWN) || (keycode == KEYBOARD_SHIFT_DOWN) || (keycode == KEYBOARD_SHIFT_DOWN2) || (terminalArray[currTerminal].currkey >=KEYBOARD_BUFFER_MAX_SIZE)) {
        send_eoi(KEYBOARD_IRQ);
        return;
    }

    /* Otherwise we retreive the character pressed. */
    char output;
    if((capslock == 1) && (shiftflag == 1)){
        output = scancode_map_shift_and_caps_lock[keycode];
    }
    else if(capslock == 1){
        output = scancode_map_caps_lock[keycode];
    }
    else if(shiftflag == 1){
        output = scancode_map_shift[keycode];
    }
    else{
        output = scancode_map_normal[keycode];
    }

    /* Below is the logic for backspacing characters */
    if(keycode == KEYBOARD_BACKSPACE){

        if(terminalArray[currTerminal].currkey == 0){
            send_eoi(KEYBOARD_IRQ);
            return;
        }

        /* First we have the logic for backspacing tab characters, since they are more complicated than most */
        if(keyboardbuffer[terminalArray[currTerminal].currkey-1] == '\t'){
            if(terminalArray[currTerminal].cursor_x > TAB_SIZE-1){             // max offset of tab from edge of screen
                terminalArray[currTerminal].cursor_x--;
                putc2Keyboard(' ');
                terminalArray[currTerminal].cursor_x--;
                terminalArray[currTerminal].cursor_x--;
                putc2Keyboard(' ');
                terminalArray[currTerminal].cursor_x--;
                terminalArray[currTerminal].cursor_x--;
                putc2Keyboard(' ');
                terminalArray[currTerminal].cursor_x--;
                terminalArray[currTerminal].cursor_x--;
                putc2Keyboard(' ');
                terminalArray[currTerminal].cursor_x--;
            }
            else if(terminalArray[currTerminal].cursor_x == TAB_SIZE-1){       // 3 is the offset of tab from edge of screen
                terminalArray[currTerminal].cursor_x--;
                putc2Keyboard(' ');
                terminalArray[currTerminal].cursor_x--;
                terminalArray[currTerminal].cursor_x--;
                putc2Keyboard(' ');
                terminalArray[currTerminal].cursor_x--;
                terminalArray[currTerminal].cursor_x--;
                putc2Keyboard(' ');
                terminalArray[currTerminal].cursor_x--;
                terminalArray[currTerminal].cursor_y--;
                terminalArray[currTerminal].cursor_x = NUM_COLS-1;
                putc2Keyboard(' ');
                terminalArray[currTerminal].cursor_y--;
                terminalArray[currTerminal].cursor_x = NUM_COLS-1;
            }
            else if(terminalArray[currTerminal].cursor_x == TAB_SIZE-2){       // 2 is the offset of tab from edge of screen
                terminalArray[currTerminal].cursor_x--;
                putc2Keyboard(' ');
                terminalArray[currTerminal].cursor_x--;
                terminalArray[currTerminal].cursor_x--;
                putc2Keyboard(' ');
                terminalArray[currTerminal].cursor_x--;
                terminalArray[currTerminal].cursor_y--;
                terminalArray[currTerminal].cursor_x = NUM_COLS-1;
                putc2Keyboard(' ');
                terminalArray[currTerminal].cursor_y--;
                terminalArray[currTerminal].cursor_x = NUM_COLS-1;
                terminalArray[currTerminal].cursor_x--;
                putc2Keyboard(' ');
                terminalArray[currTerminal].cursor_x--;
            }
            else if(terminalArray[currTerminal].cursor_x == TAB_SIZE-3){       // 1 is the offset of tab from edge of screen
                terminalArray[currTerminal].cursor_x--;
                putc2Keyboard(' ');
                terminalArray[currTerminal].cursor_x--;
                terminalArray[currTerminal].cursor_y--;
                terminalArray[currTerminal].cursor_x = NUM_COLS-1;
                putc2Keyboard(' ');
                terminalArray[currTerminal].cursor_y--;
                terminalArray[currTerminal].cursor_x = NUM_COLS-1;
                terminalArray[currTerminal].cursor_x--;
                putc2Keyboard(' ');
                terminalArray[currTerminal].cursor_x--;
                terminalArray[currTerminal].cursor_x--;
                putc2Keyboard(' ');
                terminalArray[currTerminal].cursor_x--;
            }
            else if(terminalArray[currTerminal].cursor_x == TAB_SIZE-4){       // 0 is the offset of tab from edge of screen
                terminalArray[currTerminal].cursor_y--;
                terminalArray[currTerminal].cursor_x = NUM_COLS-1;
                putc2Keyboard(' ');
                terminalArray[currTerminal].cursor_y--;
                terminalArray[currTerminal].cursor_x = NUM_COLS-1;
                terminalArray[currTerminal].cursor_x--;
                putc2Keyboard(' ');
                terminalArray[currTerminal].cursor_x--;
                terminalArray[currTerminal].cursor_x--;
                putc2Keyboard(' ');
                terminalArray[currTerminal].cursor_x--;
                terminalArray[currTerminal].cursor_x--;
                putc2Keyboard(' ');
                terminalArray[currTerminal].cursor_x--;
            }
            terminalArray[currTerminal].currkey = terminalArray[currTerminal].currkey - 1;
            charcount = charcount - TAB_SIZE;
            keyboardbuffer[terminalArray[currTerminal].currkey] = output;
            update_cursor(terminalArray[currTerminal].cursor_x, terminalArray[currTerminal].cursor_y);
            send_eoi(KEYBOARD_IRQ);
            return;
        }

        /* After handling tab, we handle the rest of the characters*/

        /* First we have the case where a character is not at the left edge of the screen on a lower line */
        else if(terminalArray[currTerminal].cursor_x > 0){
            terminalArray[currTerminal].cursor_x--;
            putc2Keyboard(' ');
            terminalArray[currTerminal].currkey = terminalArray[currTerminal].currkey - 1;
            charcount = charcount - 1;
            keyboardbuffer[terminalArray[currTerminal].currkey] = output;
            terminalArray[currTerminal].cursor_x--;
            update_cursor(terminalArray[currTerminal].cursor_x, terminalArray[currTerminal].cursor_y);
            send_eoi(KEYBOARD_IRQ);
            return;
        }
        /* Next we have the case of a character being at the left edge of bottom */
        else if((terminalArray[currTerminal].cursor_x < BACKSPACE_CHAR_BUFF1) && ( (charcount + BACKSPACE_CHAR_BUFF2) > NUM_COLS-1)){  // farthest right index in our 80-column row
            terminalArray[currTerminal].cursor_y--;
            terminalArray[currTerminal].cursor_x = NUM_COLS-1;   // farthest right index in our 80-column row
            putc2Keyboard(' ');
            terminalArray[currTerminal].currkey = terminalArray[currTerminal].currkey - 1;
            charcount = charcount - 1;
            keyboardbuffer[terminalArray[currTerminal].currkey] = output;
            terminalArray[currTerminal].cursor_x = NUM_COLS-1;   // farthest right index in our 80-column row
            terminalArray[currTerminal].cursor_y--;
            update_cursor(terminalArray[currTerminal].cursor_x, terminalArray[currTerminal].cursor_y);
            send_eoi(KEYBOARD_IRQ);
            return;
        }
    }

    if(keycode == KEYBOARD_TAB){
        if(terminalArray[currTerminal].currkey < KEYBOARD_BUFFER_MAX_SIZE - TAB_SIZE){  // prints tab if one can fit in buffer
            keyboardbuffer[terminalArray[currTerminal].currkey] = output;
            terminalArray[currTerminal].currkey++;
            charcount = charcount + TAB_SIZE;
            putc2Keyboard(' ');
            putc2Keyboard(' ');
            putc2Keyboard(' ');
            putc2Keyboard(' '); 
        }
        send_eoi(KEYBOARD_IRQ);
        return;
    }

    /* We next add that character to our buffer and print it to the screen if there is still room in the buffer */
    if(terminalArray[currTerminal].currkey < (KEYBOARD_BUFFER_MAX_SIZE - 1)){  // keyboard buffer size -1, since last char can only be newline
        keyboardbuffer[terminalArray[currTerminal].currkey] = output;
        terminalArray[currTerminal].currkey = terminalArray[currTerminal].currkey + 1;
        charcount = charcount + 1;
        if(keycode == KEYBOARD_ENTER){
            putc2Keyboard(output);
            enterflag = 1;
        }
        else
        putc2Keyboard(output);
    }

    /* The last space in the buffer is reserved for newline */
    else if((keycode == KEYBOARD_ENTER) && (terminalArray[currTerminal].currkey == (KEYBOARD_BUFFER_MAX_SIZE - 1))){    // handles enter when buffer is full/only spot left is the one for newline
        keyboardbuffer[terminalArray[currTerminal].currkey] = output;
        terminalArray[currTerminal].currkey = terminalArray[currTerminal].currkey + 1;
        charcount = charcount + 1;
        enterflag = 1;
        putc2Keyboard(output);
    }
    ctrlflag = 0;
    send_eoi(KEYBOARD_IRQ);
    return;
}

/*
int32_t keyboard_open(const uint8_t* filename)
Description: Keyboard's open function
Inputs: const uint8_t* filename = name of file to be opened
Outputs: returns int32_t = 0 on success
*/
int32_t keyboard_open(const uint8_t* filename){
    if(filename == NULL) return -1;
    return 0;
}

/*
int32_t keyboard_close(int32_t fd)
Description: Keyboard's close function
Inputs: int32_t fd = fild descriptor to close
Outputs: returns int32_t = 0 on success
*/
int32_t keyboard_close(int32_t fd){
    if(fd > MAX_FD_VAL || fd < MIN_FD_VAL) return -1;
    return 0;
}

/*
int32_t keyboard_write(int32_t fd, uint8_t* buf, int32_t nbytes)
Description: Keyboard's read function.
Inputs: int32_t fd     = unused for now
        uint8_t* buf   = unused for now
        int32_t nbytes = unused for now
Outputs: returns int32_t = 0 on success
*/
int32_t keyboard_write(int32_t fd, const void* buf, int32_t nbytes){
    if(buf == NULL || fd > MAX_FD_VAL || fd < MIN_FD_VAL) return -1;
    if(nbytes < 0) return -1;
    return 0;
}

/*
int32_t keyboard_read(int32_t fd, uint8_t* buf, int32_t nbytes)
Description: Keyboard's read function.
Inputs: int32_t fd     = unused for now
        uint8_t* buf   = unused for now
        int32_t nbytes = unused for now
Outputs: returns 0 on success
*/
int32_t keyboard_read(int32_t fd, void* buf, int32_t nbytes){
    if(buf == NULL || fd > MAX_FD_VAL || fd < MIN_FD_VAL) return -1;
    if(nbytes < 0) return -1;
    return 0;
}




