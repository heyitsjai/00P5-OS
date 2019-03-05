#ifndef _RTC_H
#define _RTC_H

#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"

#define RTC_INPUT_PORT		0x70 
#define RTC_RW_PORT			0x71

#define RTC_DEFAULT_RATE	2

void rtc_init();
void rtc_handle_interrupt();
void rtc_interrupt();

int32_t rtc_open( const uint8_t* filename );
int32_t rtc_read( int32_t fd, void *buf, int32_t nbytes );
int32_t rtc_write( int32_t fd, const void* buf, int32_t nbytes );
int32_t rtc_close( int32_t fd );


#endif
