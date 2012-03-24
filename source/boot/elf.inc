; elf.inc
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

%define Elf32_Addr 		resd
%define Elf32_Half 		resw
%define Elf32_Off 		resd
%define Elf32_Sword 	resd
%define Elf32_Word 		resd
%define unsigned_char 	resb

%define Elf64_Addr resq
%define Elf64_Off resq
%define Elf64_Half resw
%define Elf64_Word resd
%define Elf64_Sword resd
%define Elf64_Xword resq
%define Elf64_Sxword resq

%define EI_NIDENT 		16

struc Elf32_Ehdr
	.e_indent: unsigned_char EI_NIDENT
	.e_type: Elf32_Half 1
	.e_machine: Elf32_Half 1
	.e_version: Elf32_Word 1
	.e_entry: Elf32_Addr 1
	.e_phoff: Elf32_Off 1
	.e_shoff: Elf32_Off 1
	.e_flags: Elf32_Word 1
	.e_ehsize: Elf32_Half 1
	.e_phentsize: Elf32_Half 1
	.e_phnum: Elf32_Half 1
	.e_shentsize: Elf32_Half 1
	.e_shnum: Elf32_Half 1
	.e_shstrndx: Elf32_Half 1
endstruc

struc Elf64_Ehdr
	.e_indent: unsigned_char EI_NIDENT
	.e_type: Elf64_Half 1
	.e_machine: Elf64_Half 1
	.e_version: Elf64_Word 1
	.e_entry: Elf64_Addr 1
	.e_phoff: Elf64_Off 1
	.e_shoff: Elf64_Off 1
	.e_flags: Elf64_Word 1
	.e_ehsize: Elf64_Half 1
	.e_phentsize: Elf64_Half 1
	.e_phnum: Elf64_Half 1
	.e_shentsize: Elf64_Half 1
	.e_shnum: Elf64_Half 1
	.e_shstrndx: Elf64_Half 1
endstruc

%define ET_NONE 0
%define ET_REL 1
%define ET_EXEC 2
%define ET_DYN 3
%define ET_CORE 4
%define ET_LOPROC 0xFF00
%define ET_HIPROC 0xFFFF

%define EM_NONE 0
%define EM_M32 1
%define EM_SPARC 2
%define EM_386 3
%define EM_68K 4
%define EM_88K 5
%define EM_860 7
%define EM_MIPS 8

%define EV_NONE 0
%define EV_CURRENT 1

%define EI_MAG0 0
%define EI_MAG1 1
%define EI_MAG2 2
%define EI_MAG3 3
%define EI_CLASS 4
%define EI_DATA 5
%define EI_VERSION 6
%define EI_OSABI 7
%define EI_ABIVERSION 8
%define EI_PAD 9

%define ELFMAG0 0x7F
%define ELFMAG1 'E'
%define ELFMAG2 'L'
%define ELFMAG3 'F'

%define ELFCLASSNONE 0
%define ELFCLASS32 1
%define ELFCLASS64 2

%define ELFDATANONE 0
%define ELFDATA2LSB 1
%define ELFDATA2MSB 2

struc Elf32_Phdr
	.p_type: Elf32_Word 1
	.p_offset: Elf32_Off 1
	.p_vaddr: Elf32_Addr 1
	.p_paddr: Elf32_Addr 1
	.p_filesz: Elf32_Word 1
	.p_memsz: Elf32_Word 1
	.p_flags: Elf32_Word 1
	.p_align: Elf32_Word 1
endstruc

struc Elf64_Phdr
	.p_type: Elf64_Word 1
	.p_flags: Elf64_Word 1
	.p_offset: Elf64_Off 1
	.p_vaddr: Elf64_Addr 1
	.p_paddr: Elf64_Addr 1
	.p_filesz: Elf64_Xword 1
	.p_memsz: Elf64_Xword 1	
	.p_align: Elf64_Xword 1
endstruc


%define PT_NULL 0
%define PT_LOAD 1
%define PT_DYNAMIC 2
%define PT_INTERP 3
%define PT_NOTE 4
%define PT_SHLIB 5
%define PT_PHDR 6
%define PT_LOOS 0x60000000
%define PT_HIOS 0x6FFFFFFF
%define PT_LOPROC 0x70000000
%define PT_HIPROC 0x7FFFFFFF