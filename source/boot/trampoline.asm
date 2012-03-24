bits 16
org 0x1000

ap_trampoline:
	cli
	xor eax, eax
	mov ds, ax
	
	mov WORD [GDT_Limit], 23
	mov ax, GDT_Long
	mov DWORD [GDT_Base], eax

	mov eax, 10100000b
	mov cr4, eax
	mov edx, 0x50000
	mov cr3, edx
	mov ecx, 0xC0000080
	rdmsr
	or eax, 0x00000100
	wrmsr
	
	mov ebx, cr0
	or ebx, 0x80000001
	mov cr0, ebx
	
	lgdt [GDT_PTR]
	
	jmp 0x8:ap_start_64
	
times 4 db 0

bits 64
ap_start_64:
	cli
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	nop
	mov rsp, 0xCCCCCCCCCCCCCCCC
	times 6 nop
	mov rax, 0xAAAAAAAAAAAAAAAA
	call rax
	cli
	hlt

dd 0
	
GDT_PTR:
	GDT_Limit dw 0
	GDT_Base dq 0
	
dd 0
dw 0
	
GDT_Long:
	dq 0
	dq 0x0020980000000000
	dq 0x0000900000000000
	dq 0
	dq 0
	dq 0
	