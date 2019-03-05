#include "filesystem.h"
#include "lib.h"
#include "systemcalls.h"
//our filesystem follows the helpful diagrams in the documentation

/*to initialize we put in the base address of the file system and create
 * our boot block, dentries, and index nodes
 *
*/
/*
int i=0;
dentry_t* test_dentry;
index_node_t* test_index_node;
//index_node_t * test_inode;
while(read_dentry_by_index(i,test_dentry)!=-1){ //read through the entire directory
	if(strlen(test_dentry->name)!=0){ //only if non-empty name
	char name[33]; //32+1 for the null character
	strncpy((char*)name,(char*)test_dentry->name,32);
	name[32]='\0';
	printf("File Name: %d ", i);
	puts(name);
	printf(" ........ File size: ");
	test_index_node=test_dentry->inode_num+g_index_nodes;
  printf("%d B", test_index_node->file_size);
	putc('\n');
	}
	i++;

}
*/
/*
 dentry_t test_dentry;
  index_node_t * test_inode;

  if(read_dentry_by_name((uint8_t*)"shell",&test_dentry)!=-1){ //try created
    int offset_inode = test_dentry.inode_num;
    test_inode=g_index_nodes+offset_inode; //gives us the location in memory of this file's inode
    uint8_t buf[test_inode->file_size]; //buffer for our file contents
    int copied = read_data(offset_inode,0,buf,test_inode->file_size); //test out read data
    if(copied>0){
      int i;
      for(i=0;i<(test_inode->file_size);i++){
        printf("%c",buf[i]);
      }
      printf("File Name: ");//
      puts(test_dentry.name);
      printf(" ........ Bytes copied: %d\n",copied);
    } else {
      printf("copied nothing");
    }
  }
  */
int index;
void filesystem_init(int fsys_address) {
 //store the fsys in contigious memory spaces as in the diagram
 //absolute block numbers - start with boot block
 if(fsys_address<=0){
   printf("Invalid file system starting address\n");
   return;
 }
 index = 0;
 boot_block_t* b_block = (boot_block_t*)fsys_address;
 index_node_t* index_nodes = (index_node_t*)(fsys_address+BOOT_BLOCK_SIZE);
 dentry_t* directory_entries = (dentry_t*)(fsys_address+DENTRY_OFFSET_BB); //this lets us to refer to num_dir_entries
 //without using the boot_block struct
 //in case we need these values if they change, store globally
 g_b_block = b_block;
 g_index_nodes = index_nodes;
 g_directory_entries = directory_entries;
 return;
}
/* int32_t read_dentry_by_index(unit32_t index, dentry_t* dentry)
 * Inputs: file index
 * Return Value: -1 on fail, 0 on success
 * Function: gives directory entry */
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry) {
  if(index<0||index>16||dentry==NULL){
    return -1;
  }
  memcpy(dentry,(void *)&g_directory_entries[index],sizeof(dentry_t));
  return 0;
}
/* int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
 * Inputs: inode =  file index node, offset = offset from which read should start, buf = data storage, length = size of byte data
 * Return Value: -1 on fail, bytes_read on success
 * Function: fills buffer with file data */
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length) {
  int inode_size=g_index_nodes[inode].file_size;
  int offset_bytes=inode_size-offset;
  int off = offset % BOOT_BLOCK_SIZE;
  uint32_t offset_div = offset / BOOT_BLOCK_SIZE;	//correct block

  uint8_t * addr;
  int bytes_read=0;
  int i=0;
  if(inode_size<=offset){
    return 0;
  }
  if((length+offset)>inode_size){
    length=offset_bytes;
  }
  for(i=0; !(bytes_read >= inode_size) || !(bytes_read >= length);i++){ //weird read data corner case had to fix
    addr=(uint8_t *)g_b_block+BOOT_BLOCK_SIZE*(g_b_block->num_index_nodes+1); //beginning of data blocks
    addr+=(g_index_nodes[inode].data_block[i+offset_div])*BOOT_BLOCK_SIZE; //specific data block at this inode
    //printf("addr loop %d: %d",i,addr);
    int read_entire = (BOOT_BLOCK_SIZE-off);
    uint8_t * read_begin = addr+off;
    int bytes_remaining = (length+off)-bytes_read;
    if(bytes_remaining < BOOT_BLOCK_SIZE){ //in our last block
      memcpy(buf+bytes_read,read_begin,(length-bytes_read));
      bytes_read+=(length-bytes_read);
      break;
    }
    else {
      //printf("hello2");
      memcpy(buf+bytes_read,read_begin,read_entire);
      bytes_read+=read_entire; //in this case we read from the entire block minus the offset
      off=0; //offset taken care of, not on a block per block basis
    }
  }
  return bytes_read;
}
/* int32_t read_dentry_by_name (const uint8_t*fname, dentry_t* dentry)
 * Inputs: file name
 * Return Value: -1 on fail, 0 on success
 * Function: directory by name*/
