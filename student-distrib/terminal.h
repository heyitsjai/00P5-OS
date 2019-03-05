#ifndef TERMINAL_H__
#define TERMINAL_H__

/* Files Imported */
#include "lib.h"
#include "i8259.h"
#include "systemcalls.h"
#include "schedule.h"

#define BUFF_SIZE 	128

#define VIDEO       0xB8000
#define NUM_COLS    80
#define NUM_ROWS    25
#define	FOUR_KB		4096
#define ATTRIB      0x7
#define FAILURE     -1

typedef struct
{
    int x_position;
    int y_position;
	char keyboard_buffer[BUFF_SIZE];
    int buffer_index;    
	int read_index;
    int read_flag;
    //checkpoint 5
    int shell_process_num;
    int process_num;
    int previous_process_num;
    int process_activated;
    int video_memory_storage_location;
    volatile int rtc_lock;
    uint32_t current_esp;
    uint32_t current_ebp;
}terminal_t;

//going to initialize all terminals at once in the beginning
terminal_t terminal[3];

int active_terminal;
int is_visible; 

/* defined functions for terminal */
extern int32_t terminal_read(int32_t fd, void* buffer, int32_t nbytes);
extern int32_t terminal_write(int32_t fd, const void* buffer, int32_t nbytes);
int32_t terminal_open(const uint8_t* filename);
int32_t terminal_close(int32_t fd);
void active_terminal_scroll_up(int num_of_active); 
void active_terminal_putc(int num_of_active, uint8_t c);

void active_terminal_handle_enter(int num_of_active, uint8_t c);
void active_terminal_handle_tab(int num_of_active, uint8_t c);
void active_terminal_handle_backspace(int num_of_active, uint8_t c);
void active_terminal_handle_normal_char(int num_of_active, uint8_t c);


#endif /*TERMINAL_H__*/
