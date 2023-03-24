/* filesys.h - Defines used in interactions with the file system.
 */

 #ifndef _FILESYS_H
 #define _FILESYS_H

#include "multiboot.h"
#include "lib.h"
#include "terminal.h"

#include "syscall.h"

// #include "syscall.h"

/* Variables used in the program: */
#define RESERVED_BITS_DENTRY        6
#define RESERVED_BITS_BOOTBL        13
#define DIR_ENTRIES                 63
#define FOUR_KILO_BYTE              0x1000 //        4096
#define EIGHT_MEGA_BYTE             0x800000
#define EIGHT_KILO_BYTE             0x2000
#define MAX_FILENAME_LEN            32
#define MAX_FD_VAL                  7
#define MIN_FD_VAL                  0
#define START_FD_VAL                2
#define MAX_NUM_FILES               17
#define INODEDBLOCKS                1023

/* Dentry Struct */
typedef struct f1_dir
{
    int8_t filename[MAX_FILENAME_LEN];
    uint32_t ftype;
    uint32_t inode;
    uint32_t reserved[RESERVED_BITS_DENTRY];

} __attribute__((packed)) dentry_t;

/* Boot Block Struct */
typedef struct f2_dir
{
    uint32_t num_of_dirs;
    uint32_t num_of_inodes;
    uint32_t num_of_dblocks;
    uint32_t reserved[RESERVED_BITS_BOOTBL];
    // 16 entries for d entries
    dentry_t dirEntries[DIR_ENTRIES]; // index 0,1 reserved

} __attribute__((packed)) boot_block_t;

/* Inode Struct */
typedef struct f3_dir
{
    uint32_t length;
    uint32_t data_block[1023];

} __attribute__((packed)) inode_t;



/* Data Struct */
typedef struct f4_dir
{
    uint8_t data[FOUR_KILO_BYTE];

} __attribute__((packed)) dataBlock_t;




// 4 structs

/* Pointers needed to initialie our structs */
uint32_t fstart_adddr;
boot_block_t* bootblockptr;
dentry_t* currdentryptr;
inode_t* inodeptr;
dataBlock_t* datablockptr;


int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);


/* Functions wrt main reads/writes */
int32_t file_init(uint32_t startAddr);
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

/* Functions for file driver - open / close / r / w */
int32_t open_file(const uint8_t *filename,int32_t fd);
int32_t close_file(int32_t fd);
int32_t read_file(int32_t fd, void* buf, int32_t nbytes);
int32_t write_file(int32_t fd, const void* buf, int32_t nbytes);

/* Functions for directory driver - open / close / r / w */
int32_t open_dir(const uint8_t *filename,int32_t fd);
int32_t close_dir(int32_t fd);
int32_t read_dir(int32_t fd, void* buf, int32_t nbytes);
int32_t write_dir(int32_t fd, const void* buf, int32_t nbytes);

#endif /* _FILESYS_H */
