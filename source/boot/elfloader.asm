; elfloader.asm
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

RealStack dw 0
MemorySource dq 0
MemoryDestination dq 0
SourceLength dq 0
DestinationLength dq 0

ElfType db 0
ElfPhNum dw 0
ElfPhOff dd 0
ElfPhSize dw 0

LoadKernel:	
	mov [FileClusterCount], ecx
	mov eax, [BytesPerCluster]
	mul ecx
	cmp eax, 8192
	jb .DontSet8192					; We could use CMOV here, but that instruction isnt on
	mov eax, 8192					; pre i686 CPU's
	.DontSet8192:
	
	mov ecx, eax
	mov ax, 0x1000
	mov es, ax
	mov fs, ax
	xor eax, eax
	call FileRead
	
	xor ebp, ebp
	.CheckMultibootMagic:
		cmp DWORD [es:ebp], MULTIBOOT_HEADER_MAGIC
		je .FoundMagic
		add ebp, 4
		dec ecx
		jnz .CheckMultibootMagic
		jmp .InvalidImage			; We didnt find the header
		
	.FoundMagic:
		mov eax, [es:ebp + 4]		; Flags
		mov ecx, [es:ebp + 8]		; Checksum
		add ecx, MULTIBOOT_HEADER_MAGIC
		add ecx, eax
		jnz .InvalidImage			; Checksum is invalid
		mov [MbHeaderFlags], eax
		and eax, 0x10000			; Check AOUT kludge flag
		jnz .LoadAout
	
	xor ebp, ebp
	mov al, ELFMAG0					; Check ELF magic values
	cmp al, [es:ebp + EI_MAG0]		; 0x7F
	jne .InvalidImage
	mov al, ELFMAG1
	cmp al, [es:ebp + EI_MAG1]		; 'E'
	jne .InvalidImage
	mov al, ELFMAG2
	cmp al, [es:ebp + EI_MAG2]		; 'L'
	jne .InvalidImage
	mov al, ELFMAG3
	cmp al, [es:ebp + EI_MAG3]		; 'F'
	jne .InvalidImage
	mov al, [es:ebp + EI_CLASS]
	cmp al, ELFCLASS32
	je .LoadElf32
	cmp al, ELFCLASS64
	je .LoadElf64
				
	.InvalidImage:					; EI_CLASS isnt ELF32 or ELF64, so error out
		mov si, InvalidImageMsg
		call Error
		
	.NoLongMode:
		mov si, NoLongMsg
		call Error
		
	.LoadElf32:
		mov eax, [es:ebp + Elf32_Ehdr.e_entry]
		test eax, eax
		jz .InvalidImage			; Image must have an entry point
		mov [KernelEntry], eax		; Save it
		
		mov ax, [es:ebp + Elf32_Ehdr.e_phnum]
		mov [ElfPhNum], ax
		mov bx, [es:ebp + Elf32_Ehdr.e_phentsize]
		mov [ElfPhSize], bx
		mov ecx, [es:ebp + Elf32_Ehdr.e_phoff]
		mov [ElfPhOff], ecx
		mov BYTE [ElfType], ELFCLASS32
		
		call LoadElfKernel
		ret


	.LoadElf64:
		mov eax, [es:ebp + Elf64_Ehdr.e_entry]
		mov ebx, [es:ebp + Elf64_Ehdr.e_entry + 4]
		mov [KernelEntry], eax
		mov [KernelEntry + 4], ebx
		or eax, ebx
		jz .InvalidImage
		
		mov ax, [es:ebp + Elf64_Ehdr.e_phnum]
		mov [ElfPhNum], ax
		mov bx, [es:ebp + Elf64_Ehdr.e_phentsize]
		mov [ElfPhSize], bx
		mov ecx, [es:ebp + Elf64_Ehdr.e_phoff]
		mov [ElfPhOff], ecx			; Ignores the top DWORD of the file offset
		mov BYTE [ElfType], ELFCLASS64
		
		call CheckCpuid				; Make sure CPUID is supported
		jz .NoLongMode
		mov eax, 0x80000001
		cpuid
		and edx, 0x20000000			; Make sure Long Mode is supported
		jz .NoLongMode
		
		call InitPageTable
		call LoadElfKernel
		ret
	
	.LoadAout:
		mov si, AoutMsg
		call Error
		
