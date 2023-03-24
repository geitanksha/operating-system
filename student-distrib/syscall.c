#include "syscall.h"
#include "x86_desc.h"
#include "IDT.h"
#include "lib.h"
#include "linkageheader.h"
#include "filesys.h"
#include "paging.h"
//#include "scheduler.h"
#include "terminal.h"

/* Variables for the different system calls */



// extern int8_t term_2_flag;
// extern int8_t term_3_flag;
extern int terminalrun;
extern page_dir_entry page_directory[NUM_ELEMS_PAGE] __attribute__((aligned(SIZE_OF_PG)));
extern page_table_entry page_table[NUM_ELEMS_PAGE] __attribute__((aligned(SIZE_OF_PG)));
extern page_table_entry page_table_user_vidmem[NUM_ELEMS_PAGE] __attribute__((aligned(SIZE_OF_PG)));

/* System Call Functions */
int find_available_pid(){
    int i;
    for(i = MAX_TERMINALS; i<MAX_NUM_PROCESSES; i++){
        if(processesid[i] == 0){
            processesid[i] = 1;
            return i; //found empty id
        }
    }
    return -1; // at max pid's 
}
/*
int execute(const uint8_t* command)
Inputs:         command - gives the command + args from the terminal
Outputs:        integer - whether it happened successfully (0) or not (-1).
Description:    It helps us execute the functions on request by the user by calling execute_on_term. 

*/

int32_t execute(const uint8_t *command)
{
    currpid = 3; //after init 0->2 always pid 3 for max processes check 
    sti();
    while(1){
        if(currTerminal == terminalrun)
        return execute_on_term(command, currTerminal);
    }
    return 0;
}

