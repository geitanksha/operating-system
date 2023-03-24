
#include "x86_desc.h"
#include "IDT.h"
#include "linkageheader.h"
#include "filesys.h"
#include "rtc.h"


 #ifndef _SYSCALL_H
 #define _SYSCALL_H

#define PDE_PROCESS_START       2                   // 8 MB maps to second PDE
#define FOUR_MB                 0x400000            
#define PDE_VIRTUAL_MEM         32                  // 128 virt addr is 32th pd 
#define MAX_ARG_LEN             128               
#define VIRT_ADDR               0x8048000          
#define VIRT_ADDR_INSTRUC       24          
#define MAX_FD_LEN              8                 
#define BYTES_TO_COPY           4                 
#define FOUR_BYTE_OFFSET        4                 
#define MAGIC_0                 0x7f                  
#define MAGIC_1                 0x45                  
#define MAGIC_2                 0x4c                  
#define MAGIC_3                 0x46                  
#define PROG_START              0x8400000           // 132 mb             
#define MIN_FD_VAL_STD          2                  
#define ERRMSG                  33                  
#define MAX_NUM_PROCESSES       6                  
#define BYTEZERO                0
#define BYTEONE                 1
#define BYTETWO                 2
#define BYTETHREE               3
#define HALT_CODE               15
#define TYPE_FILE               2
#define TYPE_DIR                1
#define TYPE_RTC                0
#define BUFFER_SHIFT            8
#define VIDST_USER              0x8000000  
#define VIDEND_USER             0x8400000 

#define VID_START               0x084B8000 

                                // 0x8050d40

/* All calls return >= 0 on success or -1 on failure. */

/*  
 * Note that the system call for halt will have to make sure that only
 * the low byte of EBX (the status argument) is returned to the calling
 * task.  Negative returns from execute indicate that the desired program
 * could not be found.
 */ 
/* System Call Number: */
extern int32_t halt(uint8_t status);                               // 1
extern int32_t execute(const uint8_t* command);                    // 2
extern int32_t read(int32_t fd, void* buf, int32_t nybytes);       // 3
extern int32_t write(int32_t fd, void* buf, int32_t nbytes);       // 4
extern int32_t open(const uint8_t* filename);                      // 5
extern int32_t close(int32_t fd);                                  // 6
extern int32_t getargs(uint8_t* buf, int32_t nbytes);              // 7
extern int32_t vidmap(uint8_t** screen_start);                     // 8
extern int32_t set_handler(int32_t signum, void* handler_address); // 9
extern int32_t sigreturn(void);  


int32_t map_helper(uint32_t pdeentry, uint32_t pdeaddr);
int32_t map_table(uint32_t ptentry, uint32_t pteaddr);
int find_available_pid();

int32_t read_fail(const uint8_t *filename);
int32_t write_fail(int32_t fd);
int32_t open_fail(int32_t fd, void* buf, int32_t nbytes);
int32_t close_fail(int32_t fd, const void* buf, int32_t nbytes);

int32_t execute_on_term(const uint8_t *command, int term);

typedef struct func
{
    int32_t (*open)(const uint8_t *filename,int32_t fd);
    int32_t (*read)(int32_t fd, void *buf, int32_t nbytes);
    int32_t (*write)(int32_t fd, const void *buf, int32_t nbytes);
    int32_t (*close)(int32_t fd);

} __attribute__((packed)) func_t;

typedef struct file_desc
{
    func_t fileop;
    uint32_t inode;
    uint32_t filepos;
    uint8_t present;
    uint8_t type;
    uint8_t f2;
    uint8_t f3;


} __attribute__((packed)) fd_t;

/* PCB Struct */
typedef struct pcb_struct
{
    uint8_t pid;
    int8_t parent_id;
    uint32_t saved_esp;
    uint32_t saved_ebp;
    fd_t fdarray[MAX_FD_LEN];
    int8_t active;
    uint8_t argbuffer[MAX_ARG_LEN];
    uint8_t termid;

} __attribute__((packed)) pcb_t;

//pcb_t *globalpcb;
uint8_t processesid[MAX_NUM_PROCESSES];

/* Terminal struct */
typedef struct terminal_struct
{
    
    int8_t active;
    uint8_t keyboard_buffer[MAX_ARG_LEN];

} __attribute__((packed)) terminal_t;
int currpid;
int baseShellFlag;

#endif /* _SYSCALL_H */
