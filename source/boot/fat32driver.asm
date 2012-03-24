; fat32driver.asm
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

FileClusterCount dd 0
FileClusters dw 0x7E00
EntryNameBuffer times 256 dw 0
NextCluster dd 0
EntriesPerCluster dd 0
BytesPerCluster dd 0
ClusterDivShift db 0
ClusterMask dd 0

FileOpen:							; SI contains address to filepath string
	pushad
	call FindFile
	xor ecx, ecx
	xor ebp, ebp
	mov bp, [FileClusters]
	
	.MoveCluster:
		mov [ebp + ecx*4], eax
		inc ecx
		call NextClusterInChain
		mov eax, [NextCluster]
		test eax, eax
		jnz .MoveCluster
		
	popad
	ret
	
FileRead:							; EAX is file offset, CX is length to read in btyes. 
	pushad							; Always loaded at fs:0
	push es
	and ecx, 0x000FFFF				; Mask high word
	push ecx
	mov ebx, eax					; Save file offset in EBX
	mov cl, [ClusterDivShift]
	shr eax, cl						; Get cluster offset
	pop ecx
	cmp eax, [FileClusterCount]
	ja .OutOfRange
	xor ebp, ebp
	mov bp, [FileClusters]
	push eax
	mov eax, [ebp + eax*4]
	xor di, di
	mov dx, 0x6000
	mov es, dx
	call ReadCluster
	pop eax
	
	mov edx, [BytesPerCluster]
	and ebx, [ClusterMask]			; Get offset into cluster 
	mov esi, ebx
	mov ebx, eax
	xor edi, edi
	
	.MoveByte:						; We need to do a manual loop to use our own segment selectors
		mov al, [es:esi]
		mov [fs:edi], al
		dec ecx
		jz .Done
		add edi, 1
		add esi, 1
		cmp esi, edx				; BytesPerCluster
		je .GetNextCluster
		jmp .MoveByte
		
	.GetNextCluster:
		add ebx, 1
		cmp ebx, [FileClusterCount]
		ja .OutOfRange
		push edi
		xor edi, edi
		mov eax, [ebp + ebx*4]
		call ReadCluster
		pop edi
		xor esi, esi
		jmp .MoveByte
		
	.Done:
		pop es
		popad
		xor eax, eax
		ret
	
	.OutOfRange:
		pop es
		popad
		mov eax, 1
		ret

FindFile:							; SI contains address to filepath string
	pushad
	mov ax, 0x1000
	mov es, ax
	mov eax, [RootDirStartCluster]
	xor di, di
	call ReadCluster
	xor ecx, ecx
	
	.NextFileOrFolder:
		mov ax, [esi + ecx*2]
		cmp ax, 0	
		je .File
		cmp ax, utf16('/')
		je .Folder
		inc ecx
		jmp .NextFileOrFolder
		
		.File:
			xor dx, dx				; Look for file
			call FindEntry
			mov [DoubleWord], eax
			popad
			mov eax, [DoubleWord]
			ret						; EAX contains first cluster of file
		.Folder:
			mov dx, 1				; Look for directory not file
			call FindEntry
			push di
			xor di, di
			call ReadCluster		; Read the first cluster for the folder into memory
			pop di
			add ecx, 1
			shl ecx, 1				; Set the name pointer to start after the '/', that way we can
			add esi, ecx			; look for the next directory or file
			xor ecx, ecx			; reset the entry name length to 0
			jmp .NextFileOrFolder
	
FindEntry:
	pushad
	xor ebp, ebp
	.CheckNextEntry:
		call GetEntryName
		mov di, EntryNameBuffer
		push si

		.CompareWord:
			mov ax, [esi]
			sub ax, [edi]
			jne .NotEqual
			add esi, 2
			add edi, 2
			sub ecx, 1
			jnz .CompareWord
			jmp .FoundEntry
			
		.NotEqual:
			pop si
			call NextEntry
			jmp .CheckNextEntry
		
	.FoundEntry:
		pop si
		mov al, [es:ebp + 0x0B]		; Entry attribute
		cmp dx, 1
		jne .File
		
		cmp al, 0x10				; We're looking for a folder
		je .EntryGood
		call NextEntry
		jmp .CheckNextEntry			; Found a file, expected a folder
		
		.File:						; We're looking for a file
			cmp al, 0x10			; Folder attribute
			jne .EntryGood			
			call NextEntry			; We found an entry with the same name, but its a folder, not a file
			jmp .CheckNextEntry
		
	.EntryGood:
		mov ax, [es:ebp + 0x14]
		shl eax, 16
		mov ax, [es:ebp + 0x1A]
		mov [DoubleWord], eax
		popad
		mov eax, [DoubleWord]
		ret
	