/*
int execute_on_term(const uint8_t* command)
Inputs:         command - gives the command + args from the terminal
Outputs:        integer - whether it happened successfully (0) or not (-1).
Description:    It helps us execute the functions on request by the user.

Things done in execute:

1. Extract name and args - check whether executable
2. Search file system for the name of the file given                // read dentry by name
3. Extract all information about the file - if it's an executable   // read data
4. Paging: Use PID to decide 8 + (PID*4MB)
            Then copy the whole file into that virtual address      // read data

5. Set up PCB stuff too - create new PCB
            Set up the FD (1, 2 for terminal, rest empty)\

6. Set up context switch ( kernel stack base into esp of tss )
7. IRET

*/
int32_t execute_on_term(const uint8_t *command, int term)
{
    /*  1. Extract name and args - check whether executable  */

    int cmd_len = strlen((const int8_t *)(command));
    uint8_t filename[MAX_FILENAME_LEN];
    uint8_t finalarg[MAX_ARG_LEN];
    uint8_t finalarg_trimmed[MAX_ARG_LEN];
    uint8_t buffer[4];
    unsigned int i = 0;
    int filechar = 0;
    int finalchar = 0;


    /* Initialize the name, args as NULL */
    for (i = 0; i < MAX_FILENAME_LEN; i++) {
        filename[i] = '\0';
    }
    
    for (i = 0; i < MAX_ARG_LEN; i++) {
        finalarg[i] = '\0';
    }

    for (i = 0; i < MAX_ARG_LEN; i++) {
        finalarg_trimmed[i] = '\0';
    }
    
    i = 0;

    int start = 0;
    /* Parse the args / command name */
    for (; i < cmd_len; i++) {
        //printf("%d ",i);
        if (start == 0) {
            if ((command[i] == ' ') || (command[i] == '\t'))
                continue;
            start = 1;
            if(i <MAX_FILENAME_LEN)
            filename[filechar++] = command[i];
        } 
        else {
            if ((command[i] == ' ') || (command[i] == '\t'))
                break;
            if(i<MAX_FILENAME_LEN)
            filename[filechar++] = command[i];
        }
    } 
    start = 0;
    for (; i < cmd_len; i++) {
        if (start == 0) {
            if ((command[i] == ' ') || (command[i] == '\t'))
                continue;
            start = 1;
            finalarg[finalchar++] = command[i];
        } 
        else {
            finalarg[finalchar++] = command[i];
        }
    }
    int strlencmd = strlen((int8_t *)finalarg);
    /* This trims the command that was extracted. */
    int trimval = 0;
    for (i=0; i < strlencmd; i++) {
        if (!((finalarg[i] == ' ') || (finalarg[i] == '\t'))) 
            finalarg_trimmed[trimval++] = finalarg[i];
        else
            break;
    }

    /* 2. Search file system for the name of the file given: store it in the currdentry. */
    dentry_t currdentry;
    
    if (strlen((int8_t *)(filename)) == 0 || read_dentry_by_name(filename, (dentry_t*)(&currdentry)) < 0 )
        return -1;                              // Invalid filename / could not find file


    /* 3. The first 4 bytes of the file represent a “magic number” that identifies the file as an executable. These
    bytes are, respectively, 0: 0x7f; 1: 0x45; 2: 0x4c; 3: 0x46. If the magic number is not present, the execute system
    call should fail.
    */

    int exec = 0; // Check if it is an executable
    
    /* Store the 4 magic numbers into the buffer to be checked */
    if (read_data(currdentry.inode, 0, (uint8_t *)(buffer), BYTES_TO_COPY) < 0 )
        return -1;                              // Could not successfully extract the numbers
    
    /* Check the 4 bytes 0,1,2,3 in the buffer about whether those are magic numbers */
    if (buffer[0] == MAGIC_0 && buffer[1] == MAGIC_1 && buffer[2] == MAGIC_2 && buffer[3] == MAGIC_3) // magic numbers for executables
        exec = 1;
    if (!exec) 
        return -1;                              // it is not executable


    /* Checks if we have max number of processes: */
    if ((currpid >= 3)&&(!baseShellFlag)) {    // using 3 since the other pids(0->2) for base are preset 
        int rval_pid = find_available_pid();
        if (rval_pid < 0) {
            puts2("Too many processes called! (>6)\n", ERRMSG);
            return 0;
        }
        currpid = rval_pid;
        terminalArray[term].currprocessid = rval_pid;
    }
    else if(baseShellFlag){
        currpid = term;
        terminalArray[term].currprocessid = term;
    }
    baseShellFlag = 0;

    cli();

    /* 4. Paging: Use PID to decide 8 + (PID*4MB)
          Then copy the whole file into that virtual address 128MB location.
    */
    

    uint32_t physaddr = (PDE_PROCESS_START + currpid) * FOUR_MB;
    map_helper(PDE_VIRTUAL_MEM, physaddr);

    /* Now we start copying into 4MB User pages */
    uint8_t *addrptr = (uint8_t *)(VIRT_ADDR); // PASSED INTO READ DATA AS BUFFER
    uint32_t currdentryinodenum = currdentry.inode;
    uint32_t currdentryinodelen = ((inode_t *)(inodeptr + currdentryinodenum))->length;
    
    /* Uses read_data to copy information into the user page. */
    int rval = read_data(currdentryinodenum, 0, addrptr, currdentryinodelen);
    
    if (  rval< 0 )
        return -1;

    /* 5. Create new PCB for the current newly created process. */
    
    int curraddr = EIGHT_MEGA_BYTE - (EIGHT_KILO_BYTE * (currpid + 1));
    pcb_t * currpcb; 
    pcb_t * parentpcb;
    currpcb =  (pcb_t *)(curraddr);

    parentpcb = terminalArray[term].cur_PCB;

    terminalArray[term].cur_PCB = currpcb;

    currpcb->pid = currpid;
    if ((currpid == 0 || currpid == 1 || currpid == 2 )) // checking 0, 1, 2 since those pids are for base shell
        currpcb-> parent_id = -1; // check what parent of shell should be 
    else
        currpcb-> parent_id = parentpcb->pid;

   currpcb->termid = term;
    uint32_t save_esp = 0;
    uint32_t save_ebp = 0;

    asm volatile
    (
        "movl %%ebp, %0; \n"
        "movl %%esp, %1; \n"
        :"=g"(save_ebp), "=g"(save_esp)
        :
    );

    currpcb->saved_esp = save_esp;
    currpcb->saved_ebp = save_ebp;

    currpcb->termid = term;
    
    /*  Set up the FD (1, 2 for terminal, rest empty) */
    for(i = 0; i<MAX_FD_LEN; i++) {

        (currpcb->fdarray[i]).fileop.open = 0;
        (currpcb->fdarray[i]).fileop.read = 0;
        (currpcb->fdarray[i]).fileop.write = 0;
        (currpcb->fdarray[i]).fileop.close = 0;

        (currpcb->fdarray[i]).inode = -1;
        (currpcb->fdarray[i]).filepos = 0;
        (currpcb->fdarray[i]).present = 0;
        (currpcb->fdarray[i]).type = -1;
        //f2,f3 reserved (not used for now)
        (currpcb->fdarray[i]).f2 = -1;
        (currpcb->fdarray[i]).f3 = -1;
    }
    currpcb->active = 1;

    // std in
    (currpcb->fdarray[0]).fileop.open = terminal_open;
    (currpcb->fdarray[0]).fileop.read = terminal_read; 
    (currpcb->fdarray[0]).fileop.write = NULL;
    (currpcb->fdarray[0]).fileop.close = terminal_close; 
    (currpcb->fdarray[0]).present = 1;

    // std out
    (currpcb->fdarray[1]).fileop.open = terminal_open;
    (currpcb->fdarray[1]).fileop.read = NULL;
    (currpcb->fdarray[1]).fileop.write = terminal_write;
    (currpcb->fdarray[1]).fileop.close = terminal_close;
    (currpcb->fdarray[1]).present = 1;
    
    strcpy((int8_t *)(currpcb->argbuffer), (int8_t *)(finalarg_trimmed));

    /* 6. Set up context switch ( kernel stack base into esp of tss ) */
 
    uint32_t currksp = (uint32_t)(EIGHT_MEGA_BYTE - (EIGHT_KILO_BYTE * currpid) - FOUR_BYTE_OFFSET);
    tss.ss0 = KERNEL_DS;
    tss.esp0 = currksp;

    /* 
     The other important bit of information that you need to execute programs is the entry point into the
    program, i.e., the virtual address of the first instruction that should be executed. This information is stored as a 4-byte
    unsigned integer in bytes 24-27 of the executable, and the value of it falls somewhere near 0x08048000 for all programs we have provided to you. When processing the execute system call, your code should make a note of the entry
    point, and then copy the entire file to memory starting at virtual address 0x08048000. It then must jump to the entry
    point of the program to begin execution.
    */
    if (read_data(currdentry.inode, VIRT_ADDR_INSTRUC, buffer, BYTES_TO_COPY) < 0 )
        return -1;              // could not read those 4 bytes.
    uint32_t stack_eip = 0; // buffer[3],buffer[2],buffer[1],buffer[0]
    /* Reversing 4 bytes of the buffer (bytes 24-27 of the file) due to little endianness*/
    stack_eip = (stack_eip | buffer[3]); // acessing from top buffer bytes to bottom 3->0
    stack_eip = stack_eip << BUFFER_SHIFT;
    stack_eip = (stack_eip | buffer[2]);
    stack_eip = stack_eip << BUFFER_SHIFT;
    stack_eip = (stack_eip | buffer[1]);
    stack_eip = stack_eip << BUFFER_SHIFT;
    stack_eip = (stack_eip | buffer[0]);


    uint32_t stack_esp = PROG_START - FOUR_BYTE_OFFSET;
    int usrDS = USER_DS;
    int usrCS = USER_CS;

    /* Start doing IRET */    
    /* 
    Stuff to push to stack (to convert to usermode) in order:
    USER_DS
    USER_ESP
    FLAGS | 200
    USER_CS
    PROG_EIP
    The 4 bytes we just extracted was the EIP - we need to save that because it is what we push in IRET.
    The ESP is basically the 132 MB location - 4 Bytes so we do not exceed the virtual stack location.
    */
   /* Oring to enable interrupts - 0x0200 with the flags, currently stored in eax */

    asm volatile("pushl %0;"
        "pushl %1; \n"
        "pushfl; \n"
        "popl %%eax; \n"
        "orl $0x0200, %%eax; \n"
        "pushl %%eax; \n"
        "pushl %2; \n"
        "pushl %3; \n"
        "iret; \n"
        :                            
        : "g"(usrDS), "g"(stack_esp), "g"(usrCS), "g"(stack_eip)
        :"%eax", "memory", "cc"
        );
        
    return 0;
}


