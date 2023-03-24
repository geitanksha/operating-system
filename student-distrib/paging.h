/* paging.h - Defines used in interactions with the Paging setup.
 * vim:ts=4 noexpandtab
 */

#ifndef _PAGING_H
#define _PAGING_H

#include "x86_desc.h"
#include "lib.h"

#define SIZE_OF_PG              4096
#define VIDEO                   0xB8000
#define VIDEO_END               0xB9000
#define VIDEO_T1                0xBA000
#define VIDEO_T2                0xBB000
#define VIDEO_T3                0xBC000
#define KERNEL_ADDR             0x00400000
#define NUM_ELEMS_PAGE          1024
#define PAGE_SHIFT              12
#define KERNEL_PDE_ENTRY        1
#define VIDMEM_PDE_ENTRY        0
#define USERVIDMEM_PDE_ENTRY    33

/* Struct for the Page Directory. Refer to Descriptors.pdf. */
typedef struct p_d1{
        //uint32_t val[2];
    uint8_t p   : 1;
    uint8_t rw  : 1;                // set to 1
    uint8_t us  : 1;  
    uint8_t pwt : 1;
    uint8_t pcd : 1;
    uint8_t a : 1;
    uint8_t DC1 : 1;
    uint8_t ps : 1;
    uint8_t g : 1;
    uint8_t avail : 3;              // 3 is num of avaiable bits 
    uint32_t pt_baddr : 20;         // 20 for page address bits 
}__attribute__ ((packed)) page_dir_entry;


 /* Struct for the Page Table. Refer to Descriptors.pdf. */
typedef struct p_t
{
     // uint32_t val[2];
    uint8_t p : 1;
    uint8_t rw : 1;
    uint8_t us : 1;
    uint8_t pwt : 1;
    uint8_t pcd : 1;
    uint8_t a : 1;
    uint8_t d : 1;
    uint8_t pat : 1;
    uint8_t g : 1;
    uint8_t avail : 3;             // 3 is num of avaiable bits
    uint32_t pt_baddr : 20;        // 20 for page address bits
} __attribute__((packed)) page_table_entry;


// typedef struct p_d2{
//     uint32_t p   : 1;
//     uint32_t rw  : 1;
//     uint32_t us  : 1;  
//     uint32_t pwt : 1;
//     uint32_t pcd : 1
//     uint32_t a : 1;
//     uint32_t DC1 : 1;
//     uint32_t ps : 1;
//     uint32_t g : 1;
//     uint32_t avail : 3;
//     uint32_t pat : 1;
//     uint32_t reserv1 : 9;
//     uint32_t pt_baddr : 10;
// }__attribute__ ((packed)) page_dir_entry_4mb;

/* Function Declarations to use the page. */

void init_page();
extern void loadPageDir(page_dir_entry* page_directory);
extern void enPaging();

#endif 
