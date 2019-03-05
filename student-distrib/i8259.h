/* i8259.h - Defines used in interactions with the 8259 interrupt
 * controller
 * vim:ts=4 noexpandtab
 */

#ifndef _I8259_H
#define _I8259_H

/* Files Imported */
#include "types.h"
#include "lib.h"


#define MASTER_DATA_PORT    0x21
#define SLAVE_DATA_PORT    0xA1

#define ACTIVE_LOW		0xFC

#define FULL_SIZE		16
#define	HALF_SIZE		8

//intel manual defines these words
#define ICW1                0x11
#define ICW2_MASTER         0x20
#define ICW2_SLAVE          0x28
#define ICW3_MASTER         0x04
#define ICW3_SLAVE          0x02
#define ICW4                0x01
#define EOI                 0x60
//init pic
void i8259_init(void);
//disable the irq line in the parameter
void disable_irq(uint32_t irq_num);
//enable the irq line in the parameter
void enable_irq(uint32_t irq_num);

/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num);

#endif /* _I8259_H */
