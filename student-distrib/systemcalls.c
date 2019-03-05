#include "systemcalls.h"
#include "filesystem.h"
#include "paging.h"
#include "terminal.h"
#include "x86_desc.h"
#include "rtc.h"
#include "lib.h"
/*
  %eax holds system call number
*/
file_op_jmp_table term_table = {terminal_open,terminal_read,terminal_write,terminal_close};
file_op_jmp_table rtc_table = {rtc_open,rtc_read,rtc_write,rtc_close};
file_op_jmp_table directory_table = {directory_open,directory_read,directory_write,directory_close};
file_op_jmp_table file_table = {file_open,file_read,file_write,file_close};

int pcb_occupants[6] = {0,0,0,0,0,0};
int process_previous[6] = {0,0,0,0,0,0};

/* int32_t execute(const uint8_t *cmd)
 * Inputs: cmd to be executed
 * Return Value: -1 if failed
 * Function: executes the program*/
int32_t execute(const uint8_t *cmd){ 
  cli();
  int i;
  uint8_t file_name[ARG_SIZE+1];
  uint8_t args[ARG_SIZE+1];
  int pid = -1;

  //invalid input check
  if(cmd == NULL)
  {
    printf("invalid cmd input into execute");
    return -1;
  }

  //parse input
  parse(cmd,file_name,args);
  //check file validity
  int curr_inode_num = check_valid(file_name);

  //check invalid I node number
  if(curr_inode_num==-1){
    printf("curr i node number is invalid | ");
    return -1;
  }

  //find first empty slot in PCB to assign as active for your current process
  for(i=0;i<6;i++){
    if(pcb_occupants[i]==0){
      pid = i;
      pcb_occupants[i]=-1;
      process_previous[i] = terminal[active_terminal].process_num; //this tracks the previous process associated with each terminal so that we can halt correctly
      terminal[active_terminal].process_num = i;
      break;
    }
  }
	// [0, 0, 0, 1]

  //if passed through entire loop where all PCB slots are taken, PID will still be -1, therefore we are running max processes
  if(pid==-1){ //more than 6 processes
    printf("too many processes, ");
    return -1;
  }


  //set up paging
  pDirectory[32]=(EIGHTMB+(FOURMB*pid))|0x87; //0x87 permissions for program loading

  //tlb flush- change cr3
  asm volatile (
                "movl %%cr3, %%ebx;"
                "movl %%ebx, %%cr3;"
                :
                :
                :"%ebx"
  );

  //get size of file from the inode index and looking it up in our global inode table then read data from the entire file
  int size = g_index_nodes[curr_inode_num].file_size;
  read_data(curr_inode_num,0,(uint8_t*)PROGRAM_IMAGE_OFFSET,size);
  //create and initalize PCB/Open FDs: piazza on PCB location starts https://piazza.com/class/jl733iiutpk2hj?cid=976
  pcb_t* new_execute_pcb = (pcb_t*)(EIGHTMB-(EIGHTKB*(pid+1))); // get location of our applicable pcb
  if (init_pcb(new_execute_pcb,pid) == -1)
  {
    printf("init pcb failed in system calls execute");
    return -1;
  }
  memcpy(new_execute_pcb->arguments,args,ARG_SIZE);
  //prepare context switch: must write to TSS - esp0, found in os dev https://wiki.osdev.org/Getting_to_Ring_3
  uint32_t enter = 0;
  read_data(curr_inode_num,24,(uint8_t*)&enter,4);  //get bytes 24 to 27 - the starting point within the program
  //tss.ss0 = 0x0018;	//this is the Kernel Data Segment
  tss.esp0 = EIGHTMB-(EIGHTKB)*pid-4; //gotten from piazza - (-4 for the spacing)
	asm volatile (
  		"movl %%esp, %0;"
  		"movl %%ebp, %1;"
  		: "=r" (new_execute_pcb->esp), "=r" (new_execute_pcb->ebp));  
  
  sti();
  go_to_ring_three(enter,new_execute_pcb);

  //iret then return
  return -1;

}

