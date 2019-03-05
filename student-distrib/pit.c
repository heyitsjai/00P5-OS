#include "pit.h"

// see: https://wiki.osdev.org/Programmable_Interval_Timer?fbclid=IwAR0yH4amdyzz3G9RWtpkCTMmQgqhC-RzR6K9fMZgllOS4ntRY_Cmzp9NZuU
// see: https://wiki.osdev.org/Detecting_CPU_Speed 
// see: https://en.wikibooks.org/wiki/X86_Assembly/Programmable_Interval_Timer

void PIT_initialize(){

    schedule_initialization();
    
    //following https://en.wikibooks.org/wiki/X86_Assembly/Programmable_Interval_Timer
	enable_irq( PIT_IRQ );

	outb( MODE_2, MODE ); 

    //take lower 8 bits first and send, then shift the frequency to send the higher byte
    outb(PIT_Ocillator_Frequency && 0xff, CHANNEL_0);
    //take higher 8 bits 
    outb((PIT_Ocillator_Frequency >> 8), CHANNEL_0);

}

void PIT_handler(){
    //prevent keyboard (0x01) from interrupting while pit is working
    disable_irq(0x01);
    send_eoi(PIT_IRQ);
    schedule_handling();
}