GetEntryName:
	pushad
	mov cx, 0
	xor edi, edi
	mov di, EntryNameBuffer
	mov al, [es:ebp + 0x0B]			; Entry attribute
	cmp al, 0x0F
	jne .Put83Name
	
	.PutLfnEntry:
		push edi
		xor eax, eax
		mov al, [es:ebp]
		and al, 0x0F
		push eax
		sub al, 1
		mov ecx, 26					; 13 Word values per LFN entry
		mul ecx
		add edi, eax
		
		mov ecx, 1
		mov ebx, 11
		call .MoveWord
		mov ecx, 14
		mov ebx, 26
		call .MoveWord
		mov ecx, 28
		mov ebx, 32
		call .MoveWord
		
		call NextEntry				; Even if this is the last LFN entry, we need EBP to point
		pop eax						; to the normal directory entry
		pop edi
		cmp eax, 1
		je .Done
		jmp .PutLfnEntry
		
		.MoveWord:
			mov al, [es:ebp + ecx]
			mov [edi], al
			add ecx, 1
			add edi, 1
			cmp ecx, ebx
			jne .MoveWord
			ret

	.Put83Name:
		mov ebx, 8
		xor ecx, ecx
		xor ax, ax
		
	.MovChar:
		mov al, [es:edx + ecx]
		mov [edi], ax
		add edi, 2
		inc ecx
		cmp ecx, ebx
		jne .MovChar
		
		cmp ebx, 11					; We need to add the '.' 
		je .Done
		mov WORD [edi], utf16('.')
		add edi, 2
		mov ebx, 11
		jmp .MovChar
		
	.Done:
		mov [DoubleWord], ebp
		popad
		mov ebp, [DoubleWord]
		ret
	
NextEntry:
	add ebp, 32
	cmp ebp, [BytesPerCluster]
	jne .Done
	push di
	push eax
	xor di, di
	mov eax, [NextCluster]
	test eax, eax
	jz .NotFound
	call ReadCluster
	pop di
	.Done:
		ret
	
	.NotFound:
		mov si, CantFindMsg
		call Error
		
NextClusterInChain:					; EAX is the current cluster
	pushad
	shl eax, 2						; Get exact byte offset in to FAT Table of the cluster
	push eax
	shr eax, 9						; Get offset into FAT table (sector)
	xor ebx, ebx
	mov bx, [ReservedSectors]		; Get FAT table location
	add ebx, eax					; EBX now has sector for the desired FAT entry
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
	popad
	ret
	
ReadCluster:						; EAX is the desired cluster number. DI is the desired destination. ES is desination segment
	pushad
	call NextClusterInChain
	
	sub eax, 2
	xor ecx, ecx
	mov cl, BYTE [SectsPerCluster]
	mul ecx							; EAX now contains sector offset for desiered cluster relative to the first data sector
	
	xor ebx, ebx
	mov bx, WORD [ReservedSectors]
	add ebx, eax					; Compensate for reserved sectors
	xor eax, eax
	mov ax, WORD [NumberOfFats]		; and for the FAT tables
	mul DWORD [SectorsPerFatBig]
	add ebx, eax					; EBX now contains first LBA sector for the cluster
	call ReadSectors
	popad
	ret
	
DAPACK:
		db	0x10
		db	0
blkcnt	dw	0						; int 13 resets this to # of blocks actually read/written
db_add	dw	0						; memory buffer destination address
db_seg	dw	0						; in memory page zero
d_lba	dd	1						; put the lba to read in this spot
		dd	0						; more storage bytes only for big lba's ( > 4 bytes )

ReadSectors:						; EBX is LBA to start, CX is sectors to read, DI is destination address, ES is desination segment
	pushad
	mov si, WORD DAPACK
	mov [db_add], di
	mov [d_lba], ebx
	mov [blkcnt], cx
	mov [db_seg], es
	mov ah, 0x42
	mov dl, [BootDrive]
	int 0x13
	mov cx, 3
	;jc Error
	popad
	ret