/*
int halt(uint8_t status)

Inputs:             status - tells us which program called halt.
Outputs:            whether the halt call was successful (0) or not (-1)
Description:        The halt system call terminates a process, returning the specified value to its parent process. The system call handler
                    itself is responsible for expanding the 8-bit argument from BL into the 32-bit return value to the parent program’s
                    execute system call. Be careful not to return all 32 bits from EBX. This call should never return to the caller.
    
    1. Restore Parent data (stored in the PCB)
    2. Restore Parent paging
    3. Close the relevant FDs
    4. Jump to the execute's return

    1. Setup return value (check if exception // check if program is finished)
    2. close all processes
    3. set currently active process to non active
    4. check if it is the main shell (restart if yes)
    5. not main shell handler
    6. halt return (assembly)

*/
int32_t halt(uint8_t status)
{
   cli();
/* 1. Set up return value:
    - expand the 8-bit arg from BL into the 32-bit return value to the parent program
    - 8-bit 'status' arguement = the (one of 256) interupt handler that called the 'halt' */

    uint32_t status_ret_val = 0x0000;

    if (status == HALT_CODE) {
        status_ret_val = EXCEPTION_CALLED;
    }

    int parent_pcbaddr; 
   
    pcb_t* currpcb = terminalArray[terminalrun].cur_PCB;  //(pcb_t *)globalpcb;
 
    pcb_t* parentpcb;

    /* 2. Close all processes
    - Close all processes except the parent execute system call 
    - after 2.a, all currpcb == globalpcb AND all parentpcb == globalpcb - 1 */
    
    /* (a) check if the current process's parent is the shell program*/
    if (currpcb->parent_id == -1){
        baseShellFlag = 1;
        execute((const uint8_t *)"shell");
    }
    
    parent_pcbaddr = EIGHT_MEGA_BYTE - (EIGHT_KILO_BYTE * (currpcb->parent_id+1));
    parentpcb = (pcb_t *)(parent_pcbaddr); 

    /* 3. set the current pcb->active bit to 0 (non active)*/
    currpcb->active = 0;

    //set process id slot to avaliable 
    processesid[currpcb->pid] = 0;
    

    /* Close all things in fd table of the currpcb (or globalpcb)*/
    int i; int close_result;
    for(i=MIN_FD_VAL_STD; i < MAX_FD_LEN; i++){
        if(currpcb->fdarray[i].present == 1) {
            close_result = close(i);
            if (close_result < 0)
                return -1; // change!! error check
        }
    }


    /* 5. not main shell handler
    - Get parent process
    - Set the TSS for parent
    - Unmap pages for current process
    - Map pages for parent process
    - Set parent's process as active
    - Call halt return (assembly)
    */

    /* (a) Get parent Process */
    /* parentpcb was set at the begining: Step 2*/
    
    /* (c) Unmap pages for current process */
    /* currpid = static int global variable */
   
    page_directory[PDE_VIRTUAL_MEM].p = 0;
    currpid = currpcb->parent_id;

    /* (d) Map pages for parent process */
    /* currpid was decremented, now currpid is set to the parent (currpid - 1)*/
    int parentpid = currpid;
    uint32_t parent_physaddr = (PDE_PROCESS_START + parentpid) * FOUR_MB;
    map_helper(PDE_VIRTUAL_MEM, parent_physaddr);

    /* (e) Set Parents Process as active */
    parentpcb->active = 1;

    /* (b) Set TSS for parent. ksp = kernel stack pointer */
    uint32_t args_esp = currpcb->saved_esp;
    uint32_t args_ebp = currpcb->saved_ebp;
    uint32_t parentksp = (uint32_t)(EIGHT_MEGA_BYTE - (EIGHT_KILO_BYTE * (parentpid) ) - FOUR_BYTE_OFFSET); //change curpid to parent 
    tss.ss0 = KERNEL_DS;
    tss.esp0 = parentksp;
    terminalArray[terminalrun].cur_PCB = parentpcb;

    /* 6. halt return (assembly)
    take in esp, ebp, retval
    set esp, ebp as esp ebp args
    set eax regs as ret val
    */
   sti();
    asm volatile
    (

        /* set esp, ebp as esp ebp args */
        "   movl %0, %%esp \n"
        "   movl %1, %%ebp \n"
        /* set eax regs as ret val */
        "   movl %2, %%eax \n"
        "   leave;          \n"
        "   ret;            \n"
        :
        : "r"(args_esp), "r"(args_ebp), "r"(status_ret_val)             // input
        : "cc"
    );

    return 0;
}