InitPageTable:						; Inital 64 bit paging setup. Identity maps first 4GB of address space
	pushad							; It also maps the 0xFFFFFFFF00000000 -> 0xFFFFFFFFFFFFFFFF region to
	push es							; the first 4GB of physical address space
	mov ax, 0x5000
	mov es, ax
	mov cx, 0x4000
	xor di, di
	xor eax, eax
	rep stosd						; Zero out the 64kb region from 0x60000 -> 0x6FFFF inclusive
	
	mov DWORD [es:0x0], 0x51007		; PML4E for first 512GB of address space.
	mov DWORD [es:0xFF8], 0x52007	; PML4E for the last 512GB
	
	mov DWORD [es:0x1000], 0x53007	; Set 4 PDPTE entries for the first 4GB of address space
	mov DWORD [es:0x1008], 0x54007
	mov DWORD [es:0x1010], 0x55007
	mov DWORD [es:0x1018], 0x56007
	
	mov DWORD [es:0x2FE0], 0x53007
	mov DWORD [es:0x2FE8], 0x54007
	mov DWORD [es:0x2FF0], 0x55007
	mov DWORD [es:0x2FF8], 0x56007
	
	mov ebp, 0x3000
	mov ecx, 2048
	mov edx, 0x87					; The flags
	.Add2MBEntry:
		mov [es:ebp], edx
		add ebp, 8
		add edx, 0x200000			; 2MB
		dec ecx
		jnz .Add2MBEntry
	
	pop es
	popad
	ret
		
CheckCpuid:							; returns 1 if CPUID is supported, 0 otherwise (ZF is also set accordingly)
	pushfd 							; get
	pop eax
	mov ecx, eax 					; save 
	xor eax, 0x200000 				; flip
	push eax 						; set
	popfd
	pushfd 							; and test
	pop eax
	xor eax, ecx 					; mask changed bits
	shr eax, 21 					; move bit 21 to bit 0
	and eax, 1 						; and mask others
	push ecx
	popfd 							; restore original flags
	ret
	
LoadElfKernel:
	push gs
	mov dx, 0x7000
	mov gs, dx
	mov DWORD [gs:0], 0
	mov esi, 0x1004
	movzx ecx, WORD [ElfPhNum]
	mov ebx, ecx					; Store the total header count in EBX
	mov eax, [ElfPhOff]
	push eax						; Save this for later
	
	.LoadHeaders:					; ECX will have the remaining header count
		movzx eax, WORD [ElfPhSize]	
		mul ecx						; EAX now has total size of all program headers left
		cmp eax, 0xFFFF				; No matter how unlikely, we have to support more than 64Kb worth of headers
		jbe .CountResolved
		
		mov eax, 0xFFFF	
		movzx edi, WORD [ElfPhSize]
		xor edx, edx
		div edi						; EAX is header count that fits in our buffer (64kb)
		xchg eax, ecx
		mul ecx						; ECX is header count, EAX is size of all those headers
		
		.CountResolved:				; ECX will have headers to read, EAX will have number of bytes to read
			mov edx, ecx			; Save header count in EDX
			mov ecx, eax
			pop eax					; Location of e_phoff plus any headers already loaded
			mov di, 0x2000
			mov fs, di
			call FileRead

		push edx					; Save header count on stack
		mov ecx, edx				; Restore header count to ECX
		mov dx, 0x2000
		mov es, dx
		
		movzx edx, WORD [ElfPhSize]
		
		.LoadNextElfHeader:
			cmp BYTE [ElfType], ELFCLASS64
			je .LoadElf64
			cmp BYTE [ElfType], ELFCLASS32
			je .LoadElf32
			
			mov si, InvalidImageMsg
			call Error
			
			.LoadElfDone:
				add bp, dx
				add eax, edx
				dec ecx
				jnz .LoadNextElfHeader
				pop ecx				; Restore orignal header count
				sub ebx, ecx		; EBX still contains the total number of headers that haven't been read
				jz .Done			; All headers have been read
				push eax			; Push the file offset of the next header
				jmp .LoadHeaders
				
			.LoadElf64:
				call LoadElf64Phdr
				add DWORD [gs:0x1000], 1
				mov eax, [es:ebp + Elf64_Phdr.p_paddr];
				mov [gs:esi], eax
				mov eax, [es:ebp + Elf64_Phdr.p_memsz];
				mov [gs:esi + 4], eax
				add esi, 8
				jmp .LoadElfDone
			.LoadElf32:
				call LoadElf32Phdr
				add DWORD [gs:0x1000], 1
				mov eax, [es:ebp + Elf32_Phdr.p_paddr];
				mov [gs:esi], eax
				mov eax, [es:ebp + Elf32_Phdr.p_memsz];
				mov [gs:esi + 4], eax
				add esi, 8
				jmp .LoadElfDone	
	
	.Done:
		pop gs
		ret
	
	
