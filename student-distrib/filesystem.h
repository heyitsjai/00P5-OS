#ifndef FILE_SYS_H
#define FILE_SYS_H
#include "types.h"
#define DATA_BLOCK_SIZE 1023 //int is 4 bytes, need 4092 : 4*1023
#define DIRECTORY_SIZE 128 //in bytes
#define NAME_SIZE 32
#define BOOT_BLOCK_SIZE 4096 //4 kB per block
#define DENTRY_OFFSET_BB 52+4+4+4
//https://elixir.bootlin.com/linux/v4.12-rc7/source/include/linux/dcache.h#L84 - dentry
//follows doc for specification
typedef struct {
  char name[NAME_SIZE];
  int type; //0-RTC, 1-directory, 2-reg file
  int inode_num; //displacement for associated inode
  int reserved[6]; //24B = 4*6
} dentry_t;

typedef struct {
  int file_size; //in bytes
  int data_block[DATA_BLOCK_SIZE];
} index_node_t;

//decided to keep the boot block in its own struct
typedef struct {
  int num_dir_entries;
  int num_index_nodes;
  int num_data_blocks;
  int reserved[13]; //52B = 4*13
  dentry_t* dentry_list;//stores a linked list of dentry structs
} boot_block_t;
//
boot_block_t* g_b_block;
index_node_t* g_index_nodes;
dentry_t* g_directory_entries;

void filesystem_init(int fsys_address);
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);
int32_t directory_read(int32_t fd, void *buf, int32_t nbytes);
int32_t directory_close(int32_t fd);
int32_t directory_open(const uint8_t* filename);
int32_t directory_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t file_read(int32_t fd, void* buf, int32_t nbytes);
int32_t file_close(int32_t fd);
int32_t file_open(const uint8_t* filename);
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes);


#endif
