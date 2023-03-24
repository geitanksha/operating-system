#include "terminal.h"
#include "keyboard.h"
#include "lib.h"
#include "syscall.h"
#include "i8259.h"
#include "filesys.h"
#include "paging.h"

/*
int32_t terminal_open(const uint8_t* filename)
Description: Terminal's open function
Inputs: const uint8_t* filename = name of file to be opened
Outputs: returns int32_t = 0 on success
*/
extern int terminalrun;



int32_t terminal_open(const uint8_t *filename,int32_t fd){
    if(filename == NULL) return -1;
    return 0;
}

/*
int32_t terminal_close(int32_t fd)
Description: Terminal's close function
Inputs: int32_t fd = fild descriptor to close
Outputs: returns int32_t = 0 on success
*/
int32_t terminal_close(int32_t fd){
    if(fd > MAX_FD_VAL || fd < MIN_FD_VAL) return -1;
    disable_cursor();
    return 0;
}

/*
int32_t terminal_read(int32_t fd, uint8_t* buf, int32_t nbytes)
Description: Terminal's read function. Reads from keyboardbuffer into the buffer passed in as buf
Inputs: int32_t fd     = unused for now
        uint8_t* buf   = buffer to be written to
        int32_t nbytes = number of bytes to read from keyboardbuffer
Outputs: returns int32_t = number of bytes read
*/
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes){
    uint8_t* buft = (uint8_t*)buf;
    if(buft == NULL || fd > MAX_FD_VAL || fd < MIN_FD_VAL) return -1;
    if(nbytes < 0) 
        return -1;
    int i;
    int count = KEYBOARD_BUFFER_MAX_SIZE;
    int j;
    if(nbytes > KEYBOARD_BUFFER_MAX_SIZE){   // handles overflow by just chopping off extra bytes
        nbytes = KEYBOARD_BUFFER_MAX_SIZE;
    }
    /* Will loop through infinitely waiting for an enter key from user to return with their input */
    while(1){
        if((currTerminal == terminalrun)&&(enterflag)){
            int nullflag;
            nullflag = 0;
            /* Will copy keyboardbuffer to buf */
            for(i=0; i < nbytes; i++){
                if(keyboardbuffer[i] == '\0' && !nullflag){
                    count = i;
                    nullflag = 1;
                }
                    //count = i;
                    buft[i] = keyboardbuffer[i];
            }
            /* Clears out keyboardbuffer */
            keyboardbuffersize = 0;
            while(keyboardbuffer[keyboardbuffersize] != '\n'){
                keyboardbuffersize++;
            }
            keyboardbuffersize++;
            for(j=0; j<KEYBOARD_BUFFER_MAX_SIZE; j++){
                keyboardbuffer[j] = '\0';   // clear our buffer to prevent an infinite loop of read/write on the same line 
            }
            terminalArray[currTerminal].currkey = 0;
            //charcount = 0;
            enterflag = 0;
            return count;
        }

    }
}

/*
int32_t terminal_write(int32_t fd, uint8_t* buf, int32_t nbytes)
Description: Terminal's read function. Reads from keyboardbuffer into the buffer passed in as buf
Inputs: int32_t fd     = unused for now
        uint8_t* buf   = buffer to be read from 
        int32_t nbytes = number of bytes to write to screen from buf
Outputs: returns int32_t = 0 on success
*/
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes){
    uint8_t* buft = (uint8_t*)buf;
    if(buft == NULL || fd > MAX_FD_VAL || fd < MIN_FD_VAL) return -1;
    if(nbytes < 0) return -1;
    int i;
    int linecount;

    linecount = charcount/NUM_COLS;
    charcount = 0;
    for(i=0; i<nbytes; i++){
        if(buft[i] != '\0'){
            if(buft[i] == '\t'){ // tab is equivalent to four spaces
                putc2(' ');
                putc2(' ');
                putc2(' ');
                putc2(' '); 
            }
            else {
                putc2(buft[i]); 
            }
        }
    }
    keyboardbuffersize = 0;

    return 0;
}

/*
int32_t terminal_switch(int32_t newTerminal)
Description: Terminal switch function. Swaps the visible terminal given input from the keyboard
Inputs: int32_t newTerminal = terminal to be switched to
Outputs: returns int32_t = 0 on success, -1 if newTerminal is out of bounds
*/
int32_t terminal_switch(int32_t newTerminal){
    // pcb_t * prev_pcb;
    if(newTerminal > TERMINAL_3_NUM || newTerminal < TERMINAL_1_NUM)
        return -1;      // CHECK

    if(newTerminal == currTerminal){ //dont switch if on the same
            return 0;
    }
    enterflag = 0;
    
    /* unmap current to itself */
    map_table((VIDEO_T1 + FOUR_KILO_BYTE * (currTerminal)) >> PAGE_SHIFT  , (VIDEO_T1 + FOUR_KILO_BYTE * (currTerminal))   );
    /* First copy vid mem to the actual terminal location */
    memcpy((uint8_t *)((VIDEO_T1 + FOUR_KILO_BYTE * (currTerminal)) ), (uint8_t *)(VIDEO), FOUR_KILO_BYTE);

    /* Copy from the current terminal's keyboard buffer into the stored buffer */

    memcpy((uint8_t *)(terminalArray[currTerminal].terminalbuffer), (uint8_t *)(keyboardbuffer), KEYBOARD_BUFFER_MAX_SIZE);
    /* Copy from the stored buffer of the new terminal into the current terminal's keyboard buffer */
    memcpy((uint8_t *)(keyboardbuffer), (uint8_t *)(terminalArray[newTerminal].terminalbuffer), KEYBOARD_BUFFER_MAX_SIZE);

    /* newTerminal should map to vid mem*/
    /* Then copy from the new terminal location into vid mem */
    memcpy((uint8_t *)(VIDEO), (uint8_t *)(VIDEO_T1 + FOUR_KILO_BYTE * (newTerminal))  , FOUR_KILO_BYTE);
    map_table( (VIDEO_T1 + FOUR_KILO_BYTE * (newTerminal)) >> PAGE_SHIFT  , VIDEO  ); //???
    /* Update the current terminal's cursor */
    update_cursor(terminalArray[newTerminal].cursor_x, terminalArray[newTerminal].cursor_y);
    set_screen_x(terminalArray[newTerminal].cursor_x);
    set_screen_y(terminalArray[newTerminal].cursor_y);
    update_cursor(terminalArray[newTerminal].cursor_x, terminalArray[newTerminal].cursor_y);
    currTerminal = newTerminal;
    
    return 0;
}

/*
int32_t terminal_init()
Description: Terminal's initialization function
Inputs: none
Outputs: none
*/
void terminal_init(){
    int i; 
    currpid = 0;
    /* Inintialize the values in the terminal struct for all 3 terminals. */
    for(i = 0; i<MAX_TERMINALS; i++){
        processesid[i] = 1;    // SAYS PROCESS IS ACTIVE
        terminalArray[i].currRTC = -1;
        terminalArray[i].cur_PCB = NULL;
        terminalArray[i].cursor_x = 0;
        terminalArray[i].cursor_y = 0;
        terminalArray[i].currprocessid = i;

    }
    terminalArray[TERMINAL_1_NUM].vidmemloc = (uint32_t)(VIDEO_T1);
    terminalArray[TERMINAL_2_NUM].vidmemloc = (uint32_t)(VIDEO_T2);
    terminalArray[TERMINAL_3_NUM].vidmemloc = (uint32_t)(VIDEO_T3);
    currpid = -1;
    currTerminal = 0;

}

