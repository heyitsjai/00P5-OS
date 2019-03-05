#include "keyboard.h"
#include "lib.h"
#include "idt_init.h"
#include "i8259.h"

/* Global Keyboard Status Variables */
volatile int SHIFT_STATUS = INACTIVE;
volatile int ALT_STATUS = INACTIVE;
volatile int CAPS_STATUS = INACTIVE;
volatile int CTRL_STATUS = INACTIVE;

int left_right_cursor_counter = 0;
int left_right_cursor_offsetter = 0;

// tables below taken from: https://wiki.osdev.org/PS/2_Keyboard#Scan_Code_Set_1
char keyboard_scancode_regular[keyboard_max_buffer_size] =
    {
        0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', /* Backspace */ '\t', /* Tab */
      'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', /* Enter key */ 0, /* Control */
      'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, /* Left shift */
     '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, /* Right shift */
      '*',
        0,  /* Alt */
      ' ',  /* Space bar */
        0,  /* Caps lock */
        0,  /* F1 key ...  */
        0,   0,   0,   0,   0,   0,   0,   0,
        0,  /* ... F10 */
        0,  /* Num lock */
        0,  /* Scroll Lock */
        0,  /* Home key */
        0,  /* Up Arrow */
        0,  /* Page Up */
      '-',
        0,  /* Left Arrow */
        0,
        0,  /* Right Arrow */
      '+',
        0,  /* End key*/
        0,  /* Down Arrow */
        0,  /* Page Down */
        0,  /* Insert Key */
        0,  /* Delete Key */
        0,   0,   0,
        0,  /* F11 Key */
        0,  /* F12 Key */
        0,  /* All other keys are undefined */
    };

char keyboard_scancode_shift[keyboard_max_buffer_size] =
    {
        0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', /* Backspace */ '\t',         /* Tab */
      'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', /* Enter key */ 0, /* Control */
      'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~',   0, /* Left shift */
     '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',   0, /* Right shift */
      '*',
        0,  /* Alt */
      ' ',  /* Space bar */
        0,  /* Caps lock */
        0,  /* F1 key ... */
        0,   0,   0,   0,   0,   0,   0,   0,
        0,  /* ... F10 */
        0,  /* Num lock*/
        0,  /* Scroll Lock */
        0,  /* Home key */
        0,  /* Up Arrow */
        0,  /* Page Up */
      '-',
        0,  /* Left Arrow */
        0,
        0,  /* Right Arrow */
      '+',
        0,  /* End key*/
        0,  /* Down Arrow */
        0,  /* Page Down */
        0,  /* Insert Key */
        0,  /* Delete Key */
        0,   0,   0,
        0,  /* F11 Key */
        0,  /* F12 Key */
        0,  /* All other keys are undefined */
    };

char keyboard_scancode_caps[keyboard_max_buffer_size] =
    {
        0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', /* Backspace */ '\t',/* Tab */
      'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n', /* Enter key */ 0, /* Control */
      'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`',   0,  /* Left shift */
      '\\', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/',   0,  /* Right shift */
      '*',
        0,  /* Alt */
      ' ',  /* Space bar */
        0,  /* Caps lock */
        0,  /* F1 key ... */
        0,   0,   0,   0,   0,   0,   0,   0,
        0,  /* ... F10 */
        0,  /* Num lock*/
        0,  /* Scroll Lock */
        0,  /* Home key */
        0,  /* Up Arrow */
        0,  /* Page Up */
      '-',
        0,  /* Left Arrow */
        0,
        0,  /* Right Arrow */
      '+',
        0,  /* End key*/
        0,  /* Down Arrow */
        0,  /* Page Down */
        0,  /* Insert Key */
        0,  /* Delete Key */
        0,   0,   0,
        0,  /* F11 Key */
        0,  /* F12 Key */
        0,  /* All other keys are undefined */
    };

char keyboard_scancode_shift_and_caps[keyboard_max_buffer_size] =
    {
        0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', /* Backspace */ '\t', /* Tab */
      'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '{', '}', '\n', /* Enter key */ 0,  /* Control */
      'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ':', '\"', '~',   0, /* Left shift */
     '|', 'z', 'x', 'c', 'v', 'b', 'n', 'm', '<', '>', '?',   0, /* Right shift */
      '*',
        0,  /* Alt */
      ' ',  /* Space bar */
        0,  /* Caps lock */
        0,  /* F1 key ... */
        0,   0,   0,   0,   0,   0,   0,   0,
        0,  /* ... F10 */
        0,  /* Num lock*/
        0,  /* Scroll Lock */
        0,  /* Home key */
        0,  /* Up Arrow */
        0,  /* Page Up */
      '-',
        0,  /* Left Arrow */
        0,
        0,  /* Right Arrow */
      '+',
        0,  /* End key*/
        0,  /* Down Arrow */
        0,  /* Page Down */
        0,  /* Insert Key */
        0,  /* Delete Key */
        0,   0,   0,
        0,  /* F11 Key */
        0,  /* F12 Key */
        0,  /* All other keys are undefined */
    };

/* void initialize_keyboard()
 * Input: none
 * Output: none
 * Functionality: enable the irq to the keyboard irq
 */
void keyboard_initialize()
{
    enable_irq(keyboard_IRQ);
    terminal_open(NULL);
    active_terminal = 0;
    is_visible = 0;
    CAPS_STATUS = INACTIVE;
    SHIFT_STATUS = INACTIVE;
    CTRL_STATUS = INACTIVE;
    ALT_STATUS = INACTIVE;
}

