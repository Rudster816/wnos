; fat32mbr.asm
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

org 0x7C00
bits 16

; Uninitialized variables 

%define BytesPerCluster 0x500
%define DoubleWord 0x504
%define NextCluster 0x508

entry:
	jmp short main
	nop

OEMName				times 8 db 0
BytesPerSector		dw 0
SectsPerCluster		db 0
ReservedSectors		dw 0
NumberOfFats		db 0
MaxRootEntries		dw 0
TotalSectors		dw 0
MediaDescriptor		db 0
SectorsPerFat		dw 0
SectorsPerTrack		dw 0
NumberOfHeads		dw 0
HiddenSectors		dd 0
TotalSectorsBig		dd 0
SectorsPerFatBig	dd	0
ExtendedFlags		dw	0
FSVersion			dw	0
RootDirStartCluster	dd	0
FSInfoSector		dw	0
BackupBootSector	dw	6
Reserved1			times 12 db 0
BootDrive			db 0
Reserved			db 0
ExtendSig			db 0
SerialNumber		dd 0
VolumeLabel			times 11 db 0
FileSystem			times 8 db 0 

main:
	xor ax,ax               		; Setup segment registers
	mov ds,ax
	mov es,ax
	mov ss,ax
	xor bp, bp
	mov sp,5000h
	
    mov [BootDrive], dl				; Save the boot drive	
	
	mov ah, 0x41					; Check if this computer supports extended reads
	mov bx, 0x55AA
	int 13h
	mov cx, 0x01
	jc Error						; Display error. Supporting CHS is too size prohibitive
	
	mov eax, DWORD [RootDirStartCluster]
	mov di, 0x7E00
	call ReadCluster				; Load the first root directory cluster

	xor eax, eax
	mov ax, [BytesPerSector]
	xor ecx, ecx
	mov cl, [SectsPerCluster]
	mul ecx
	mov [BytesPerCluster], eax
	mov ebx, eax
	shr ebx, 5						; Number of Entries per cluster	
	mov si, Filename	
	mov di, 0x7E00					; Location of the cluster
	mov ax, si
	mov dx, di
	
ReadDirEntry:
	mov cx, 11
	rep cmpsb
	jz FileFound					; File found, dx contains address of directory entry
	add dx, 32						; Go to next entry
	mov si, ax
	mov di, dx
	dec bx
	jnz ReadDirEntry
	
	push eax						; End of the entries for this cluster
	mov di, 0x7E00					; loads next cluster (if one exists) and restarts search
	mov eax, [NextCluster]
	test eax, eax
	jz FileNotFound					; We've searched all clusters and havent found the file
	call ReadCluster				; There is anotehr cluster in the chain, load it
	pop eax
	mov dx, di
	jmp ReadDirEntry				; Start searching again
	
	
FileNotFound:
	mov cx, 2
	jmp Error
	
FileFound:
	xor edi, edi
	mov di, dx
	mov ax, [di + 0x14]
	shl eax, 16
	mov ax, [di + 0x1A]
	xor edi, edi
	mov di, 0x1000
	
	.ReadFileCluster:
		call ReadCluster
		mov eax, [NextCluster]
		test eax, eax
		jz 0x1000					; Jump to 'Stage2' (ezloader.bin)
		add edi, [BytesPerCluster]
		jmp .ReadFileCluster
		

	
ReadCluster:						; eax is the desired cluster number. di is the desired destination
	pushad
	push di
	push eax
									; First lets load the cluster's entry from the FAT
	shl eax, 2						; Get exact byte offset in to FAT Table of the cluster
	push eax
	shr eax, 9						; Get offset into FAT table (sector)
	xor ebx, ebx
	mov bx, [ReservedSectors]		; Get FAT table location
	add ebx, eax					; ebx now has sector for the desired FAT entry
	mov cx, 1
	mov di, 0x7E00
	call ReadSectors
	pop ebx
	and ebx, 0x1FF
	mov eax, [0x7E00 + ebx]			; Get the desired entry
	and eax, 0x0FFFFFFF
	xor edx, edx
	cmp eax, 0x0FFFFFF8
	jae .EndOfChain
	mov edx, eax
	.EndOfChain:
	mov [NextCluster], edx			; If it was an end of chain marker, 'NextCluster' is now 0
	
	pop eax
	pop di
	.ReadClusterIntoMemory:
	sub eax, 2
	xor ecx, ecx
	mov cl, BYTE [SectsPerCluster]
	mul ecx							; Eax now contains sector offset for desiered cluster relative to the first data sector
	
	xor ebx, ebx
	mov bx, WORD [ReservedSectors]
	add ebx, eax					; Compensate for reserved sectors
	xor eax, eax
	mov ax, WORD [NumberOfFats]		; and for the FAT tables
	mul DWORD [SectorsPerFatBig]
	add ebx, eax					; Ebx now contains first LBA sector for the cluster
	call ReadSectors
	popad
	ret
	
DAPACK:
		db	0x10
		db	0
blkcnt	dw	16						; int 13 resets this to # of blocks actually read/written
db_add	dw	0x500					; memory buffer destination address
		dw	0						; in memory page zero
d_lba	dd	1						; put the lba to read in this spot
		dd	0						; more storage bytes only for big lba's ( > 4 bytes )

ReadSectors:						; ebx is LBA to start, cx is sectors to read, di is destination address
	pusha
	mov si, WORD DAPACK
	mov [db_add], di
	mov [d_lba], ebx
	mov [blkcnt], cx
	mov ah, 0x42
	mov dl, [BootDrive]
	int 0x13
	mov cx, 3
	jc Error
	popa
	ret
		
Error:
	mov si, ErrorString
	add [si + 6], cx
	.PutChar:
		lodsb
		or al,al
		jz .Done
		mov ah, 0x0E
		mov bx, 0x07
		int 0x10
		jmp .PutChar
	.Done:
		xor ax, ax
		int 0x16					; Wait for any keypress
		int 0x19					; Reboot

ErrorString db 'Error 0 Press any key to restart',0xD, 0xA,0
Filename db 'EZLOADERBIN'

times 510-($-$$) db 0   			; Pad to 510 bytes
dw 0xAA55							; Boot sector signature
        