/**
 *  Terminates a process, returning the specified value to its parent process. The system call handler
 * itself is responsible for expanding the 8-bit argument from BL into the 32-bit return value to the
 * parent programs execute system call.
 * Be careful not to return all 32 bits from EBX. This call should never return to the call
 * Input: status - 8-bit status from execute function
 * Output: 32-bit status from execute function
 */
int32_t halt(uint8_t* status) {
  //cli();
	pcb_t* child_pcb = get_pcb();
  int pid;
	//pcb_t* parent_pcb;		//TODO get parent pcb from child
  uint32_t ret_status = (uint32_t)status;		// cast status to 32-bit return val
  	// Need to clear out all values for our child:
	// clear child file_descriptor_array
	int i;
	for(i = 2; i < PCB_FILE_DESCRIPTOR_ARR_SIZE; i++) {
		// if file is open, need to close it before we halt
		if(child_pcb->file_descriptor_array[i].flags == 1) {
			close( i );
		}
		child_pcb->file_descriptor_array[i].flags = OUT_OF_USE;
		child_pcb->file_descriptor_array[i].descriptor_op_ptr = NULL;
	}
	// clear any other child descriptors
	//child_pcb->process_num = -1;
  int process_term_num = -1;
  //find term with process - we need this for the case that the 
  //thing being halted is not at the top of the stack (child_pcb)
  //or in the current terminal
  for(i=0;i<3;i++){
    if(terminal[i].process_num == child_pcb->process_num){
      //printf("i: %d ",i);
      process_term_num = i;
      break;
    }
  }
	// if we are trying to halt parent (has no parent), then ???
	// halt process -> process now free for someone else to use
	// set (current) pid to parent pid
  //TODO: NEED to fix the first terminals exit system with incorrect PID being 1 instead of 0 which it should be
	pid = terminal[process_term_num].process_num;
  pcb_occupants[pid] = OUT_OF_USE;
  //printf("process halting: %d , previous: %d ",pid, process_previous[pid] );
  //TODO
  //these pids are associated with the first three terminals that are initialized on bootup
  if(pid == 0 || pid == 1 || pid == 2){ //piazza stated we should just restart in this case: https://piazza.com/class/jl733iiutpk2hj?cid=1043
    printf("Restarting Shell\n");
    execute((uint8_t*)"shell");
  }
  terminal[process_term_num].process_num = process_previous[pid];
  //have to update pid again here so we reload the previous page associated with this process (stored in terminals previous_process_num)
  pid = terminal[process_term_num].process_num; 
	// reload page because we changed things
	//set up paging
	pDirectory[32] = (EIGHTMB + (FOURMB*(pid))) | 0x87; //0x87 permissions for program loading - had to get rid of -1, now we have a correct pid value to return to 
  //previous page
	//tlb flush- change cr3
asm volatile (
                "movl %%cr3, %%ebx;"
                "movl %%ebx, %%cr3;"
                :
                :
                :"%ebx"
  );

	// set tss to parent
	uint32_t parent_tss_esp0 = child_pcb->esp;
	tss.esp0 = parent_tss_esp0;
	//

	/*
	 * Switch back to parent on kernel stack
	 * Pushes ESP, return status, and
	 * ASM linkage see: http://gcc.gnu.org/onlinedocs/gcc-4.7.2/gcc/Extended-Asm.html
	*/
 // sti();
  asm volatile (
		"movl %0, %%esp;"						//restore ESP,EBP and prepare to return status
		"movl %1, %%ebp;"
		"movl %2, %%eax;"
    "jmp ret_from_execute;"
		: : "r" (child_pcb->esp), "r" (child_pcb->ebp), "r"(ret_status) : "%eax");

	return -1;

}


/* int32_t open(const uint8_t* filename)
 * Inputs: file name to open
 * Return Value: -1 if failed
 * Function: closes the current fd*/
