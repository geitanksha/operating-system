# pagingasm.S - ASM coding for paging - enabling paging and flushing.
# vim:ts=4 noexpandtab

# alloc the space for the directory and table
#enable the paging after we have inialized the page tables (bit 31 in CR0 reg)

# took from this: https://wiki.osdev.org/Paging
#    .align 4096
#page_directory:  
#    .rept 1024
#    .long 0    #init 1024 32-bit entireis in page directory  
#    .endr

#    .align 4096
#page_table:  
#    .rept 1024
#    .long 0    #init 1024 32-bit entireis in page table  
#    .endr

#    .align 4096
#page:  
#    .rept 1024
#    .long 0    #init a 1 kb page   
#   .endr

# set PS = 1 pde points directly to 4MB page for kernel 

/*
enPaging
Description: enables paging  
Inputs: none
Outputs: none
*/
.text
.globl enPaging
enPaging:
push %ebp
mov %esp, %ebp
mov %cr4, %eax
or  0x00000010, %eax #0x00000010 enables 4mb paging 
mov %eax, %cr4
mov %cr0, %eax
or $0x80000001, %eax # $0x80000000 sets the 32 bit for cr0 to turn on paging 
mov %eax, %cr0
mov %ebp, %esp
pop %ebp
ret



/*
loadPageDir
Description: takes the directory ptr and puts it to the cr3 
Inputs: takes page struct ptr 
Outputs: none
*/
.text
.globl loadPageDir
loadPageDir:
push %ebp
mov %esp, %ebp
mov 8(%esp), %eax
mov %eax, %cr3
mov %ebp, %esp
pop %ebp
ret