int32_t read_dentry_by_name (const uint8_t*fname, dentry_t* dentry) {
  int i;
  if(fname==NULL || dentry==NULL || (strlen((int8_t*)fname)>NAME_SIZE)){
    printf("read dentry by name inputs invalid ");
    return -1;
  }
  else {
    for(i=0; i<64;i++){
      if(strncmp((char*)fname, (char*)g_directory_entries[i].name,32)==0){
        memcpy(dentry,(void *)&g_directory_entries[i],sizeof(dentry_t));
        return 0;
      }
    }
  return -1;
  }
}
/* void dir_read()
 * Inputs: none
 * Return Value: none
 * Function: reads and prints each file name*/
int32_t directory_read(int32_t fd, void* buf, int32_t nbytes) {
dentry_t test_dentry;
pcb_t * this_pcb = get_pcb();
uint32_t index = this_pcb->file_descriptor_array[fd].file_pos;
if(read_dentry_by_index(index,&test_dentry)==-1){
  return 0;
}
strncpy(buf,(char*)test_dentry.name,NAME_SIZE);
this_pcb->file_descriptor_array[fd].file_pos++;
return NAME_SIZE;
}

/* int32_t directory_close(int32_t fd)
 * Inputs: file descriptor
 * Return Value:
 * Function: closes dir*/
int32_t directory_close(int32_t fd){
  return 0;
}
/* int32_t directory_open(const uint8_t* filename)
 * Inputs: file descriptor
 * Return Value: 0
 * Function: opens dir*/
int32_t directory_open(const uint8_t* filename){
  return 0;
}
/* int32_t directory_write(int32_t fd, const void* buf, int32_t nbytes)
 * Inputs: file descriptor, buffer to be written, number of bytes to be written
 * Return Value: -1 if failed
 * Function: writes and prints file contents*/
int32_t directory_write(int32_t fd, const void* buf, int32_t nbytes){
  return 0;
}
/* void file_read()
 * Inputs: file descriptor, buffer to be read, number of bytes to be read
 * Return Value: -1 if failed
 * Function: reads and prints file contents*/
int32_t file_read(int32_t fd, void* buf, int32_t nbytes) {
  int32_t number_of_bytes_read = 0;
  pcb_t *current_pcb = get_pcb();
  int32_t inode_number = current_pcb->file_descriptor_array[fd].inode_num;
  int32_t length_of_curr_node = g_index_nodes[inode_number].file_size;

  //input and param checking
  if(buf == NULL)
  {
    printf("buffer in file read is empty ");
    return -1;
  }
  else if(current_pcb->file_descriptor_array[fd].flags != 1) // check if in use
  {
      printf("pcb not in use ");
      return -1;
  }
  else if(current_pcb->file_descriptor_array[fd].descriptor_op_ptr == NULL)
  {
      printf("file descriptor arrays op table pointer is null");
      return -1;
  }
  else  //proceed with read op
  {
    //read according to most fitting length
    uint32_t file_offset = current_pcb->file_descriptor_array[fd].file_pos;
    if(nbytes > length_of_curr_node)
    {
      //printf("proceeding with file read, reading n = file size bytes");
      number_of_bytes_read = read_data(inode_number,file_offset,(uint8_t*)buf, length_of_curr_node);
    }
    else
    { 
      //printf("proceeding with file read, reading n = file read input nbytes");
      number_of_bytes_read = read_data(inode_number,file_offset,(uint8_t*)buf, nbytes);
    }

    //update number of bytes read depending on the length of current node
    if(length_of_curr_node > file_offset && number_of_bytes_read == 0)
      number_of_bytes_read = nbytes;
    
    //increment file pos with num of bytes read
    current_pcb->file_descriptor_array[fd].file_pos = current_pcb->file_descriptor_array[fd].file_pos + number_of_bytes_read;
    //printf("bytes read: %d ", number_of_bytes_read);
    return number_of_bytes_read;
  }
}

/* int32_t file_close(int32_t fd)
 * Inputs: file descriptor
 * Return Value: -1 if failed
 * Function: closes the file descriptor*/
int32_t file_close(int32_t fd){
  return 0;
}
/* int32_t file_open(const uint8_t* filename
 * Inputs: filename to be opened
 * Return Value: -1 if failed
 * Function: opens the file specified in filename*/
int32_t file_open(const uint8_t* filename){
  return 0;
}

/* int32_t file_write(int32_t fd, const void* buf, int32_t nbytes)
 * Inputs: file descriptor, buffer to be written, number of bytes to be written
 * Return Value: -1 if failed
 * Function: writes the buffer to fd with the number of bytes specified*/
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes){
  return 0;
}
