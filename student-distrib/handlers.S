.text

.align 4
.globl rtc_interrupt

.align 4
.globl PIT_interrupt

.align 4
.globl keyboard_interrupt

.align 4
.globl sys_call_interrupt

.align 4
.globl go_to_ring_three

.align
.globl ret_from_execute
# jmp table for system calls, they are well-defined to be
# numbered in this order
system_call_jmp:
  .long halt, execute, read, write, open, close, getargs, vidmap
sys_call_retval:
  .long 0x0
rtc_interrupt:
  pushal
  pushfl
  call rtc_handle_interrupt
  popfl
  popal
  iret
  
PIT_interrupt:
  pushal
  pushfl
  call PIT_handler
  popfl
  popal
  iret

keyboard_interrupt:
  pushal
  pushfl
  call keyboard_handler
  popfl
  popal
  iret
# %eax has the associated number || ebx/ecx/edx have our parameters

sys_call_interrupt:
  cmpl $8, %eax
  jg out_of_bounds
  cmpl $1, %eax
  jl out_of_bounds
# cli
  pushal #save our registers
  pushfl
  pushl %edx
  pushl %ecx
  pushl %ebx #here are our parameters from right to left
  decl %eax
 # sti
  call *system_call_jmp(,%eax,4) #long is 4 bytes
 cli
  movl %eax, sys_call_retval
  addl $12, %esp #pop our three parameters
  popfl
  popal
  movl sys_call_retval, %eax
 sti
  iret

 out_of_bounds:
  movl $-1, %eax
  iret

# os dev https://wiki.osdev.org/Getting_to_Ring_3
go_to_ring_three:

    pushl %ebp
    movl %esp, %ebp

    movl 12(%ebp), %ecx # 24 byte entry point address passed in, the enter variable
    movl %esp, (%ecx)
    movl %ebp, 4(%ecx)
    mov $0x2B, %ax    # user data segment
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    movl %esp, %eax
    pushl $0x2B       # user data segment
    pushl $0x083FFFFC  # program entry point
    pushfl             # flags
    pushl $0x23       # user code segment
    movl 8(%ebp), %ecx # one of the arguments
    pushl %ecx # other argument
    iret
ret_from_execute:
  leave
  ret
