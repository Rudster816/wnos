bits 64

global cpuid_apic
global cpuid_vendor_id
global set_cpuid

section .data
cpuid_vendor_id times 13 db 0
cpuid_apic db 0

section .text

set_cpuid:
	push rbp
	mov rbp, rsp
	push rbx
	
	call SetVendorID
	mov eax, 1
	cpuid
	shr edx, 9
	and edx, 1
	mov rdi, cpuid_apic
	mov BYTE [rdi], cl
	
	pop rbx
	pop rbp
	ret
	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;	

SetVendorID:
	mov rdi, cpuid_vendor_id
	xor eax, eax
	cpuid
	xor rax, rax
	
	.SetByte:
		mov [rdi + rax], bl
		mov [rdi + rax + 0x4], dl
		mov [rdi + rax + 0x8], cl
		shr ebx, 8
		shr edx, 8
		shr ecx, 8
		add eax, 1
		cmp eax, 4
		jnz .SetByte
	
	ret
	