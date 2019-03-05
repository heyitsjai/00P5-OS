/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */
// See: https://wiki.osdev.org/PIC
/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

/* Files Imported */
#include "i8259.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_bit_mask;
uint8_t slave_bit_mask;


void i8259_init(void)
{
	master_bit_mask = ACTIVE_LOW;
	slave_bit_mask = ACTIVE_LOW;

	//init master
	outb(ACTIVE_LOW, MASTER_DATA_PORT);
	outb(ICW1, (MASTER_DATA_PORT-1)); //no offset for this
	outb(ICW2_MASTER, MASTER_DATA_PORT);
	outb(ICW3_MASTER, MASTER_DATA_PORT);
	outb(ICW4, MASTER_DATA_PORT);
	//init slave
	outb(ACTIVE_LOW, SLAVE_DATA_PORT);
	outb(ICW1, (SLAVE_DATA_PORT-1));
	outb(ICW2_SLAVE, SLAVE_DATA_PORT);
	outb(ICW3_SLAVE, SLAVE_DATA_PORT);
	outb(ICW4, SLAVE_DATA_PORT);

	enable_irq(2); //2 is the line with the slave in this example
}

/* Enable (unmask) the specified IRQ
 * Enable specified IRQ
 *
 * Input: irq_num - IRQ line to be enabled
*/

void enable_irq(uint32_t irq_num)
{
	uint32_t mask = 0x1;

	if ((irq_num >= 16)||(irq_num < 0)){
			printf("Invalid irq number");
			return;
	}
	//check for master or slave, mask to the correct irq location, and enable it
	//active low
	//disable follows similar path without the NOT, we're toggling bits at the correspon ding
	//spot
	if(irq_num>8){ //>8 are for slave lines
		mask = ((irq_num - 8) << mask);
		slave_bit_mask = (~mask & slave_bit_mask);
		outb(slave_bit_mask, SLAVE_DATA_PORT);
	} else {
		mask = (irq_num << mask);
		master_bit_mask = (~mask & master_bit_mask);
		outb(master_bit_mask, MASTER_DATA_PORT);
	}
}

void disable_irq(uint32_t irq_num)
{
	uint32_t mask = 0x1;

	if ((irq_num >= 16)||(irq_num < 0)){
		printf("Invalid irq number");
		return;
	}
	//now we're disabling, so use OR to enable the bits instead
	if(irq_num > 8){
		mask = ((irq_num - 8) << mask);
		slave_bit_mask = (mask | master_bit_mask);
		outb(slave_bit_mask, SLAVE_DATA_PORT);
	} else {
		mask = irq_num << mask;
		master_bit_mask = (mask | master_bit_mask); //mask out
		outb(master_bit_mask, MASTER_DATA_PORT);
	}
}

void send_eoi(uint32_t irq_num)
{
	// check non valid
	if(irq_num < 0 || irq_num > 15) {
		printf( "Invalid irq_num" );
		return;
	}

	uint32_t temp_irq_line;

	// master IRQ 0 to 7
	if(irq_num >= 0 && irq_num <= 7) {
		outb( (EOI | irq_num), (MASTER_DATA_PORT-1) );
	}
	// slave IRQ 8 to 15
	else if(irq_num >= 8 && irq_num <= 15) {
		temp_irq_line = irq_num - 8;
		outb( (EOI | temp_irq_line), (SLAVE_DATA_PORT-1) );
		outb( (EOI | 2), (MASTER_DATA_PORT-1) ); //2 for slave line
	}
}
