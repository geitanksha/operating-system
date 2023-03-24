#include "x86_desc.h"
#include "IDT.h"
#include "lib.h"
#include "linkageheader.h"
#include "filesys.h"
#include "syscall.h"

// used code from here https://wiki.osdev.org/Interrupt_Descriptor_Table

/* Note for all interrupt functions:
 * Each of these just define the exceptions and interrupts as defined in the Interrupt Descriptor Table.
 */

/*
int generic_interrupt()
Description:  Generic interrupt for what was not described in the IDT but was still called.
Inputs: none
Outputs: int (0 if valid) - never reached
*/
int generic_interrupt()
{
    printf("Generic non-descript interrupt");
    halt(HALT_CODE);
    return 0;
}

/*
int divide_error()
Description:  Interrupt generated for Divide by 0.
Inputs: none
Outputs: int (0 if valid) - never reached
*/
int divide_error()
{
    printf("Divide by zero error");
    halt(HALT_CODE);
    return 0;
}

/*
int RESERVED()
Description:  Interrupt generated for sections reserved by intel
Inputs: none
Outputs: int (0 if valid) - never reached
*/
int RESERVED()
{
    printf("Reserved for Intel");
    halt(HALT_CODE);
    return 0;
}

/*
int NMI()
Description:  Interrupt generated for Non maskable interrupts sent in
Inputs: none
Outputs: int (0 if valid) - never reached
*/
int NMI()
{
    printf("Nonmaskable external interrupt");
    halt(HALT_CODE);
    return 0;
}

/*
int breakpoint()
Description:  Interrupt generated for a breakpoint that was reached, especially for debugging
Inputs: none
Outputs: int (0 if valid) - never reached
*/
int breakpoint()
{
    printf("Breakpoint reached");
    halt(HALT_CODE);
    return 0;
}

/*
int overflow()
Description:  Interrupt generated for any sort of overflow when logic arrives
Inputs: none
Outputs: int (0 if valid) - never reached
*/
int overflow()
{
    printf("Overflow");
    halt(HALT_CODE);
    return 0;
}

/*
int bound()
Description:  Interrupt generated for out of bound in logic
Inputs: none
Outputs: int (0 if valid) - never reached
*/
int bound()
{
    printf("Bounds range exceeded (BOUND)");
    halt(HALT_CODE);
    return 0;
}

/*
int InvalidOpcode()
Description:  Interrupt generated for trying to decode an invalid Opcode in logic
Inputs: none
Outputs: int (0 if valid) - never reached
*/
int InvalidOpcode()
{
    printf("Invalid opcode");
    halt(HALT_CODE);
    return 0;
}

/*
int WAIT()
Description:  Interrupt generated when the device is unavailable, must wait.
Inputs: none
Outputs: int (0 if valid) - never reached
*/
int WAIT()
{
    printf("Device not available");
    halt(HALT_CODE);
    return 0;
}

/*
int DoubleFault()
Description:  Interrupt generated for doublefaulting the kernel.
Inputs: none
Outputs: int (0 if valid) - never reached
*/
int DoubleFalt()
{
    printf("Double fault");
    halt(HALT_CODE);
    return 0;
}

/*
int overrun()
Description:  Interrupt generated for when the coprocessor overruns a segment.
Inputs: none
Outputs: int (0 if valid) - never reached
*/
int overrun()
{
    printf("Coprocessor segment overrun");
    halt(HALT_CODE);
    return 0;
}

/*
int TSS()
Description:  Interrupt generated for an invalid TSS element accessed.
Inputs: none
Outputs: int (0 if valid) - never reached
*/
int TSS()
{
    printf("Invalid TSS");
    halt(HALT_CODE);
    return 0;
}

/*
int segment()
Description:  Interrupt generated for segment not present.
Inputs: none
Outputs: int (0 if valid) - never reached
*/
int segment()
{
    printf("Segment not present");
    halt(HALT_CODE);
    return 0;
}

/*
int
Description:  Interrupt generated for having a segment fault wrt stack.
Inputs: none
Outputs: int (0 if valid) - never reached
*/
int stackSegment()
{
    printf("Stack-segment fault");
    halt(HALT_CODE);
    return 0;
}

/* protect()
int
Description:  Interrupt generated for trying to access something that has been protected.
Inputs: none
Outputs: int (0 if valid) - never reached
*/
int protect()
{
    printf("General protection fault");
    halt(HALT_CODE);
    return 0;
}

/*
int pagefault()
Description:  Interrupt generated for a page fault.
Inputs: none
Outputs: int (0 if valid) - never reached
*/
int pageFault()
{
    printf("Page fault");
    // uint32_t save_esp = 0;
    // uint32_t save_ebp asm("s_ebp") = 0;     
    // uint32_t cr2val = 0;
    // asm volatile
    // (
    //     "movl %%cr2, %%eax; \n"
    //     "movl %%eax, %0; \n"
    //     :"=g"(cr2val)
    //     :
    //     : "%eax"
    // );

    halt(HALT_CODE);

    return 0;
}

/*
int RESERVED
Description:  Interrupt generated for another reserved section.
Inputs: none
Outputs: int (0 if valid) - never reached
*/
int RESERVED2()
{
    printf("Reserved");
    
    halt(HALT_CODE);
    return 0;
}

