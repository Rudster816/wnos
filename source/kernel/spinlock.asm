bits 64

global spinlock_acquire
global spinlock_release

section .text

spinlock_acquire:
	lock bts DWORD [rdi], 0
	jc .spin
	ret	; Lock acquired, so return
	.spin:
		pause
		bt DWORD [rdi], 0
		jnc spinlock_acquire
		jmp .spin

spinlock_release:
	lock bts DWORD [rdi], 0
	ret