/*
int read(int32_t fd, void* buf, int32_t nybytes)
Inputs:             fd: the file descriptor
                    buf: where we need to copy into
                    nbytes: number of bytes to be copied
Outputs:            whether call was successful (0) or not (-1)
Description:        reads data from keyboard , file, device(RTC), or directory
*/
int32_t read(int32_t fd, void *buf, int32_t nbytes)
{
    sti();
    pcb_t * currpcb = terminalArray[terminalrun].cur_PCB;

    /* sanity check: initial file position at eof or beyonf end of curr file */
    if(buf == NULL || fd > MAX_FD_VAL || fd < MIN_FD_VAL || nbytes < 0) {return -1;}

    /* use a jmptable referenced by the tasks files array...
    which calls a generic handler for the specific file type's specific read function */
    int rval = 0;
    int rval2 = 0;
    if(fd == 1) return -1;                                  // This is terminal_write

    if((fd == 0)){
        rval2 = terminal_read(fd, buf, nbytes);
        return rval2;
    }

    if(currpcb->fdarray[fd].present == 0) 
    return -1;                                              // FD is absent 


    rval = ((currpcb->fdarray[fd]).fileop.read)(fd, buf, nbytes);
    if (rval < 0)
        return -1;

    return rval;
}


/*
int write(int32_t fd, void* buf, int32_t nbytes)
Inputs:             fd: the file descriptor
                    buf: stuff to copy
                    nbytes: number of bytes to be copied
Outputs:            whether call was successful (0) or not (-1)
Description:        writes data to either the terminal or to a device (RTC), depending on the fd
*/
int32_t write(int32_t fd, void *buf, int32_t nbytes)
{
    pcb_t * currpcb = terminalArray[terminalrun].cur_PCB;
    /* sanity check: if device(RTC), else we write to terminal */
    if(buf == NULL || fd > MAX_FD_VAL || fd < MIN_FD_VAL || nbytes < 0) 
        return -1;

    /* if RTC: syscall should always accept a 4-byte int specifyinng the interrupt rate in Hz (should set the rate of periodic interuppts accordingly) */
    if(fd == 0) 
        return -1;                                          // This is terminal_read

    if((fd == 1))
        return terminal_write(fd, buf, nbytes);

    if(currpcb->fdarray[fd].present == 0) 
        return -1;                                          // FD is absent

    int rval = ((currpcb->fdarray[fd]).fileop.write)(fd, buf, nbytes); // not sure how to call function
    if (rval < 0)
        return -1;
    
    return nbytes;
}