/*
int
Description:  Interrupt generated for an FPU Error
Inputs: none
Outputs: int (0 if valid) - never reached
*/
int FPU()
{
    printf("x87 FPU error");
    halt(HALT_CODE);
    return 0;
}

/*
int align()
Description:  Interrupt generated for alignment issues.
Inputs: none
Outputs: int (0 if valid) - never reached
*/
int align()
{
    printf("Alignment check");
    halt(HALT_CODE);
    return 0;
}

/*
int machine()
Description:  Interrupt generated for machine faults.
Inputs: none
Outputs: int (0 if valid) - never reached
*/
int machine()
{
    printf("Machine check");
    halt(HALT_CODE);
    return 0;
}

/*
int SIMD
Description:  Interrupt generated for a SIMD Floating Point exception.
Inputs: none
Outputs: int (0 if valid) - never reached
*/
int SIMD()
{
    printf("SIMD Floating-Point Exception");
    halt(HALT_CODE);
    return 0;
}

/*
void init_IDT()
Description: Initialize the IDT.
Inputs: none
Outputs: none
Side Effects: Initializes and populates the IDT.
*/
void init_IDT()
{
    int i;
    // initializes our functions array
    for (i = 0; i < TOTAL_IDT_ENTRIES; i++)
    {
        /* i is being compared to the index in the Intel Document corresponding different entries on the IDT Table. */
        if (i == 0)
            funcs[i] = &divide_error;
        else if (i == 1) // Intel Document num
            funcs[i] = &RESERVED;
        else if (i == 2) // Intel Document num
            funcs[i] = &NMI;
        else if (i == 3) // Intel Document num
            funcs[i] = &breakpoint;
        else if (i == 4) // Intel Document num
            funcs[i] = &overflow;
        else if (i == 5) // Intel Document num
            funcs[i] = &bound;
        else if (i == 6) // Intel Document num
            funcs[i] = &InvalidOpcode;
        else if (i == 7) // Intel Document num
            funcs[i] = &WAIT;
        else if (i == 8) // Intel Document num
            funcs[i] = &DoubleFalt;
        else if (i == 9) // Intel Document num
            funcs[i] = &overrun;
        else if (i == 10) // Intel Document num
            funcs[i] = &TSS;
        else if (i == 11) // Intel Document num
            funcs[i] = &segment;
        else if (i == 12) // Intel Document num
            funcs[i] = &stackSegment;
        else if (i == 13) // Intel Document num
            funcs[i] = &protect;
        else if (i == 14) // Intel Document num
            funcs[i] = &pageFault;
        else if (i == 15) // Intel Document num
            funcs[i] = &RESERVED2;
        else if (i == 16) // Intel Document num
            funcs[i] = &FPU;
        else if (i == 17) // Intel Document num
            funcs[i] = &align;
        else if (i == 18) // Intel Document num
            funcs[i] = &machine;
        else if (i == 19) // Intel Document num
            funcs[i] = &SIMD;
        else
            funcs[i] = &generic_interrupt;
    }

    // populates our IDT
    for (i = 0; i < TOTAL_IDT_ENTRIES; i++)
    {
        idt_desc_t curr;
        int *curr_func_addr = (void *)funcs[i];

        if(i == SYSTEM_CALL_IDT_ENTRY)
        {   
            curr.seg_selector = KERNEL_CS;
            curr.reserved4 = 0;
            curr.reserved3 = 1;     // Set to 1 to convert to trap gate (see diagram: https://ibb.co/DD3pcFF)
            curr.reserved2 = 1;
            curr.reserved1 = 1;
            curr.size = 1;
            curr.reserved0 = 0;
            curr.dpl = USER_DPL;
            curr.present = 1;
            SET_IDT_ENTRY(curr, (uint32_t *)curr_func_addr);
            idt[i] = curr;
        }
        else if (curr_func_addr != 0)
        {
            curr.seg_selector = KERNEL_CS;
            curr.reserved4 = 0;
            curr.reserved3 = 0;
            curr.reserved2 = 1;
            curr.reserved1 = 1;
            curr.size = 1;
            curr.reserved0 = 0;
            curr.dpl = 0;
            curr.present = 1;
            SET_IDT_ENTRY(curr, (uint32_t *)curr_func_addr);
            idt[i] = curr;
        }
        else
        {
            printf("Bad function got into our array somehow");
            while (1)
            {
            };
        }
    }
    // puts system call
    SET_IDT_ENTRY(idt[SYSTEM_CALL_IDT_ENTRY], call_handler); // System Calls are in IDT entry table 0x80
    SET_IDT_ENTRY(idt[KEYBOARD_IDT_ENTRY], keyboard_handler_function);  // Keyboard is in IDT entry table 0x21
    SET_IDT_ENTRY(idt[RTC_IDT_ENTRY], rtc_handler_linkage);             // RTC is in IDT entry table 0x28
    SET_IDT_ENTRY(idt[PIT_IDT_ENTRY], pit_handler_linkage);             // RTC is in IDT entry table 0x28
    return;
}

