
#ifndef _PAGING_H
#define _PAGING_H
#include "types.h"

#define num_page_entries 1024
#define page_size 4096 //4 kB pages
// see: https://wiki.osdev.org/Printing_To_Screen for video memory location 
#define video_memory_buffer_loc 0xB8000
#define PRESENT_WRITE 		0x00000003
#define WRITE_ONLY 			0x00000002
#define PRESENT_WRITE_SIZE 	0x00000087
#define KERNEL				0x00000083
#define VID_MEM_PRESENT		0x00000007

#define FOURKB_HEX     			0x1000
#define VIDMEM_OFFSET 			0xB8000
#define VIDMEM_OFFSET_1 		VIDMEM_OFFSET + FOURKB_HEX
#define VIDMEM_OFFSET_2 		VIDMEM_OFFSET_1 + FOURKB_HEX
#define VIDMEM_OFFSET_3 		VIDMEM_OFFSET_2 + FOURKB_HEX	

#define KERNELMEM			0x400000
#define FOUR_MB				0x400000
#define EIGHT_MB			0x800000
#define VIRTUAL 			0x8000000
uint32_t pDirectory[num_page_entries] __attribute__((aligned (page_size)));
uint32_t pTable[num_page_entries] __attribute__((aligned (page_size))); //first page table
uint32_t pTable2[num_page_entries] __attribute__((aligned (page_size)));
uint32_t videopTable[num_page_entries] __attribute__((aligned (page_size)));

void paging_init();


#endif