/*
int open(const uint8_t* filename)

Inputs:                 filename: the file to be opened
Outputs:                whether call was successful (0) or not (-1)
Description:            Opens the respective file (rtc, terminal, file or directory)
*/
int32_t open(const uint8_t *filename)
{
    int i;
    /* find the dir entry corresponding to the named file */
    if(filename == NULL) return -1;
    pcb_t * currpcb = terminalArray[terminalrun].cur_PCB;
    int dentry_name_return;
    dentry_t currdentry;


    dentry_name_return = read_dentry_by_name(filename, (dentry_t *)(&currdentry));
    if(dentry_name_return == -1) 
    return -1;                                  // check if file exists

    /* Traverses through the file descriptor array for the  free fd entry. */
    int fd = -1;
     for(i = START_FD_VAL; i<MAX_FD_LEN; i++){
        if(!(currpcb->fdarray[i]).present){
            fd = i;
            break;
        } 
    }

    if(fd == -1) 
    return -1;                                  // fd array is full, failed to open file

    /* allocate an unused file descriptor iff filename is not already present */
    /* RTC */
    if(currdentry.ftype == TYPE_RTC){
        (currpcb->fdarray[fd]).fileop.open = open_rtc;
        (currpcb->fdarray[fd]).fileop.read = read_rtc;
        (currpcb->fdarray[fd]).fileop.write = write_rtc;
        (currpcb->fdarray[fd]).fileop.close = close_rtc;
        (currpcb->fdarray[fd]).filepos = -1;
    }
    /* DIRECTORY */
    else if(currdentry.ftype == TYPE_DIR){
        (currpcb->fdarray[fd]).fileop.open = open_dir;
        (currpcb->fdarray[fd]).fileop.read = read_dir;
        (currpcb->fdarray[fd]).fileop.write = write_dir;
        (currpcb->fdarray[fd]).fileop.close = close_dir;
        (currpcb->fdarray[fd]).filepos = 0;
    }
    /* NORMAL FILE */
    else if(currdentry.ftype == TYPE_FILE){
        (currpcb->fdarray[fd]).fileop.open = open_file;
        (currpcb->fdarray[fd]).fileop.read = read_file;
        (currpcb->fdarray[fd]).fileop.write = write_file;
        (currpcb->fdarray[fd]).fileop.close = close_file;
        (currpcb->fdarray[fd]).filepos = 0;
    }
    (currpcb->fdarray[fd]).inode = currdentry.inode;
    (currpcb->fdarray[fd]).present = 1;
    (currpcb->fdarray[fd]).type = currdentry.ftype;
    
    int rval =  (currpcb->fdarray[fd]).fileop.open(filename,fd);
    
    /* if named file does not exist OR if no descriptor are free, then return -1 */
    if (rval < 0)
        return -1;

    return fd;
}

