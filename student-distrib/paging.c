
//need page directory and tables to be initialized in this file
// 4 MB per page, 4096/4 = 1024 PTEs
// 2^10 ptes per table, 2^12 bytes per page, page directory = 2^10 entries total
#include "paging.h"

//https://wiki.osdev.org/Paging - for page directory and table format
//https://wiki.osdev.org/Setting_Up_Paging -  lots of code comes from here
void paging_init() {
  int i=0;
  for(i=0;i<num_page_entries;i++) {
    pDirectory[i] = 0x00000002; //enables r/w, supervisor, not present
    pTable[i] = (i*0x1000) | 3; //enables r/w, supervisor, present
    videopTable[i]=2; //video memeory started off without present, initialize will change this
    //start off our page table values
  }
  pTable2[0]=0x400000; //second page in the
  pDirectory[0]=((unsigned int)pTable) | 3; // enables r/w, presence, and supervisor
  pDirectory[1]=((unsigned int)pTable2) | 0x83; //enables r/w, PSE, supervisor, presence - these are active mappings for our kernel to use
  //PSE scheme allows page size flag, more than two level translation scheme
  pDirectory[33]=(unsigned int)videopTable;
//lots from osdev
  asm volatile(
    "movl %0, %%eax;"
    "movl %%eax, %%cr3;"

    "movl %%cr4, %%eax;"
    "orl $0x00000010, %%eax;"
    "movl %%eax, %%cr4;"

    "movl %%cr0, %%eax;"
    "orl $0x80000001, %%eax;"
    "movl %%eax, %%cr0;"


    :
    :"r"(pDirectory)
    :"%eax"
  );

  return;
}
