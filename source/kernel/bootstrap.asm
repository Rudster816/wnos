bits 64

global loader
global ap_trampoline

extern kmain							; kmain is defined in start.c
 
MODULEALIGN equ  1<<0					; align loaded modules on page boundaries
MEMINFO     equ  1<<1					; provide memory map
FLAGS       equ  MODULEALIGN | MEMINFO	; this is the Multiboot 'flag' field
MAGIC       equ    0x1BADB002			; 'magic number' lets bootloader find the header
CHECKSUM    equ -(MAGIC + FLAGS)		; checksum required

section .text
 
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM
 

STACKSIZE equ 0x4000				; reserve initial kernel stack space
 
loader:
    mov rsp, stack + STACKSIZE		; set up the stack
    mov rsi, rax					; Multiboot magic number
    mov rdi, rbx					; Multiboot info structure
 
    call kmain						; call kernel proper
 
    cli
.hang:
    hlt								; halt machine should kernel return
    jmp  .hang
 
section .bss
 
align 16
stack: resb STACKSIZE				; reserve 16k stack on a 16 byte boundary


section .data
ap_trampoline:
incbin "./../output/bin/trampoline.bin"