/*
int close(int32_t fd)
Inputs:                 fd: the file to be closed
Outputs:                whether call was successful (0) or not (-1)
Description:            Closes the respective file (rtc, terminal, file or directory)
*/
int32_t close(int32_t fd)
{
    pcb_t * currpcb = terminalArray[terminalrun].cur_PCB;

    if((fd < MIN_FD_VAL_STD) || (fd > MAX_FD_VAL)) 
    return -1;                                          // cannot be stdin or stdout

    if ((currpcb->fdarray[fd]).present == 0)
        return -1;

    int rval = ((currpcb->fdarray[fd]).fileop.close)(fd); // try closing the file
    if (rval < 0)
        return -1;  
        
                                                // could not close the file
        
    // Otherwise you can safely close it.
    (currpcb->fdarray[fd]).fileop.open = 0;
    (currpcb->fdarray[fd]).fileop.read = 0;
    (currpcb->fdarray[fd]).fileop.write = 0;
    (currpcb->fdarray[fd]).fileop.close = 0;

    (currpcb->fdarray[fd]).inode = 0;
    (currpcb->fdarray[fd]).filepos = 0;
    (currpcb->fdarray[fd]).present = 0;
    (currpcb->fdarray[fd]).type = -1;

    //f2,f3 reserved (not used for now)
    (currpcb->fdarray[fd]).f2 = -1;
    (currpcb->fdarray[fd]).f3 = -1;

    return 0;
}
/*
int getargs(uint8_t* buf, int32_t nbytes)
Inputs:                 buf: buffer where the args will get stored
                        nbytes: size of the buffer to copy into
Outputs:                whether call was successful (0) or not (-1)
Description:            Puts the arguments from the PCB into the buffer
*/
int32_t getargs(uint8_t *buf, int32_t nbytes)
{ 
    pcb_t * currpcb = terminalArray[terminalrun].cur_PCB;
    if((buf == NULL) || (strlen((int8_t *)currpcb->argbuffer) == 0) || (strlen((int8_t *)currpcb->argbuffer) + 1 > nbytes)) 
        return -1;                      // Added based off "if the arguments and a terminal NULL (0-byte) do not fit in the buffer"

    strncpy((int8_t *)buf, (int8_t *)(currpcb->argbuffer), nbytes);

    return 0;
}