/* void keyboard_handler()
 *Input: none
 *Output: none
 *Functionality: handles the keypress value by mapping to the scancode then creates end of interrupt signal
 */
void keyboard_handler()
{
    cli();
    send_eoi(keyboard_IRQ);
    unsigned char pressed_char;
    unsigned char keypress_scancode = inb(keyboard_port_address);   //get the input from the keyboard
        switch(keypress_scancode)
        {
		    case CTRL_DOWN:
                CTRL_STATUS = ACTIVE;
			    break;
		    case CTRL_RELEASE:
                CTRL_STATUS = INACTIVE;
                break;
	    	case RSHIFT_DOWN:
                SHIFT_STATUS = ACTIVE;
	    		break;
		    case RSHIFT_RELEASE:
                SHIFT_STATUS = INACTIVE;
			    break;
	    	case LSHIFT_DOWN:
                SHIFT_STATUS = ACTIVE;
	    		break;
		    case LSHIFT_RELEASE:
                SHIFT_STATUS = INACTIVE;
			    break;
		    case ALT_DOWN:
                ALT_STATUS = ACTIVE;
			    break;
		    case ALT_RELEASE:
                ALT_STATUS = INACTIVE;
			    break;
		    case CAPS_LOCK:
			    if (CAPS_STATUS == INACTIVE)
                    CAPS_STATUS = ACTIVE;
			    else
                    CAPS_STATUS = INACTIVE;
			    break;
            case ENTER:
                putc('\n');
                terminal[active_terminal].read_index = terminal[active_terminal].buffer_index;
                terminal[active_terminal].read_flag = 1;
                terminal[active_terminal].buffer_index = 0;
                left_right_cursor_counter = 0;
                left_right_cursor_offsetter = 0;
   			    break;
            case TAB:
                putc('\t');
   			    break;
            case BACKSPACE:
                if(terminal[active_terminal].buffer_index < 0)
                    terminal[active_terminal].buffer_index = 0;
                if(terminal[active_terminal].buffer_index > 0)
                {
                    putc('\b');
                    terminal[active_terminal].buffer_index--;
                    left_right_cursor_offsetter++;
                }
   			    break;
            case UP:
   			    break;
            case DOWN:
   			    break;
            case LEFT:
                if(terminal[active_terminal].buffer_index <= 0)
                    break;
                else
                {
                    if(terminal[active_terminal].buffer_index > left_right_cursor_counter)
                        left_right_cursor_counter = terminal[active_terminal].buffer_index;
                    terminal[active_terminal].buffer_index--;
                    screen_x--;
                    update_cursor(screen_x, screen_y);
   			        break;
                }
            case RIGHT://todo buffer stuff for interrupts, if it pops up it messes up the buffer
                if((left_right_cursor_counter + left_right_cursor_offsetter) % NUM_COLS <= screen_x)
                    break;
                else
                {
                    if(left_right_cursor_offsetter > 0)
                        left_right_cursor_offsetter--;
                    terminal[active_terminal].buffer_index++;
                    screen_x++;
                    update_cursor(screen_x, screen_y);
   			        break;
                }
            case UNKNOWN:
                break;
		    default:
                if(ALT_STATUS == ACTIVE)
                {
                    if(keypress_scancode == F1)
                    {
                        //printf("Terminal: %d", active_terminal);
                        is_visible = 0;
                        save_and_switch_term(0);
                        active_terminal = 0;

                    }    
                    else if(keypress_scancode == F2)
                    {
                        //printf("Terminal: %d", active_terminal);
                        is_visible = 1; 
                        save_and_switch_term(1);
                        active_terminal = 1;
                    }
                    else if(keypress_scancode == F3)
                    {
                        //printf("Terminal: %d", active_terminal);
                        is_visible = 2; 
                        save_and_switch_term(2);
                        active_terminal = 2;
                    }
                    else
                    {
                        //do nothing
                    }
                }
                pressed_char = keypress_handler(keypress_scancode);

                if((int)pressed_char==0){
                  break;
                }
                if (CTRL_STATUS)    //clear the screen
                {
                    if(pressed_char == 'l' || pressed_char == 'L')
                    {
                        clear();
                        break;
                    }
                }


                if(terminal[active_terminal].buffer_index < keyboard_max_buffer_size)    //print out the characters using the buffer
                {
                    terminal[active_terminal].keyboard_buffer[terminal[active_terminal].buffer_index] = pressed_char;
                    terminal_write(0, &terminal[active_terminal].keyboard_buffer[terminal[active_terminal].buffer_index], 1);
                    terminal[active_terminal].buffer_index++;
                }
			    break;
	    }

      sti();
}

/* char keypress_handler(int keypress)
 * Input: scan code
 * Output: pressed char
 * Functionality: gives the relevant key based on the active status of keyboard flags (SHIFT, ALT, CTRL, CAPS)
 */
extern unsigned char keypress_handler(int keypress)
{
    if (keypress > 0 && keypress < keyboard_max_buffer_size)
    {
        if (SHIFT_STATUS && !CAPS_STATUS)
            return keyboard_scancode_shift[keypress];
        else if (!SHIFT_STATUS && CAPS_STATUS)
            return keyboard_scancode_caps[keypress];
        else if (SHIFT_STATUS && CAPS_STATUS)
            return keyboard_scancode_shift_and_caps[keypress];
        else
            return keyboard_scancode_regular[keypress];
    }
    else
        return 0;
}

