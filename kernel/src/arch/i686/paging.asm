[bits 32]

section .text

global paging_load_directory
global paging_enable

paging_load_directory:
  push ebp
  mov ebp, esp
  mov eax, [esp+8]
  mov cr3, eax
  pop ebp
  ret

paging_enable:
  push ebp
  mov ebp, esp
  mov eax, cr0
  or eax, 0x80000000
  mov cr0, eax
  pop ebp
  ret
