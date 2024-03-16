[bits 32]

section .text

global tss_load

tss_load:
  push ebp
  mov ebp, esp
  mov ax, [ebp+8]
  ltr ax
  pop ebp
  ret
