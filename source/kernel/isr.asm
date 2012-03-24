%macro PUSHAQ 0
	push r15
	push r14
	push r13
	push r12
	push r11
	push r10
	push r9
	push r8
	push rbp
	push rsi
	push rdi
	push rdx
	push rcx
	push rbx
	push rax
%endmacro
	
%macro POPAQ 0
	pop rax
	pop rbx
	pop rcx
	pop rdx
	pop rdi
	pop rsi
	pop rbp
	pop r8
	pop r9
	pop r10
	pop r11
	pop r12
	pop r13
	pop r14
	pop r15
%endmacro

bits 64

global idt_table
global isr_table
global idt_set
global isr_template_error
global isr_template_noerror

extern kpanic

section .data

idtr_val:
	.Limit: dw 4096
	.Base: dq idt_table

section .bss

align 16
idt_table: resb 4096

align 16
isr_table: resb 24576

section .text

idt_set:
	mov rdi, idtr_val
	lidt [rdi]
	ret

isr_template_noerror:
	PUSHAQ	
	mov rdi, rsp
	xor rsi, rsi
	nop
	mov rax, 0xAAAAAAAAAAAAAAAA
	mov si, 0xCC
	call rax
	POPAQ
	iretq
	times 25 db 0

isr_template_error:	
	push rax
	mov rax, [rsp + 8]
	mov [rsp - 112], rax
	pop rax
	add rsp, BYTE 8
	PUSHAQ
	mov rdi, rsp
	xor rsi, rsi
	mov si, 0xCC
	mov rdx, [rsp - 8]
	mov rax, 0xAAAAAAAAAAAAAAAA
	call rax
	POPAQ
	iretq
	times 5 db 0
	

	