int32_t open(const uint8_t* filename) {
  file_op_jmp_table* table_ptr;
  uint32_t index_node_num = 0; //only changes for files
  uint32_t flags = 1; //set this to in use
  uint32_t file_pos = 0; //start at 0
  int loc= -1; //fd index for new fd
  int i;

  // check invalid input 
  if (*filename == '\0'||filename==NULL){
    printf("invalid filename input to system calls open");
    return -1;
  }

  //get PCB for this call -
    pcb_t * this_pcb = NULL;
  this_pcb = get_pcb();
  dentry_t temp_dentry;
  //printf("filename: %s",filename);
  if(read_dentry_by_name(filename, &temp_dentry)==-1){
    printf("read dentry by name failed : system calls open");
    return -1;
  }

  uint32_t type = temp_dentry.type;
  switch(type){ //0-RTC, 1-directory, 2-reg file
    case(0):
      table_ptr = &rtc_table;
      file_pos=-1;
      break;
    case(1):
      table_ptr = &directory_table;
      break;
    case(2):
      table_ptr = &file_table;
      index_node_num=temp_dentry.inode_num;
      break;
    default:
      //printf("system calls open : default case in switch statement");
      return -1;
  }
  //find a fd with zero flag
  for(i=0;i<8;i++){
    if(!this_pcb->file_descriptor_array[i].flags){
      loc = i;
      break;
    }
  }
  if(loc==-1){ //can't open more than 7 files
    return -1;
  }
  populate_pcb(this_pcb,loc,flags,index_node_num,table_ptr,file_pos);

  this_pcb->file_descriptor_array[loc].descriptor_op_ptr->open(filename);//actual open

  return loc;
}

/* int32_t close(int32_t fd)
 * Inputs: file descriptor
 * Return Value: -1 if failed
 * Function: closes the current fd*/
int32_t close(int32_t fd) {
  pcb_t * this_pcb = NULL;
  this_pcb = get_pcb();
  if(fd==STDIN){
    this_pcb->file_descriptor_array[fd].descriptor_op_ptr=&term_table;
  }
  if((fd<FD_MIN)||(fd>FD_MAX)||(this_pcb->file_descriptor_array[fd].flags==OUT_OF_USE)||(fd==STDOUT)||(fd==STDIN)) {
    printf("system calls close failed");
    return -1;
  }
  //printf("term close: %x ",terminal_close);
  //printf("addr: %d",fd);
  this_pcb->file_descriptor_array[fd].flags=OUT_OF_USE;
  return this_pcb->file_descriptor_array[fd].descriptor_op_ptr->close(fd);
}

/* int32_t read(int32_t fd, void * buf, int32_t nbytes)
 * Inputs: file descriptor, buffer to write to, number of bytes to be read
 * Return Value: -1 if failed, otherwise the number of bytes read
 * Function: reads number of bytes specified from buffer to fd*/
int32_t read(int32_t fd, void * buf, int32_t nbytes){
  //should be stdin
  pcb_t * this_pcb = NULL;
  this_pcb = get_pcb();
  if((this_pcb->file_descriptor_array[fd].flags==OUT_OF_USE)||(buf==NULL)||(fd<FD_MIN)||(fd>FD_MAX)||(fd==STDOUT)) {
    printf("system calls read failed");
    return -1;
  }
  //hardcoded cause terminal read isn't being passed in correctly for some reason
  if(fd==STDIN){
    this_pcb->file_descriptor_array[fd].descriptor_op_ptr=&term_table;
  }

  return this_pcb->file_descriptor_array[fd].descriptor_op_ptr->read(fd,(char*)buf,nbytes);
}

/* int32_t write(int32_t fd, const void* buf, int32_t nbytes)
 * Inputs: file descriptor, buffer to write to, number of bytes to be written
 * Return Value: -1 if failed, otherwise the number of bytes written
 * Function: writes number of bytes specified from buffer to fd*/
int32_t write(int32_t fd, const void* buf, int32_t nbytes){

  pcb_t * this_pcb = NULL;
  this_pcb = get_pcb();

  if((this_pcb->file_descriptor_array[fd].flags==OUT_OF_USE) || (fd==STDIN) || (buf==NULL)||(fd<FD_MIN)||(fd>FD_MAX)) {
    printf("system calls write failed");
    return -1;
  }
  return this_pcb->file_descriptor_array[fd].descriptor_op_ptr->write(fd,buf,nbytes);
}

