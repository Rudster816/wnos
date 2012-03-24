; main.asm
;
; Copyright (c) 2012, Riley Barnett
; All rights reserved.

; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are met: 

; 1. Redistributions of source code must retain the above copyright notice, this
;    list of conditions and the following disclaimer. 
; 2. Redistributions in binary form must reproduce the above copyright notice,
;    this list of conditions and the following disclaimer in the documentation
;    and/or other materials provided with the distribution. 

; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
; ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
; WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
; DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
; ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
; (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
; ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
; (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
; SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

; The views and conclusions contained in the software and documentation are those
; of the authors and should not be interpreted as representing official policies, 
; either expressed or implied, of the FreeBSD Project.

%include "elf.inc"
%include "multiboot.inc"

%define utf16(x) __utf16__(x)

org 1000h
bits 16

Entry:
	jmp Main

; Variables:						; Easier to read compared to putting all of them at the end

%define BytesPerSector 0x7C0B
%define SectsPerCluster 0x7C0D
%define ReservedSectors 0x7C0E
%define NumberOfFats 0x7C10
%define SectorsPerFatBig 0x7C24
%define RootDirStartCluster 0x7C2C
%define BootDrive 0x7C40

LoadingMsg db 'Loading Kernel...',0xD, 0xA, 0
CantFindMsg db 'Could not find kernel. ',  0xD, 0xA, 0
InvalidImageMsg db 'The kernel is not in a compatible format. ',  0xD, 0xA, 0
AoutMsg db 'Loading AOUT kludge kernels not supported!', 0xD, 0xA, 0
NoLongMsg db 'This CPU does not support long mode!', 0xD, 0xA, 0
OutOfMemMsg db 'Not enough memory to map kernel address space!', 0xD, 0xA, 0
RebootMsg db 'Press any key to reboot...', 0xD, 0xA, 0

KernelName dw utf16('kernel.elf64'), 0
DoubleWord dd 0
KernelEntry dd 0
MbInfoAddr dd 0
MbHeaderFlags dd 0

GDT_PTR:
	GDT_Limit dw 0
	GDT_Base dd 0
Flat_GDT:
	dq 0							; Null selector
	dw 0xFFFF						; 32 Bit Code selector
	dw 0
	db 0
	db 0x9A
	dw 0xCF
	dw 0xFFFF						; 32 Bit Data selector
	dw 0
	db 0
	db 0x92
	dw 0xCF	
	dw 0xFFFF						; 16 Bit Code selector
	dw 0
	db 0
	db 0x9A
	dw 0x8F
	dw 0xFFFF						; 16 Bit Data selector
	dw 0
	db 0
	db 0x92
	dw 0x8F

GDT_Long:
	dq 0
	dq 0x0020980000000000
	dq 0x0000900000000000

DEBUGBREAK:
	mov esp, 0xDEADC0DE
	cli
	hlt	

%include "fat32driver.asm"
%include "elfloader.asm"

Main:
	mov sp, 0x1000					; Setup new stack. Limited to 1280 bytes before overwriting BDA\IVT
	xor ax, ax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	
	mov si, LoadingMsg
	call PrintString

	xor eax, eax
	mov ax, [BytesPerSector]
	xor cx, cx
	mov cl, [SectsPerCluster]
	mul cx
	mov [BytesPerCluster], eax
	bsf ecx, eax
	mov [ClusterDivShift], cl
	sub eax, 1
	mov [ClusterMask], eax
	
	xor esi, esi
	mov si, KernelName
	call FileOpen

	call EnableA20

	call LoadKernel

	mov ax, 0x7000	
	mov es, ax
	xor ebp, ebp	
	xor edx, edx
	
	call GetMemoryInfo
	jc .MemInfoError
	or edx, MULTIBOOT_INFO_MEMORY
	mov [es:ebp + multiboot_info.mem_lower], eax
	mov [es:ebp + multiboot_info.mem_upper], ebx
	.MemInfoError:
	
	call GetMemoryMap
	jc .MmapError
	or edx, MULTIBOOT_INFO_MEM_MAP
	push edx
	mov eax, 28
	mul ebx
	pop edx
	mov [es:ebp + multiboot_info.mmap_length], eax
	mov DWORD [es:ebp + multiboot_info.mmap_addr], 0x70400
	.MmapError:
	
	mov [es:ebp + multiboot_info.flags], edx
	mov eax, [es:0x1000]
	mov [es:ebp + multiboot_info.elf_phdr_length], eax
	mov DWORD [es:ebp + multiboot_info.elf_phdr_addr], 0x71004
	
	cmp BYTE [ElfType], ELFCLASS64
	je EnterLong
	
	cli								; Switch to protected mode and jump to the kernel
	mov ax, 39						; 4 entries * 8 bytes - 1
	mov [GDT_Limit], ax
	xor eax, eax
	mov ax, Flat_GDT
	mov [GDT_Base], eax
	lgdt [GDT_PTR]
	mov eax, cr0					; Enter Protected 32 Bit mode
	or al, 1
	mov cr0, eax
	jmp 0x08:KernelStart32			; Long jump to protected mode
	
Error:
	call PrintString
	mov si, RebootMsg
	call PrintString
	xor ax, ax
	int 0x16						; Wait for keypress
	int 0x19						; Reboot
	
PrintString:
	push eax
	push ebx
	
	.PutChar:
		lodsb
		or al,al
		jz .Done
		mov ah,0eh
		mov bx,07h
		int 10h
		jmp .PutChar
	
	.Done:
		pop ebx
		pop eax
		ret
		
EnableA20:							; TODO: Check for error\add more methods\etc
	pushad
	mov ax, 0x2401
	int 0x15
	popad
	ret
	
GetMemoryInfo:
	push ecx
	push edx
	
	mov ah, 0x88					; Get high memory count the same way GRUB does
	int 0x15
	jc .Error
	test ax, ax
	jz .Error
	cmp ah, 0x86
	je .Error
	cmp ah, 0x80
	je .Error
	and eax, 0xFFFF
	shl eax, 10
	mov ebx, eax
	
	xor eax, eax					; Get low memory count the same way GRUB does
	int 0x12
	jc .Error
	test ax, ax
	jz .Error
	shl eax, 10
	
	clc
	pop edx
	pop ecx
	ret
	
	.Error:
		stc
		pop edx
		pop ecx
		ret
	
GetMemoryMap:
	push es
	push edx
	push ecx
	push ebp
	
	mov ax, 0x7000
	mov es, ax
	mov di, 0x404
	xor ebx, ebx
	xor ebp, ebp
	
	.PlaceEntry:
		mov DWORD [es:edi - 4], 24	; Size of entry for Multiboot compatablity
		mov DWORD [es:edi + 20], 1	; Force ACPI 3.x compatablity 
		mov edx, 0x534D4150
		mov eax, 0xE820
		mov ecx, 24
		int 0x15
		jc .CarrySet
		cmp eax, 0x534D4150
		jne .Error
		add ebp, 1
		add edi, 28
		mov eax, 0xE820
		mov ecx, 24
		test ebx, ebx
		jnz .PlaceEntry
	
	.Done:
		sub edi, 0x404
		mov eax, edi
		mov ebx, ebp
		pop ebp
		pop ecx
		pop edx
		pop es
		clc
		ret
		
	.CarrySet:
		cmp edi, 0x404
		je .Error
		jmp .Done
	
	.Error:
		pop ebp
		pop ecx
		pop edx
		pop es
		stc
		ret
		
EnterLong:
	cli
	mov WORD [GDT_Limit], 23
	xor eax, eax
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
	
	jmp 0x8:KernelStart64
	
bits 32
KernelStart32:
	cli
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov ss, ax
	
	mov eax, 0x2BADB002
	mov ebx, 0x70000				; Multiboot Info always stored there 
	mov edi, [KernelEntry]
	jmp edi
	
bits 64
KernelStart64:
	cli
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	
	mov rax, 0x2BADB002
	mov rbx, 0x70000
	mov rdi, [KernelEntry]
	jmp rdi
	
	