
#include "terminal.h"



/* int32_t terminal_open(const uint8_t* filename)
 * Input: file
 * Output:
 * Functionality: initialize terminal driver
 */
int32_t terminal_open(const uint8_t* filename)
{
    int i;
    int j; 
    for(j=0;j<3;j++){
		//init all vars
        terminal[j].video_memory_storage_location = VIDEO+FOUR_KB*(j+1); // starting location of 4kB "buffer" for each terminal - for switching terminals
		terminal[j].x_position = 0;
		terminal[j].y_position = 0;

		//fill the keyboard buffer with empties
		for(i = 0; i < BUFF_SIZE; i++)
			terminal[j].keyboard_buffer[i] = '\0';
        //copied from lib.c to clear video memory, now we're using our "buffer" for each terminal instead:
        int32_t i;
        for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
            *(uint8_t *)(terminal[j].video_memory_storage_location + (i << 1)) = ' ';
            *(uint8_t *)(terminal[j].video_memory_storage_location + (i << 1) + 1) = ATTRIB;
        }
		//init more vars
		terminal[j].buffer_index = 0;
		terminal[j].read_index = 0;
		terminal[j].read_flag = 0;
        terminal[j].process_activated = 0;
        terminal[j].current_esp = 0;
        terminal[j].current_ebp = 0;
        terminal[j].rtc_lock = 0;
    }
    return 0;
}

/* int32_t terminal_close(int32_t fd)
 * Input: fd
 * Output:
 * Functionality: close terminal driver
 */
int32_t terminal_close(int32_t fd)
{
	return 0;
}

/* int32_t terminal_read(int32_t fd, void* buffer, int32_t nbytes)
 * Input: destination, buffer, bytes to copy
 * Output: number of bytes read
 * Functionality: read to terminal from keyboard, ile, rtc, or directory
 */
int32_t terminal_read(int32_t fd, void* buffer, int32_t nbytes)
{
    int i;
    //check if the buffer exists and the bytes are valid
	if (buffer == NULL || nbytes < 0){
        printf("fail");

		return FAILURE;
    }
	else if (nbytes == 0){
            printf("no bytes");

		return 0;
    }
    else
    {
	    char* temp_buffer = (char*)buffer;

        //keep reading until the enter key is pressed which will offset the flag in keyboard.c
        while(terminal[active_terminal].read_flag != 1);
        //copy over everything that has been typed on the terminal
        for(i = 0; i < terminal[active_terminal].read_index; i++)
		    temp_buffer[i] = terminal[active_terminal].keyboard_buffer[i];

        //add newline and ending chars onto the final buffer
        temp_buffer[terminal[active_terminal].read_index] = '\n';
        terminal[active_terminal].read_index++;
        temp_buffer[terminal[active_terminal].read_index] = '\0';
        terminal[active_terminal].read_flag=0;
        //return the number of bytes read
        return terminal[active_terminal].read_index;
    }
}

/* nt32_t terminal_write(int32_t fd, const void* buffer, int32_t nbytes)
 * Input: destination, buffer, bytes to copy
 * Output: number of bytes written
 * Functionality: write from terminal
 */
int32_t terminal_write(int32_t fd, const void* buffer, int32_t nbytes)
{
    int i;

    //check if the buffer exists and the bytes are valid
	if (buffer == NULL || nbytes < 0)
		return FAILURE;
	else if(terminal[active_terminal].buffer_index == BUFF_SIZE)
		return 0;
    else
    {
        char* temp_buffer = (char*)buffer;
	    cli();
	    // print buffer characters
	    for(i = 0; i < nbytes; i++)
	    {
		    if(temp_buffer[i])
		    {
			    if( (active_schedule.current_running_process_count < 3 )|| (active_schedule.next_terminal_in_schedule == active_terminal)) 
                //not at base terminal - base terminal uses the default putc while the others need offsets for their specific terminal
                {
                    putc((char) temp_buffer[i]);
                }
                else
                {
                    active_terminal_putc(active_schedule.next_terminal_in_schedule, (char) temp_buffer[i]);

                }
                    
			}
	    }
	    sti();
    }

    //return number of bytes written
   	return nbytes;

}

