#include <arch/asm.h>
#include <wnos/kdbgio.h>
#include <wnos/interrupt.h>
#include <wnos/types.h>
#include <standards/multiprocessor.h>
#include <standards/apic.h>

#include <string.h>

#define KDEBUG kprintf

#ifdef DONT_INCLUDE_THIS

void pic_disable()
{
	outb(0xA1, 0xFF);
	outb(0x21, 0xFF);
}

unsigned int* get_ioapicbase(unsigned char id)
{
	int i;
	for (i = 0; i < ioapic_count; i++)
	{
		if (mp_ioapics[i]->IoApicID == id)
		{
			return (unsigned int*)((unsigned long)mp_ioapics[i]->PhysicalAddr);
		}
	}
	
	return 0;
}

void write_ioapic(volatile unsigned int *base, unsigned int reg, unsigned int val)
{
	base[0] = reg;
	base[4] = val;
}

unsigned int read_ioapic(volatile unsigned int *base, unsigned int reg)
{
	base[0] = reg;
	return base[4];
}

int ioapic_init()
{
	int i;
	unsigned int *ioapic_base;
	unsigned int redent, t;
	unsigned char bsp_localapic, intvector = 0x20;
	
	if (parse_mptable())
	{
		return 1;
	}
	
	for (i = 0; i < processor_count; i++)
	{
		if (mp_processors[i]->CpuFlags & 0x2)
		{
			bsp_localapic = mp_processors[i]->LocalApicID;
			KDEBUG("BSP Local APIC ID Set: %i\n", (int)bsp_localapic);
		}
	}
	
	/* Add entries in the I/O APIC vector table(s) for each I/O APIC Interrupt */
	for (i = 0; i < ioint_count; i++)
	{
		if (!(ioapic_base = get_ioapicbase(mp_ioints[i]->IoApicID)))
		{
			KDEBUG("WARNING: IO APIC Interrupt with unknown destination, ignoring entry");
			continue;
		}
		
		/* Set the destination local APIC to the bootstrap processor's (BSP) APIC */
		write_ioapic(ioapic_base, 
					 IOAPIC_REG_IOREDTBL(mp_ioints[i]->IoApicIntin) + 1, 
					 ((unsigned int)bsp_localapic) >> 24);
		
		/* Set interrupt vector */
		redent = 0;
		redent |= (unsigned int)intvector;
		intvector++;
		
		/* Set interrupt polarity */
		t = mp_ioints[i]->IoIntFlags & 0x3;
		if (t == 0x0) /* Conforms to specification of bus */
		{
			t = 0; // FIXME Assumes ISA bus
		}
		else if (t == 0x1)
			t = 0;
		else if (t == 0x3)
			t = 1;
		
		redent |= (t << 13);
		
		/* Set trigger mode */
		t = (mp_ioints[i]->IoIntFlags & 0xC) >> 2;
		if (t == 0x0) /* Conforms to specification of bus */
		{
			t = 0; // FIXME Assumes ISA bus
		}
		else if (t == 0x1)
			t = 0;
		else if (t == 0x3)
			t = 1;
		
		redent |= (t << 15);
		
		write_ioapic(ioapic_base, IOAPIC_REG_IOREDTBL(mp_ioints[i]->IoApicIntin), redent);
		
		KDEBUG("IOAPIC: %i Intin: %i Vector: %i IRQ: %i\n",
			   (int)mp_ioints[i]->IoApicID,
			   (int)mp_ioints[i]->IoApicIntin,
			   (int)intvector - 1,
			   (int)mp_ioints[i]->BusIrq);
	}
	
	
	
	return 0;
}

#endif

PRIVATE void pit_init();

/* Defined in kernel/isr.asm */
extern void idt_set();
extern idt_descriptor_t idt_table[256];
extern unsigned char isr_table[24576];
extern unsigned char isr_template_error[96];
extern unsigned char isr_template_noerror[96];

/* Defined in kernel/panic.c */
extern void kpanic(registers_t*, unsigned long);

isr_error_t *isr_table_err;
isr_noerror_t *isr_table_noerr;

