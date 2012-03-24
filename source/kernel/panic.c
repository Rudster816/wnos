#include <wnos/kdbgio.h>
#include <wnos/interrupt.h>
#include <arch/asm.h>

void kpanic(registers_t *regs, unsigned long vector)
{
	kprintf_cls();
	kprintf("Kernel Panic...Vector: %i\n\n", (int)vector);

	kprintf("RIP: %X\n", regs->rip);
	kprintf("RAX: %X\n", regs->rax);
	kprintf("RBX: %X\n", regs->rbx);
	kprintf("RCX: %X\n", regs->rcx);
	kprintf("RDX: %X\n", regs->rdx);
	kprintf("RDI: %X\n", regs->rdi);
	kprintf("RSI: %X\n", regs->rsi);
	kprintf("RBP: %X\n", regs->rbp);
	kprintf("RSP: %X\n", regs->rsp);
	kprintf(" R8: %X\n", regs->r8);
	kprintf(" R9: %X\n", regs->r9);
	kprintf("R10: %X\n", regs->r10);
	kprintf("R11: %X\n", regs->r11);
	kprintf("R12: %X\n", regs->r12);
	kprintf("R13: %X\n", regs->r13);
	kprintf("R14: %X\n", regs->r14);
	kprintf("R15: %X\n", regs->r15);


	disable_interrupts();
	halt_cpu();
}