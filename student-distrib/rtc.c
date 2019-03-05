#include "rtc.h"
#include "terminal.h"
/* see: https://wiki.osdev.org/RTC */

volatile int is_rtc_interrupt_flag;

void rtc_init() {
  cli();
  // Turn on IRQ 8, periodic interrupt
  outb(0x8B, RTC_INPUT_PORT ); //value,port - port 70, register B, disables NMI
  char prev = inb( RTC_RW_PORT ); //read value of register 0x71
  outb(0x8B, RTC_INPUT_PORT ); //set the index again (0x8B, 0x70)
  outb((prev|0x40), RTC_RW_PORT ); //write the previous value ORed with 0x40 - turn on bit 6 of reg B
  enable_irq(0x08); //turn on irq 8 on RTC

  //rtc_open( 0 );
  sti();
}

void rtc_handle_interrupt() {

	cli();
		
	send_eoi(0x08); //irq 8
	is_rtc_interrupt_flag = 0;		// stop interrupts while handling
	//also from osdev - allows multiple rtc interrupts
	outb(0x8C, RTC_INPUT_PORT ); //select register C
	inb( RTC_RW_PORT ); //throw away contents in reg A (0x71)

	//test_interrupts(); //as per the specification, call this

	sti();

	//putc( '1' );
}


/**
 * Opens the RTC by setting the the frequency to the default value of 2Hz.
 *
 * Input:
 * fd - file directory
 */
int32_t rtc_open( const uint8_t* filename ) {
	// start RTC interrupt rate at default value of 2. nbytes is always 4 in this case
	int init_frequency = RTC_DEFAULT_RATE;
	rtc_write( 0, &init_frequency, 4 );

	return 0;
}


/**
 * The read system call reads data from a device (RTC).  This call returns the number of bytes read. For the real-time clock (RTC), this call should always return 0, but only
 * after an interrupt has occurred (set a flag and wait until the interrupt handler clears
 * it, then return 0).
 *
 * Input:
 * fd - file directory to write to
 * buf - void pointer to value of frequency we want to write
 * nbytes - number of bytes to write (always 4 in this case)
 * Return:
 * waits until interrupt flag is cleared (handler clears), then returns 0
 * always returns 0 eventually
 */
int32_t rtc_read( int32_t fd, void *buf, int32_t nbytes ) {
	is_rtc_interrupt_flag = 1;
	
	while(1) {
		if(!is_rtc_interrupt_flag){
			return 0;
		}
	}
	return 0;
}


/**
 * The write system call writes data to a device (RTC). In the case of the RTC,
 * the system call should always accept only a 4-byte integer specifying the interrupt rate
 * in Hz, and should set the rate of periodic interrupts accordingly. Writes to regular
 * files should always return -1 to indicate failure since the file system is read-only. The
 * call returns the number of bytes written, or -1 on failure
 *
 * Input:
 * fd - file directory to write to
 * buf - void pointer to value of frequency we want to write
 * nbytes - number of bytes to write (always 4 in this case)
 * Return:
 * 0 for write success, -1 for write fail
 */
int32_t rtc_write( int32_t fd, const void* buf, int32_t nbytes ) {

	// invalid write when buffer doesn't exist or not 4-byte integer
	if(buf == NULL || nbytes != 4) {
		return -1;
	}

	int interrupt_rate = *(int *)buf;

	int rate = 0;		// valid range is 1 to 15
	int32_t frequency_options[] = { 1024, 512, 256, 128, 64, 32, 16, 8, 4, 2 };
	// int32_t frequency_options[] = { 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024 };
	int frequency_array_length = 10;

	int i;
	for(i = 0; i < frequency_array_length; i++) {
		// interrupt rate buf must be in our array of options frequency_options
		if( interrupt_rate == frequency_options[i] ) {
			/*
			 * according to OS dev, to calculate new frequency:
			 * frequency =  32768 >> (rate-1), with range 1 to 15
			 * Since we have our max at 1024, start at rate 6
			*/
			rate = i + 6;
			break;
		}
	}

	// fastest rate you can select is 3, max is 15
	if ( rate <= 2 || rate > 15 ) {
		return -1;
	}

	rate &= 0x0F;
	outb( 0x8a, RTC_INPUT_PORT );
	char prev = inb( RTC_RW_PORT );
	outb( 0x8a, RTC_INPUT_PORT );
	outb( ((prev & 0xf0) | rate), RTC_RW_PORT );

	return 0;
}


/**
 * Closes RTC, returning 0
 *
 * Input:
 * fd - file directory
 * Return:
 * always returns success
 */
int32_t rtc_close( int32_t fd ) {
	return 0;
}