void set_isr(unsigned char vector, unsigned long isr, unsigned short segment, unsigned short flags)
{
	int i;
	unsigned long isr_stub;
	
	isr_stub = ((unsigned long)isr_table) + 96 * vector;
	
	idt_table[vector].Reserved = 0;
	idt_table[vector].Offset0 = (unsigned short)(isr_stub & 0xFFFF);
	idt_table[vector].Offset16 = (unsigned short)((isr_stub >> 16) & 0xFFFF);
	idt_table[vector].Offset32 = (unsigned int)((isr_stub >> 32) & 0xFFFFFFFF);
	idt_table[vector].Segment = segment;
	idt_table[vector].Flags = flags;
	
	for (i = 0; i < 96; i++)
	{
		isr_table[(vector * 96) + i] = isr_template_noerror[i];
	}
	
	isr_table_noerr[vector].IsrFunction = isr;
	isr_table_noerr[vector].IsrVector = vector;
}

int exceptions_init()
{
	int i;
	
	isr_table_err = (isr_error_t*)&isr_table;
	isr_table_noerr = (isr_noerror_t*)&isr_table;
	
	idt_set();
	
	for (i = 0; i < 48; i++)
	{
		set_isr(i, (unsigned long)kpanic, 0x8, 0x8E00);
	}

	pit_init();
	
	return 0;
}

#define MASTER_PIC_COMMAND 0x20
#define MASTER_PIC_DATA 0x21
#define SLAVE_PIC_COMMAND 0xA0
#define SLAVE_PIC_DATA 0xA1
#define ICW1_ICW4	0x01		/* ICW4 (not) needed */
#define ICW1_SINGLE	0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL	0x08		/* Level triggered (edge) mode */
#define ICW1_INIT	0x10		/* Initialization - required! */ 
#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM	0x10		/* Special fully nested (not) */

#define PIT0_DATA 0x40
#define PIT_COMMAND 0x43

/* We are only going to use the PIC for SMP start delays
 * which we do before we have done any APIC stuff , so we 
 * need to use the PIC to recieve the PIT's interrupts
 */

volatile PRIVATE u64 ticks = 0;
void pit_tick()
{
	ticks++;
	outb(MASTER_PIC_COMMAND, 0x20);
}


PRIVATE void pit_init()
{
	/* First setup the PIC */

	outb(MASTER_PIC_COMMAND, ICW1_INIT + ICW1_ICW4);
	outb(SLAVE_PIC_COMMAND, ICW1_INIT + ICW1_ICW4);
	outb(MASTER_PIC_DATA, 0x20);
	outb(SLAVE_PIC_DATA, 0x28);
	outb(MASTER_PIC_DATA, 4);
	outb(SLAVE_PIC_DATA, 2);
	outb(MASTER_PIC_DATA, ICW4_8086);
	outb(SLAVE_PIC_DATA, ICW4_8086);
	/* Mask all interrupts besides IRQ 0 (the PIT) */
	outb(MASTER_PIC_DATA, 0xFE);
	outb(SLAVE_PIC_DATA, 0xFF);

	outb(PIT_COMMAND, 0x34);
	outb(PIT0_DATA, 0xB0);
	outb(PIT0_DATA, 0x04);

	set_isr(32, (unsigned long)pit_tick, 0x8, 0x8E00);
}

PUBLIC void pit_delay(unsigned int millaseconds)
{
	ticks = 0;
	enable_interrupts();
	while (ticks < millaseconds);
	disable_interrupts();
}

PUBLIC bool pit_timeout(unsigned int timeout, unsigned int* towatch)
{
	u32 orginal = *towatch;
	volatile uint32_t* vtw = towatch;

	ticks = 0;
	enable_interrupts();
	while((ticks < timeout) && (orginal == *vtw));
	return orginal == *vtw;
}

/*int interrupts_init()
{	
	int i;

	for (i = 32; i < 48; i++)
	{
		set_isr(i, (unsigned long)kpanic, 0x8, 0x8E00);
	}

	if (ioapic_init())
	{
		return 1;
	}
	
	for (;;)
		enable_interrupts();
	
	return 0;
}*/



