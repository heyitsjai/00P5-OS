#ifndef _IDT_INIT_H
#define _IDT_INIT_H

#include "x86_desc.h"
#include "lib.h"

void initialize_idt();

void divide_error();
void debug_exception();
void nmi_interrupt();
void breakpoint_interrupt();
void overflow_exception();
void bound_range_exceeded();
void invalid_opcode();
void device_not_available();
void double_fault();
void coprocessor_segment_overrun();
void invalid_tss();
void segment_not_present();
void stack_segment_fault();
void general_protection();
void page_fault();
void reserved();
void floating_point_error();
void alignment_check();
void machine_check();
void simd_floating_point_exception();
void undefined_interrupt();

#endif
