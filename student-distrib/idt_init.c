#include "idt_init.h"
#include "rtc.h"
#include "keyboard.h"
#include "systemcalls.h"
#include "pit.h"

// http://www.logix.cz/michal/doc/i386/chp09-08.htm#09-08-01
void initialize_idt() {
    int system_call = 0x80;

    //load the IDT using its pointer
    lidt(idt_desc_ptr);

    int i;
    //initialize the IDT - based on OSDev and Reference Guide
    for( i = 0; i < NUM_VEC; i++ ) {
        idt[i].seg_selector = KERNEL_CS;
        idt[i].reserved4 = 0x0;
        idt[i].reserved3 = 0x1;
        idt[i].reserved2 = 0x1;
        idt[i].reserved1 = 0x1;
        idt[i].size = 0x1;
        idt[i].reserved0 = 0x0;
        idt[i].dpl = 0x0;
        idt[i].present = 0x1;
        //check if IDT entry is the system call one
        if( i == system_call ) {
            //if system call - set privilege level to 3
            idt[i].dpl = 0x3;
        }
        //user defined interrupts
        if( i > 31 ) { // should be changed to 31
           SET_IDT_ENTRY(idt[i], undefined_interrupt);
        }
    }

  //RTC INTERRUPT
  SET_IDT_ENTRY(idt[0x20], PIT_interrupt ); //0x20 in IDT for PIT
  SET_IDT_ENTRY(idt[0x21], keyboard_interrupt);
  SET_IDT_ENTRY(idt[0x28], rtc_interrupt); //0x28 in IDT for rtc
  SET_IDT_ENTRY(idt[0x80], sys_call_interrupt); //going into this eax has our system call number
  
  SET_IDT_ENTRY(idt[0], divide_error);
  SET_IDT_ENTRY(idt[1], debug_exception);
  SET_IDT_ENTRY(idt[2], nmi_interrupt);
  SET_IDT_ENTRY(idt[3], breakpoint_interrupt);
  SET_IDT_ENTRY(idt[4], overflow_exception);
  SET_IDT_ENTRY(idt[5], bound_range_exceeded);
  SET_IDT_ENTRY(idt[6], invalid_opcode);
  SET_IDT_ENTRY(idt[7], device_not_available);
  SET_IDT_ENTRY(idt[8], double_fault);
  SET_IDT_ENTRY(idt[9], coprocessor_segment_overrun);
  SET_IDT_ENTRY(idt[10], invalid_tss);
  SET_IDT_ENTRY(idt[11], segment_not_present);
  SET_IDT_ENTRY(idt[12], stack_segment_fault);
  SET_IDT_ENTRY(idt[13], general_protection);
  SET_IDT_ENTRY(idt[14], page_fault);
  SET_IDT_ENTRY(idt[15], reserved);
  SET_IDT_ENTRY(idt[16], floating_point_error);
  SET_IDT_ENTRY(idt[17], alignment_check);
  SET_IDT_ENTRY(idt[18], machine_check);
  SET_IDT_ENTRY(idt[19], simd_floating_point_exception);

}


// IA32 ISA Ref Manual 6.4.1 interrupt table
//for each interrupt spin as a sort of "blue screen"
// 0

void divide_error() {
  ////clear();
    puts("Divide error");
    while(1);
}

// 1
void debug_exception() {
  ////clear();
    printf("Debug exceptions");
    while(1);
}

// 2
void nmi_interrupt() {
  ////clear();
    printf("Non-maskable external interrupt");
    while(1);
}

// 3
void breakpoint_interrupt() {
  ////clear();
    printf("Breakpoint");
    while(1);
}

// 4
void overflow_exception() {
  ////clear();
    printf("Overflow");
    while(1);
}

// 5
void bound_range_exceeded() {
  ////clear();
    printf("Bounds range exceeded");
    while(1);
}

// 6
void invalid_opcode() {
  ////clear();
    printf("Invalid opcode");
    while(1);
}

// 7
void device_not_available() {
  ////clear();
    printf("Device not available");
    while(1);
}

// 8
void double_fault() {
    //////clear();
    printf("Double fault");
    while(1);
}

// 9
void coprocessor_segment_overrun() {
  ////clear();
    printf("Coprocessor segment overrun");
    while(1);
}

// 10
void invalid_tss() {
  ////clear();
    printf("Invalid TSS");
    while(1);
}

// 11
void segment_not_present() {
  ////clear();
    printf("Segment not present");
    while(1);
}

// 12
void stack_segment_fault() {
  ////clear();
    printf("Stack segment fault");
    while(1);
}

// 13
void general_protection() {
  ////clear();
    printf("General protection");
    while(1);
}

// 14
void page_fault() {
    //////clear();
    int i;
    asm("\t movl %%ecx,%0" : "=r"(i));
    printf("%x ",i+1);
    printf("Page fault");
    while(1);
}

// 15
void reserved() {
  ////clear();
    printf("Reserved");
    while(1);
}

// 16
void floating_point_error() {
  ////clear();
    printf("Floating point error");
    while(1);
}

// 17
void alignment_check() {
  ////clear();
    printf("Alignment check");
    while(1);
}

// 18
void machine_check() {
  ////clear();
    printf("Machine check");
    while(1);
}

// 19
void simd_floating_point_exception() {
  ////clear();
    printf("SIMD floating-point exception");
    while(1);
}

// for undefined
void undefined_interrupt() {
  ////clear();
    printf("Undefined interrupt");
    //while(1);
}
