#ifndef SYS_CALLS_H
#define SYS_CALLS_H
#include "types.h"
#include "lib.h"

#define DELETE 0x7F
#define E 0x45
#define L 0x4C
#define F 0x46
#define EIGHTMB 0x800000
#define EIGHTKB 0x2000
#define FOURMB 0x400000
#define BIT_MASK        0xFFF000
#define FOURKB_HEX 0x1000
#define ARG_SIZE 32
#define PROGRAM_IMAGE_OFFSET 0x08048000 //appendix C
#define PROGRAM_IMAGE_ENTRY_POINT PROGRAM_IMAGE_OFFSET+24//This information is stored as a 4-byte unsigned integer
  //in bytes 24-27 of the executable, and the value of it falls somewhere near 0x08048000, pg 22
# define PCB_FILE_DESCRIPTOR_ARR_SIZE 8
#define STDIN 0
#define STDOUT 1
#define OUT_OF_USE 0
#define FD_MIN 0
#define FD_MAX 7
#define ONE_TWENTY_EIGHT_MB 0x8000000
#define VIDEO_MEM_START_LOC		ONE_TWENTY_EIGHT_MB + FOURMB



//pg 42 course notes gives this structure
typedef struct file_op_jmp_table{
  int32_t (*open) (const uint8_t* filename);
  int32_t (*read) (int32_t fd, void* buf, int32_t nbytes);
  int32_t (*write) (int32_t fd, const void* buf, int32_t nbytes);
  int32_t (*close) (int32_t fd);
}file_op_jmp_table;

//pg 19 documentation
typedef struct file_descriptor_t{
    file_op_jmp_table *descriptor_op_ptr;
    uint32_t inode_num;
    uint32_t file_pos;
    uint32_t flags;
}file_descriptor_t;
//pg 19 diagram as well
typedef struct pcb_t{
  file_descriptor_t file_descriptor_array[8]; //2 for stdin/stdout, 6 dynamically assigned
  //whatever needed to start or stop a process - 6.3.5
  uint32_t esp;
  uint32_t ebp;
  uint32_t process_num;
  uint8_t arguments[ARG_SIZE];
}pcb_t;


int32_t execute(const uint8_t *cmd);
int32_t halt(uint8_t* status);
int32_t open(const uint8_t* filename);
int32_t close(int32_t fd);
int32_t read(int32_t fd, void * buf, int32_t nbytes);
int32_t write(int32_t fd, const void* buf, int32_t nbytes);
int32_t getargs(uint8_t * buf, int32_t nbytes);


void parse(const uint8_t* cmd, uint8_t *file, uint8_t* arg);
int32_t check_valid(uint8_t* fname);
int init_pcb(pcb_t* new_pcb, uint32_t pid);
void sys_call_interrupt();
void go_to_ring_three(uint32_t n, pcb_t* new_execute_pcb); 
pcb_t* get_pcb();
void populate_pcb(pcb_t* this_pcb,uint32_t loc, uint32_t flags, uint32_t inode_num, file_op_jmp_table* descriptor_op_ptr, uint32_t file_pos);
void save_and_switch_term(int new_active);

#endif
