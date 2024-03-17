[bits 32]

section .text

global restore_gp_registers
global task_return
global user_registers

restore_gp_registers:
  push ebp
  mov ebp, esp
  mov ebx, [ebp+8]
  mov edi, [ebx]
  mov esi, [ebx+4]
  mov ebp, [ebx+8]
  mov edx, [ebx+16]
  mov ecx, [ebx+20]
  mov eax, [ebx+24]
  mov ebx, [ebx+12]
  pop ebp
  ret

task_return:
  mov ebp, esp
  mov ebx, [ebp+4]
  push dword [ebx+44]
  push dword [ebx+40]

  pushf
  pop eax
  or eax, 0x200
  push eax

  push dword [ebx+32]
  push dword [ebx+28]

  mov ax, [ebx+44]
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax

  push dword [ebx+4]
  call restore_gp_registers
  add esp, 4

  iretd ; drop into userland

user_registers:
  mov ax, 0x20 ; TODO: idk if this works
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  ret
