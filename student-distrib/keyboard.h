/* Import Files */
#include "lib.h"
#include "idt_init.h"
#include "i8259.h"
#include "types.h"
#include "terminal.h"
#include "rtc.h"

/* Defined constants*/
#define keyboard_IRQ                    0x01 // IRQ 1 is for the keyboard
#define keyboard_port_address           0x60 // this is where we will read the key presses
#define keyboard_status_port_address    0x64
#define keyboard_max_buffer_size        128

/* Key Values */
// taken from: https://wiki.osdev.org/PS/2_Keyboard#Scan_Code_Set_1
#define CTRL_DOWN 		29
#define CTRL_RELEASE 	157
#define RSHIFT_DOWN 	54
#define RSHIFT_RELEASE 	182
#define LSHIFT_DOWN 	42
#define LSHIFT_RELEASE 	170
#define ALT_DOWN		56
#define ALT_RELEASE		184
#define ENTER 			28
#define TAB				15
#define BACKSPACE		14
#define CAPS_LOCK    	58
#define UP				72
#define DOWN			80
#define LEFT			75
#define RIGHT			77
#define F1				59
#define F2				60
#define F3				61
#define UNKNOWN 		0

#define ACTIVE          1
#define INACTIVE        0

#define NUM_COLS        80
#define NUM_ROWS        25
/* Defined Fuctions*/
extern void keyboard_initialize();
extern void keyboard_handler();
extern unsigned char keypress_handler(int keypress);

void keyboard_interrupt();
