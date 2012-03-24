#ifndef _KERNEL_INCLUDE_INTERRUPT_H
#define _KERNEL_INCLUDE_INTERRUPT_H

#include <wnos/types.h>

extern int interrupts_init();
extern int exceptions_init();
extern int mp_init();
extern void pit_delay(unsigned int microseconds);
extern bool pit_timeout(unsigned int timeout, unsigned int* towatch);

typedef struct registers
{
	unsigned long rax;
	unsigned long rbx;
	unsigned long rcx;
	unsigned long rdx;
	unsigned long rdi;
	unsigned long rsi;
	unsigned long rbp;
	unsigned long r8;
	unsigned long r9;
	unsigned long r10;
	unsigned long r11;
	unsigned long r12;
	unsigned long r13;
	unsigned long r14;
	unsigned long r15;
	unsigned long rip;
	unsigned long cs;
	unsigned long rflags;
	unsigned long rsp;
} registers_t;

typedef struct __attribute__((__packed__)) isr_noerror
{
	char Pad1[32];
	unsigned long IsrFunction;
	char Pad2[2];
	unsigned char IsrVector;
	char Pad3[53];
} isr_noerror_t;

typedef struct __attribute__((__packed__)) isr_error
{
	char Pad1[47];
	unsigned char IsrVector;
	char Pad2[8];
	unsigned long IsrFunction;
	char Pad3[32];
} isr_error_t;

typedef struct __attribute__((__packed__)) idt_descriptor
{
	unsigned short Offset0;
	unsigned short Segment;
	unsigned short Flags;
	unsigned short Offset16;
	unsigned int Offset32;
	unsigned int Reserved;
} idt_descriptor_t;

#endif