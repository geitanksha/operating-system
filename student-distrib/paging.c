#include "paging.h"


page_dir_entry page_directory[NUM_ELEMS_PAGE] __attribute__((aligned(SIZE_OF_PG)));
page_table_entry page_table[NUM_ELEMS_PAGE] __attribute__((aligned(SIZE_OF_PG)));
page_table_entry page_table_user_vidmem[NUM_ELEMS_PAGE] __attribute__((aligned(SIZE_OF_PG))); 
void init_page();

// Referenced this: https://wiki.osdev.org/Paging
/*
page table base Addr 31->12: Upper 20 bits of the physical address of a 4K-aligned page table. In MP4, kernel
physical and virtual addresses are the same, so the page table base address should be just a label or variable
name from their code. All page tables must be 4K aligned, so the lower 12 bits of their addresses are 0 
*/

/*
void init_page()
Description: inits the page vid mem and kernel mem 
Inputs: none
Outputs: none
Side Effects: inits the page vid mem and kernel mem 
*/
// set PS = 1 for 1 MB page for the kernel
void init_page(){
    int i;
// set the page dirctory 
    for(i = 0; i<NUM_ELEMS_PAGE; i++){

        /* Set up the page directory. Refer to descriptors.pdf for why we set these bits. */
        page_directory[i].p = 0;
        page_directory[i].rw = 1;               // set to 1 (enable rw)
        page_directory[i].us = 0;               // set to kernel permission 
        page_directory[i].pwt = 0;  
        page_directory[i].pcd =  0;             //for vid mem     
        page_directory[i].a =  0;               // not used 
        page_directory[i].DC1 =  0;             //always set  
        page_directory[i].ps = 0;               // 4k pg table  
        page_directory[i].g =  0;               
        page_directory[i].avail = 0;            // dont use 
        page_directory[i].pt_baddr = 0;

        /* Set up the page directory. Refer to descriptors.pdf for why we set these bits. */
        page_table[i].p = 0;
        page_table[i].rw = 1;                   // set to 1 (enable rw)
        page_table[i].us = 0;
        page_table[i].pwt = 0;  
        page_table[i].pcd =  0;    
        page_table[i].a =  0;   
        page_table[i].d =  0;   
        page_table[i].pat = 0;                  // 4mb page table
        page_table[i].g =  0;                   
        page_table[i].avail = 0;   
        page_table[i].pt_baddr = i;             // cr3 - set in assebly. 

        
        page_table_user_vidmem[i].rw = 1;                   // set to 1 (enable rw)
        page_table_user_vidmem[i].us = 1;            // set us to 1 for user 
        page_table_user_vidmem[i].pwt = 0;  
        page_table_user_vidmem[i].pcd =  0;    
        page_table_user_vidmem[i].a =  0;   
        page_table_user_vidmem[i].d =  0;   
        page_table_user_vidmem[i].pat = 0;                  
        page_table_user_vidmem[i].g =  0;                   
        page_table_user_vidmem[i].avail = 0;   
        page_table_user_vidmem[i].pt_baddr = i;           
        page_table_user_vidmem[i].p = 0; 
    }


    
    /* Set up the 4MB kernel */
    page_directory[KERNEL_PDE_ENTRY].pt_baddr = (uint32_t)(KERNEL_ADDR) >> PAGE_SHIFT;     // x1 << 10 
    page_directory[KERNEL_PDE_ENTRY].g = 1;                    // 4mb page table  
    page_directory[KERNEL_PDE_ENTRY].ps = 1;                   // 4mb page table
    page_directory[KERNEL_PDE_ENTRY].pcd =  1; 
    page_directory[KERNEL_PDE_ENTRY].rw = 1;
    page_directory[KERNEL_PDE_ENTRY].p = 1;

    /* Setting up Video memory (actual) where physical and virtual are both B8. (0xB8000 – 0xC0000) descripters.pdf pg 5
    PD entry is 0, PT entry is B8
    */
    page_directory[VIDMEM_PDE_ENTRY].pt_baddr = (int)(page_table) >> PAGE_SHIFT;       // Shift << 12 since lower 12 bits 0 for alignment B8000 -> B8
    page_directory[VIDMEM_PDE_ENTRY].rw = 1;
    page_directory[VIDMEM_PDE_ENTRY].p = 1;
    page_directory[VIDMEM_PDE_ENTRY].us = 1;
    /* Setting Video Memory inside the page table */
    page_table[(VIDEO >> PAGE_SHIFT)].p = 1; 
    
    // /* Setting up Video memory for user where physical is B8 and virtual is 132 + B8. (0xB8000 – 0xC0000) descripters.pdf pg 5
    // PD entry is 0, PT entry is B8
    // */
    // page_directory[USERVIDMEM_PDE_ENTRY].pt_baddr = (int)(page_table_user_vidmem) >> PAGE_SHIFT;       // Shift << 12 since lower 12 bits 0 for alignment B8000 -> B8
    // page_directory[USERVIDMEM_PDE_ENTRY].rw = 1;
    // page_directory[USERVIDMEM_PDE_ENTRY].us = 1;
    // page_directory[USERVIDMEM_PDE_ENTRY].p = 1;

    // /* Setting Video Memory inside the page table */
    // page_table_user_vidmem[(VIDEO >> PAGE_SHIFT)].us = 1;                   // set us to 1 for user 
    // page_table_user_vidmem[(VIDEO >> PAGE_SHIFT)].p = 1;
    
    page_table[(VIDEO_T1 >> PAGE_SHIFT)].pt_baddr = VIDEO >> PAGE_SHIFT;
    page_table[(VIDEO_T1 >> PAGE_SHIFT)].p = 1;
    page_table[(VIDEO_T1 >> PAGE_SHIFT)].us= 1;
    page_table[(VIDEO_T2 >> PAGE_SHIFT)].p = 1;
    page_table[(VIDEO_T2 >> PAGE_SHIFT)].us = 1;
    page_table[(VIDEO_T3 >> PAGE_SHIFT)].p = 1;
    page_table[(VIDEO_T3 >> PAGE_SHIFT)].us = 1;
    loadPageDir(page_directory); 
    enPaging();

} //end of init 