// Vidmap: System Call Number 8
/*
int vidmap(uint8_t** screen_start)
Inputs:                 screen_start: the double pointer from the user side that needs to be set to the start of the vidmem section.
Outputs:                whether call was successful (0) or not (-1)
Description:            It moves the user's pointer to the newly created page that will help the user to write directly into vidmap.
*/
int32_t vidmap(uint8_t **screen_start)
{
    /* We are given a double pointer - we need to check validity.*/
    pcb_t * currpcb = terminalArray[terminalrun].cur_PCB;
    if (screen_start == NULL)
        return -1;
    //  screen_start from test: 0x8050d40
    if ((int)screen_start < (int)VIDST_USER || (int)screen_start > (int)VIDEND_USER)
        return -1;

    /* Perform map of 132 + b8 */

    *screen_start = (uint8_t *)terminalArray[currpcb->termid].vidmemloc; //(uint8_t *)VID_START;

    return (int32_t)(*screen_start);
}

// Set_handler: System Call Number 9
/*
int set_handler(int32_t signum, void* handler_address)
Description:
Inputs:
Outputs:
*/
int32_t set_handler(int32_t signum, void *handler_address)
{
    return -1;
}

// Sigreturn: System Call Number 10
/*
int sigreturn(void)
Description:
Inputs:
Outputs:
*/
int32_t sigreturn(void)
{
    return -1;
}

/*-------------fail helper funcs--------------*/
// read_fail
/*
Description:
Inputs:
Outputs: returns -1
*/
int32_t read_fail(const uint8_t *filename){
    return -1;
}
// Sigreturn: System Call Number 10
/*
int sigreturn(void)
Description:
Inputs:
Outputs:
*/
int32_t write_fail(int32_t fd){
    return -1;
}
// Sigreturn: System Call Number 10
/*
int sigreturn(void)
Description:
Inputs:
Outputs:
*/ 
int32_t open_fail(int32_t fd, void* buf, int32_t nbytes){
    return -1;
}
// Sigreturn: System Call Number 10
/*
int sigreturn(void)
Description:
Inputs:
Outputs:
*/
int32_t close_fail(int32_t fd, const void* buf, int32_t nbytes){
    return -1;
}  

/*
int32_t map_helper(uint32_t pdeentry, uint32_t pdeaddr)
Inputs:             pdeentry: the entry of the PDE that needs to be changed.
                    pdeaddr: the address of the PDE to be changed depending on the process number
Outputs:            0 if successful
Description:        Maps the user page to the right location
*/
int32_t map_helper(uint32_t pdeentry, uint32_t pdeaddr) {
    page_directory[pdeentry].ps = 1;             // make it a 4 mb page
    page_directory[pdeentry].pt_baddr = pdeaddr >> PAGE_SHIFT;
    page_directory[pdeentry].pcd = 1;            // in desc.pdf
    page_directory[pdeentry].us = 1;             // must be 1 for all user-level pages and mem ranges
    page_directory[pdeentry].p = 1;              // the page is present.
    /* Flush the TLB */
    loadPageDir(page_directory); // flush TLB //? check with os dev maybe other stuff for flushing
    return 0;
}

/*
int32_t map_table(uint32_t pdeentry, uint32_t pdeaddr)
Inputs:             ptentry: the entry of the PT that needs to be changed.
                    pdeaddr: the address of the PT to be changed depending on the process number
Outputs:            0 if successful
Description:        Maps the user page to the right location
*/
int32_t map_table(uint32_t ptentry, uint32_t pteaddr) {
    page_table[ptentry].pt_baddr = pteaddr >> PAGE_SHIFT; 
    page_table[ptentry].p = 1;
    /* Flush the TLB */
    loadPageDir(page_directory); // flush TLB 
    return 0;
}




/*
int32_t unmap_helper(uint32_t pdeentry, uint32_t pdeaddr)
Inputs:             pdeentry: the entry of the PDE that needs to be changed.
                    pdeaddr: the address of the PDE to be chan
Outputs:            
Description:
*/
int32_t unmap_helper(uint32_t pdeentry, uint32_t pdeaddr) {
    page_directory[pdeentry].ps = 1;             // make it a 4 mb page
    page_directory[pdeentry].pt_baddr = pdeaddr >> PAGE_SHIFT;
    page_directory[pdeentry].pcd = 1;            // in desc.pdf
    page_directory[pdeentry].us = 1;             // must be 1 for all user-level pages and mem ranges
    page_directory[pdeentry].p = 1;              // should we set p = 0 ??
    /* Flush the TLB */
    loadPageDir(page_directory); // flush TLB //? check with os dev maybe other stuff for flushing
    return 0;
}





