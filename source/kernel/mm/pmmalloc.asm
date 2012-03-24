
global pmm_alloc

extern pmm_pmap
extern pmm_pfreestack

pmm_alloc:
	push rbp
	mov rbp, rsp
	mov r11, rbx
	
	mov ecx, esi
	or ecx, ecx
	jz .Done

	mov r8, pmm_pmap
	mov r8, [r8]
	mov rsp, pmm_pfreestack 		; No stack usage beyond this point
	mov rsp, [rsp]
	jmp .FromFreeStack
	
	.NextRange:
		mov [rsi], rdx
		add rsi, 8
		mov rdx, [rsi]
		add rbx, 0x40000
		or rdx, rdx
		jnz .AllocPage
	
	.FromFreeStack:
		pop rsi
		or rsi, rsi
		jz .Error
		mov rbx, rsi
		sub rbx, r8
		shl rbx, 15
		mov rdx, [rsi]
	
	.AllocPage:
		bsf rax, rdx
		jz .NextRange
		btc rdx, rax
		shl rax, 12
		add rax, rbx
		mov [rdi], rax
		add rdi, BYTE 8
		sub ecx, BYTE 1
		jnz .AllocPage
		
	mov [rsi], rdx
	or rdx, rdx
	jnz .Push
	
	add rsi, 8
	mov rdx, [rsi]
	or rdx, rdx
	jz .Done
	
	.Push:
		push rsi
	
	.Done:
		mov rbx, r11
		mov rsp, rbp
		pop rbp
		xor eax, eax
		ret
	
	.Error:
		mov rbx, r11
		mov rsp, rbp
		pop rbp
		mov eax, 1
		ret
		