int32_t getargs(uint8_t * buf, int32_t nbytes){
  pcb_t * this_pcb = get_pcb();
  if(this_pcb->arguments[0]=='\0'){
    printf("No arguments ");
    return -1;
  }
  memcpy(buf,this_pcb->arguments,ARG_SIZE);
  //puts(buf);
  return 0;
}
/*
 * Maps the text-mode video memory into user space at a pre-set virtual address. 
*/

int32_t vidmap( uint8_t** screen_start ) {
	// check for location validity
	// See mp3 doc section 6.1.5 for valid range 
	if(screen_start == NULL || (uint32_t)screen_start < ONE_TWENTY_EIGHT_MB ||
		(uint32_t)screen_start > (ONE_TWENTY_EIGHT_MB + FOURMB*(active_terminal + 1))) {
		return -1; 
	}
  int i;
  int process_term_num = -1;
  pcb_t * child_pcb = get_pcb();
  
  for(i=0;i<3;i++){
    if(terminal[i].process_num == child_pcb->process_num){
      //printf("i: %d ",i);
      process_term_num = i;
      break;
    }
  }
  pDirectory[33+process_term_num] = (unsigned int)videopTable|7; //each terminal has its own video memory page
	videopTable[0] = (video_memory_buffer_loc+(process_term_num))|7; //map page table to single physical address

	// tlb flush- change cr3
	asm volatile(
		"movl %%cr3, %%eax;"
		"movl %%eax, %%cr3;"
    :
    :
    :"%eax"
		); 

  *screen_start = (uint8_t*)ONE_TWENTY_EIGHT_MB + FOURMB*(process_term_num + 1);
	return (int32_t)&(*screen_start); 
}

/* void parse(const uint8_t* cmd, uint8_t *file, uint8_t* arg )
 * Inputs: command from typed execute input, file from kernel execute function call, arguments after first part of typed in command
 * Return Value: none
 * Function: goes through the file input and gets the file name, the command and its arguments it is passed in with.*/
void parse(const uint8_t* cmd, uint8_t *file, uint8_t* arg ){
  int i;
  int cmd_length=0;
  int max = ARG_SIZE+1;
  int cmd_first = 0;

  /* parse args */

  //invalid input check
  if(cmd == NULL || file == NULL || arg == NULL)
  {
    printf("parse function inputs invalid: NULL ");
    return;
  }
  
  //set up for parsing
  arg[ARG_SIZE+1] = '\0';
  file[ARG_SIZE+1] = '\0';
  for(i = 0; i < max; i++)
  {
    arg[i] = 0;
  }
  for(i = 0; i < max; i++)
  {
    file[i] = 0;
  }
  i = 0;
  while((cmd[i] != ' ') && (cmd[i] != '\0'))
  {
     cmd_length++;
     i++;
  }
  //cmd_length = cmd_length + 1;
  i = 0;
  while(cmd[i] != '\0')
  {
     if((cmd[i] == ' ') && (cmd_first == 0))
     {
       cmd_first = 1;
     }
     else if( cmd_first == 1)
     { 
       arg[i - cmd_length-1] = cmd[i];
     }
     else
     {
       file[i] = cmd[i];
     }
     i++;
  }

}

/* int32_t check_valid(uint8_t* fname)
 * Inputs: the file name
 * Return Value: inode number that is associated with the file to be run, or negative 1 if invalid
 * Function: goes and check if the directory entry is valid and reads in the directory entry into an object, then it
 * get the first 4 bytes by rading the data from the corresponding inode which signifies if it is an executable file
 * by checking if the first four bytes are DELETE, E, L, F */
int32_t check_valid(uint8_t* fname){
  dentry_t test_valid;
  char buf[4];
  int l;

  // check invalid input
  if(fname == NULL)
  {
    printf("check valid input filename is null ");
    return -1;
  }

  //fill buffer with 0's
  for(l = 0; l < 4; l ++){
    buf[l] = 0;
  }
  int bytes_read=0;
  if(read_dentry_by_name(fname,&test_valid)==-1){
    //printf("read dentry by name failed ");
    return -1;
  }

  //each executable has these four bytes: 0x7F, 0x45, 0x4C, 0x46 : ELF
  //check to see if this file has at least 4 bytes to check valid
  bytes_read = read_data(test_valid.inode_num, 0, (uint8_t*)buf, 4);
  if(bytes_read!=4){
    //printf("4 bytes not read ");
    return -1;
  }

  //now check if executable:
  //printf("inode_num: %d ",test_valid.inode_num);
 if((buf[0]!=DELETE) && (buf[1]!=E) && (buf[2]!=L) && (buf[3]!=F)){
   //printf("magic executable values not found: %s ", buf);
   return -1;
 }

 return test_valid.inode_num;
}

