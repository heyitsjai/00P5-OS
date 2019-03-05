
#include "schedule.h"


void schedule_initialization()
{
    active_schedule.current_running_process_count = 0;
    active_schedule.next_terminal_in_schedule = 2;
    active_schedule.schedule_activated = 1;
}

//MP3.5 Scheduling
    //utilize the kernel stack (HALT)
    //again you will be using assembly to the context switch 
    //switch esp/ebp to next process' kernel stack
    //restore the next process TSS
    //FLUSH TLB on process switch 

void schedule_handling()
{
	cli();
    int last_running_process;
    if(active_schedule.schedule_activated) {
        if(active_schedule.current_running_process_count < MAX_TERMINALS) // total # of terminals not max but not 0
        {
        // can active_schedule another task
		// need to put current esp, ebp to the terminal struct before switching tasks 
	    	if(active_schedule.current_running_process_count != 0)
            {
                asm volatile (
                    "movl %%esp, %0;"
                    "movl %%ebp, %1;"
                    :
                    : "m"(terminal[active_terminal].current_esp), "m"(terminal[active_terminal].current_ebp)
                ); //m for memory address of this to be used
            }
            last_running_process = active_schedule.current_running_process_count;
            save_and_switch_term(last_running_process); // decrement because we want the previous terminal, not the incremented one

            active_terminal = last_running_process;
            active_schedule.current_running_process_count++;
        sti();
           execute((uint8_t*)"shell");
        }
        else if(active_schedule.current_running_process_count == MAX_TERMINALS) // all terminals just got instantiated
        {
            //already running max number of tasks
            asm volatile (
		    	"movl %%esp, %0;"
		    	"movl %%ebp, %1;"
		    	: "=r"(terminal[2].current_esp), "=r"(terminal[2].current_ebp) //we can have 2 
		    ); 
            active_schedule.current_running_process_count++;
            save_and_switch_term(0); 
        }
        else //need to round robin between existing tasks
        {
            asm volatile (
		    	"movl %%esp, %0;"
		    	"movl %%ebp, %1;"
		    	: "=r"(terminal[active_schedule.next_terminal_in_schedule].current_esp), "=r"(terminal[active_schedule.next_terminal_in_schedule].current_ebp) //we can have 2 
		    );

            active_schedule.next_terminal_in_schedule++;
            active_schedule.next_terminal_in_schedule = active_schedule.next_terminal_in_schedule % 3;

            if(active_schedule.next_terminal_in_schedule != active_terminal){ //keeps keyboard from interrupting a different term than active
                disable_irq(0x01); //if pit is not looking at current term, keep keyboard from interrupting
            }
            else {
                enable_irq(0x01); //have to be able to type on current term
            }
        //set up paging
		    int pid = terminal[active_schedule.next_terminal_in_schedule].process_num; 
            pDirectory[32]=(EIGHTMB+(FOURMB*pid))|0x87; //0x87 permissions for program loading

        //tlb flush- change cr3
            asm volatile (
                        "movl %%cr3, %%ebx;"
                        "movl %%ebx, %%cr3;"
                        :
                        :
                        :"%ebx"
            );

		    tss.esp0 = EIGHTMB - (EIGHTKB)*pid - 4; //gotten from piazza - (-4 for the spacing) 

		    asm volatile (
			    "movl %0, %%esp;"
			    "movl %1, %%ebp;"
			    : 
			    : "r"(terminal[active_schedule.next_terminal_in_schedule].current_esp), "r"(terminal[active_schedule.next_terminal_in_schedule].current_ebp) 
			    );
        }
	    sti(); 
    } else {
        printf("schedule not yet activated for this OS");
    }
}

