#ifndef PIT_H
#define PIT_H

#include "lib.h"
#include "i8259.h"
#include "schedule.h"

// from https://wiki.osdev.org/Programmable_Interval_Timer#Using_the_IRQ_to_Implement_sleep
// I/O port     Usage
// 0x40         Channel 0 data port (read/write) // use this one because it is connected to PIC 
// 0x41         Channel 1 data port (read/write)
// 0x42         Channel 2 data port (read/write)
// 0x43         Mode/Command register (write only, a read is ignored)

#define CHANNEL_0 0x40 // <--- PIC 
#define CHANNEL_1 0x41
#define CHANNEL_2 0x42
#define MODE 0x43 
#define PIT_IRQ 0

// see: https://wiki.osdev.org/Detecting_CPU_Speed:
// The PIT has two operating mode that can be useful for telling the cpu speed:
// 1. the periodic interrupt mode (0x36), in which a signal is emitted to the interrupt controller at a fixed frequency. This is especially interesting on PIT channel 0 which is bound to IRQ0 on a PC.
// 2. the one shot mode (0x34), in which the PIT will decrease a counter at its top speed (1.19318 MHz) until the counter reaches zero

#define PIT_Ocillator_Frequency 11931 //using 100 hz frequencey 
#define MODE_1 0x36
#define MODE_2 0x34 //<-- one shot

extern void PIT_initialize();
extern void PIT_handler();
void PIT_interrupt();


#endif