LoadElf32Phdr:
	pushad
	mov eax, [es:ebp + Elf32_Phdr.p_type]

	mov edx, [es:ebp + Elf32_Phdr.p_paddr]
	mov eax, [es:ebp + Elf32_Phdr.p_align]
	sub eax, 1						; Create a mask for the desired alignment
	jo .NoAlignment					; sh_addaralign was 0, so no alignment was wanted
	
	not eax
	and edx, eax					; Align the address via trunacation
	.NoAlignment:
		
	mov edi, edx
	mov ebx, [es:ebp + Elf32_Phdr.p_filesz]
	mov eax, [es:ebp + Elf32_Phdr.p_offset]
	mov ecx, [es:ebp + Elf32_Phdr.p_memsz]
	call LoadFileSection
	
	popad
	ret
		
LoadElf64Phdr:
	pushad

	mov edx, [es:ebp + Elf64_Phdr.p_paddr]
	mov eax, [es:ebp + Elf64_Phdr.p_align]
	sub eax, 1						; Create a mask for the desired alignment
	jo .NoAlignment					; p_align was 0, so no alignment was wanted
	
	not eax
	and edx, eax					; Align the address via trunacation
	.NoAlignment:
		
	mov edi, edx
	mov ebx, [es:ebp + Elf64_Phdr.p_filesz]
	mov eax, [es:ebp + Elf64_Phdr.p_offset]
	mov ecx, [es:ebp + Elf64_Phdr.p_memsz]	
	call LoadFileSection
	
	.Done:
	popad
	ret
		
LoadFileSection:					; EAX is file offset, EBX is file size, ECX is memory size
	pushad							; EDI is memory destination
	mov [MemoryDestination], edi
	mov edx, 0x3000
	sub ecx, ebx
	push ecx						; Bytes to zero after file contents
	
	.StoreBytes:
		mov fs, dx
		mov ecx, ebx
		cmp ecx, 0xFFFF
		jna .LengthResolved
		mov ecx, 0xFFFF
		
		.LengthResolved:
			call FileRead		
			sub ebx, ecx
			jz .MoveBytes
			add dx, 0x1000
			cmp dx, 0x5000
			je .MoveBytes
			jmp .StoreBytes
			
	.MoveBytes:
		mov eax, edx
		sub eax, 0x3000
		shl eax, 4
		add eax, ecx				; Calculate amount of bytes to copy
		mov [SourceLength], eax
		test ebx, ebx
		jz .FinalMove
		
		mov [DestinationLength], eax
		call PModeMemCpy
		add [MemoryDestination], eax
		mov edx, 0x3000
		jmp .StoreBytes
		
	.FinalMove:
		pop edx						; Bytes to zero after file contents
		add eax, edx
		mov [DestinationLength], eax
		call PModeMemCpy
		popad
		ret
	
PModeMemCpy:
	pushad
	push ds
	push es
	push fs
	push gs
	mov DWORD [MemorySource], 0x30000
	
	mov [RealStack], sp
	cli
	mov ax, 39						; 4 entries * 8 bytes - 1
	mov [GDT_Limit], ax
	xor eax, eax
	mov ax, Flat_GDT
	mov [GDT_Base], eax
	lgdt [GDT_PTR]
	mov eax, cr0					; Enter Protected 32 Bit mode
	or al, 1
	mov cr0, eax
	jmp 0x08:PModeMemCpy_Protected	; Long jump to protected mode
	
	PModeReturn:
		sti
		mov ax, 0
		mov ss, ax
		mov sp, [RealStack]
		pop gs
		pop fs
		pop es
		pop ds
		popad
		ret
	
bits 32
PModeMemCpy_Protected:
	cli
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	mov esi, [MemorySource]			; Copy source to destination
	mov edi, [MemoryDestination]
	mov ecx, [SourceLength]
	mov ebx, ecx
	rep movsb
	
	mov ecx, [DestinationLength]	; Zero memory
	sub ecx, ebx
	jz .Done
	
	xor edx, edx
	.ZeroByte:
		mov [edi], dl
		add edi, 1
		dec ecx
		jnz .ZeroByte
	
	.Done:
		jmp 0x18:Protected16Bits	; Jump to 16 bit protected mode

bits 16
Protected16Bits:
	xor ax, ax
	mov ds, ax
	mov eax, cr0
	and eax, 0xFFFFFFFE
	mov cr0, eax					; Disable protected mode
	jmp 0x0:PModeReturn				; Long jump back to real mode
	