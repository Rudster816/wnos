; multiboot.inc
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

%define MULTIBOOT_HEADER_MAGIC 0x1BADB002

struc multiboot_header
	.magic: resd 1
	.flags: resd 1
	.checksum: resd 1
	.header_addr: resd 1
	.load_addr: resd 1
	.load_end_addr: resd 1
	.bss_end_addr: resd 1
	.entry_addr: resd 1
	.mode_type: resd 1
	.width: resd 1
	.height: resd 1
	.depth: resd 1
endstruc

struc multiboot_info
	.flags: resd 1
	
	.mem_lower: resd 1
	.mem_upper: resd 1
	
	.boot_device: resd 1
	
	.cmdline: resd 1
	
	.mods_count: resd 1
	.mods_addr: resd 1
	
	.aout_tabsize:
	.elf_shdr_num: resd 1
	.aout_strsize:
	.elf_shdr_size: resd 1
	.aout_addr:
	.elf_shdr_addr: resd 1
	.aout_reserved:
	.elf_shdr_shndx: resd 1
	
	.mmap_length: resd 1
	.mmap_addr: resd 1
	
	.drives_length: resd 1
	.drives_addr: resd 1
	
	.config_table: resd 1
	.boot_loader_name: resd 1
	
	.apm_table: resd 1
	
	.vbe_control_info: resd 1
	.vbe_mode_info: resd 1
	.vbe_mode: resw 1
	.vbe_interface_seg: resw 1
	.vbe_interface_off: resw 1
	.vbe_interface_len: resw 1	
	
	.elf_phdr_length: resd 1
	.elf_phdr_addr: resd 1
endstruc

%define MEMORY_AVAILABLE 1
%define MEMORY_RESERVED 2

struc mmap_entry
	.size: resd 1
	.addr: resq 1
	.len: resq 1
	.type: resd 1
endstruc

 %define MULTIBOOT_INFO_MEMORY                   0x00000001
 %define MULTIBOOT_INFO_BOOTDEV                  0x00000002
 %define MULTIBOOT_INFO_CMDLINE                  0x00000004
 %define MULTIBOOT_INFO_MODS                     0x00000008
 %define MULTIBOOT_INFO_AOUT_SYMS                0x00000010
 %define MULTIBOOT_INFO_ELF_SHDR                 0X00000020
 %define MULTIBOOT_INFO_MEM_MAP                  0x00000040
 %define MULTIBOOT_INFO_DRIVE_INFO               0x00000080
 %define MULTIBOOT_INFO_CONFIG_TABLE             0x00000100
 %define MULTIBOOT_INFO_BOOT_LOADER_NAME         0x00000200
 %define MULTIBOOT_INFO_APM_TABLE                0x00000400
 %define MULTIBOOT_INFO_VIDEO_INFO               0x00000800