//modified from lib.c to account for active teriminal
void active_terminal_scroll_up(int num_of_active)
{
    int i, j;
    //iterate and get the next row's video mem to display on current row
    for(i = 0; i < NUM_ROWS - 1 ; i++)
    {
        for(j = 0; j < NUM_COLS; j++)
        {
            *(uint8_t *)(terminal[num_of_active].video_memory_storage_location + ((NUM_COLS * i + j) << 1)) =  *(uint8_t *)(terminal[active_terminal].video_memory_storage_location + ((NUM_COLS * (i + 1) + j) << 1));
            *(uint8_t *)(terminal[num_of_active].video_memory_storage_location + ((NUM_COLS * i + j) << 1) + 1) = ATTRIB;
        }
    }
    //put space into every column location on the bottom row to clear it
    for(j = 0; j < NUM_COLS; j++)
    {
        *(uint8_t *)(terminal[num_of_active].video_memory_storage_location + ((NUM_COLS * (NUM_ROWS - 1) + j) << 1)) = ' ';
        *(uint8_t *)(terminal[num_of_active].video_memory_storage_location + ((NUM_COLS * (NUM_ROWS - 1) + j) << 1) + 1) = ATTRIB;
    }
    terminal[num_of_active].x_position = 0;
    terminal[num_of_active].y_position = NUM_ROWS - 1;
}

void active_terminal_putc(int num_of_active, uint8_t c) {
    if(c == '\n' || c == '\r') {    //handle enter
        active_terminal_handle_enter(num_of_active, c);
    }
    else if(c == '\t')              // handle tab, increment by 4 spaces at a time
    {
        active_terminal_handle_tab(num_of_active, c);
    }
    else if(c == '\b')              //handle backspace
    {
        active_terminal_handle_backspace(num_of_active, c);
    }    
    else                            //handle normal characters
    {
        active_terminal_handle_normal_char(num_of_active, c);
    }
}

void active_terminal_handle_enter(int num_of_active, uint8_t c)
{
    terminal[num_of_active].y_position++;
    terminal[num_of_active].x_position = 0;
    if (terminal[num_of_active].y_position == NUM_ROWS) //if at bottom, scroll up
        active_terminal_scroll_up(num_of_active);
}
void active_terminal_handle_tab(int num_of_active, uint8_t c)
{
    if (terminal[num_of_active].x_position < (NUM_COLS - 8))
        terminal[num_of_active].x_position = terminal[num_of_active].x_position + 8;
    else
    {
        terminal[num_of_active].y_position++;
        terminal[num_of_active].x_position = 8;
        if (terminal[num_of_active].y_position == NUM_ROWS)
            active_terminal_scroll_up(num_of_active);
    }
}
void active_terminal_handle_backspace(int num_of_active, uint8_t c)
{
    if (terminal[num_of_active].x_position > 0) //delete 1 char, same line
        terminal[num_of_active].x_position--;
    else
    {
        if (screen_y > 0) //go to end of row at previous line only if not at top, might need to change
        {
            terminal[num_of_active].x_position = NUM_COLS - 1;
            terminal[num_of_active].y_position--;
        }
    }
    //clear previous char with space char
    *(uint8_t *)(terminal[num_of_active].video_memory_storage_location + ((NUM_COLS * terminal[num_of_active].y_position + terminal[num_of_active].x_position) << 1)) = ' ';
    *(uint8_t *)(terminal[num_of_active].video_memory_storage_location + ((NUM_COLS * terminal[num_of_active].y_position + terminal[num_of_active].x_position) << 1) + 1) = ATTRIB;
}
void active_terminal_handle_normal_char(int num_of_active, uint8_t c)
{
    //put char into video memory
    *(uint8_t *)(terminal[num_of_active].video_memory_storage_location + ((NUM_COLS * terminal[num_of_active].y_position + terminal[num_of_active].x_position) << 1)) = c;
    *(uint8_t *)(terminal[num_of_active].video_memory_storage_location + ((NUM_COLS * terminal[num_of_active].y_position + terminal[num_of_active].x_position) << 1) + 1) = ATTRIB;
    if (screen_x == NUM_COLS - 1) //if trying to insert at end of line, update cursor placement, might need to change
    {
        terminal[num_of_active].x_position = 0;
        terminal[num_of_active].y_position++;
        if (terminal[num_of_active].y_position == NUM_ROWS) //check if at bottom of screen
            active_terminal_scroll_up(num_of_active);
    }
    else //normal insert update x coord for cursor
    {
        terminal[num_of_active].x_position++;
    }
}