/* int init_pcb(pcb_t* new_pcb, uint32_t pid)
 * Inputs: PCB struct to be initialized, PID is our process ID number
 * Return Value: 0 if success, -1 if fail
 * Function: initalizes pcb structure
 */
int init_pcb(pcb_t* new_pcb, uint32_t pid){
  if(new_pcb == NULL)
  {
    //printf("init pcb input new_pcb is null");
    return -1;
  }

  int i;
  file_descriptor_t new_fd;
  new_fd.descriptor_op_ptr=&term_table;
  new_fd.inode_num=0;
  new_fd.file_pos=0;
  new_fd.flags=1;
  //set first two as stdin/stdout
  new_pcb->file_descriptor_array[0]=new_fd;
  new_pcb->file_descriptor_array[1]=new_fd;


  for(i=2;i<8;i++) {
    file_descriptor_t loop_fd;
    loop_fd.descriptor_op_ptr=NULL;
    loop_fd.inode_num=0;
    loop_fd.file_pos=0;
    loop_fd.flags=OUT_OF_USE;
    new_pcb->file_descriptor_array[i]=loop_fd;
  }
  new_pcb->process_num=pid;
  return 0;
}

/*"To get at each process’s PCB, you need only AND the process’s ESP
 * register with an appropriate bit mask to reach the top of its 8 kB
 * kernel stack, which is the start of its PCB."
 */
pcb_t* get_pcb(){
  pcb_t* esp_reg;
  uint32_t bitmask=0xFFFFE000; // top 8kB of stack
  register uint32_t esp asm ("esp");
  esp_reg=(pcb_t*)(bitmask&esp);
  return esp_reg;
}

/* void populate_pcb(pcb_t* this_pcb,uint32_t loc, uint32_t flags, uint32_t inode_num, file_op_jmp_table* table_ptr, uint32_t file_pos)
 * Inputs: pcb object to be populated, relevant info to populate
 * Return Value: none
 * Function: fills the pcb structure
 */
void populate_pcb(pcb_t* this_pcb,uint32_t loc, uint32_t flags, uint32_t inode_num, file_op_jmp_table* table_ptr, uint32_t file_pos){
  if(this_pcb == NULL)
  {
    //printf("populate_pcb input this_pcb is null");
    return;
  }

  this_pcb->file_descriptor_array[loc].flags=flags;
  this_pcb->file_descriptor_array[loc].inode_num=inode_num;
  this_pcb->file_descriptor_array[loc].descriptor_op_ptr=table_ptr;
  this_pcb->file_descriptor_array[loc].file_pos=file_pos;
  return;
}

/* void save_and_switch_term
 * Input: active terminal (0,1,2)
 * Output: N/A
 * Functionality: stores the video mem of the active terminal 
 */
// this helper function saves all the data associated with a given terminal number then 
void save_and_switch_term(int new_active)
{
    if(new_active == active_terminal) //change to same terminal
    { 
        return;
    }
    //save cursor location for previous terminal 

    //clear();
    memcpy((void*)terminal[active_terminal].video_memory_storage_location, (void*)VIDEO, FOUR_KB);  //load the current physical video memory into old term's storage buffer
    //after bootup: check if the terminal we're switching to has been opened
    memcpy((void*)VIDEO, (void*)terminal[new_active].video_memory_storage_location, FOUR_KB); //load new term's buffer into physical video memory 
    
    terminal[active_terminal].y_position = screen_y;
    terminal[active_terminal].x_position = screen_x;
    //set cursor location for new terminal
    screen_y = terminal[new_active].y_position;
    screen_x = terminal[new_active].x_position;
    
    update_cursor(screen_x,screen_y);
    active_terminal=new_active;
}


