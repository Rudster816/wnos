#include <arch/asm.h>

#include <wnos/kdbgio.h>
#include <wnos/types.h>
#include <wnos/smp.h>
#include <wnos/globals.h>
#include <wnos/mm.h>
#include <wnos/interrupt.h>

#include <standards/multiprocessor.h>
#include <standards/apic.h>

#include <string.h>

#define KDEBUG kprintf


mpentry_processor_t *mp_processors[256];
mpentry_bus_t *mp_busses[16];
mpentry_ioapic_t *mp_ioapics[16];
mpentry_ioint_t *mp_ioints[256];
mpentry_localint_t *mp_localints[256];
int processor_count = 0;
int bus_count = 0;
int ioapic_count = 0;
int ioint_count = 0;
int localint_count = 0;

int parse_mptable()
{
	int i;
	unsigned char checksum;
	void *mp_entry;
	void *bios_ebda, *lastkilo = (void*)0x9FC00, *bios_rom = (void*)0xF0000;
	mp_pointer_t *mp_pointer;
	mp_header_t *mp_header;
	
	bios_ebda = (void*)((unsigned long)(((unsigned short*)globals.bios_bda)[7] << 4));
	KDEBUG("EBDA Location Set: %p\n", bios_ebda);
	
	/* Search EBDA and last kilobyte of base memory simultaneously for the MP Floating Pointer*/
	for (i = 0; i < 256; i++)
	{
		if (((unsigned int*)bios_ebda)[i] == MP_POINTER_SIG)
		{
			mp_pointer = (mp_pointer_t*)&((unsigned int*)bios_ebda)[i];
			goto MPPointerFound;
		}
		else if (((unsigned int*)lastkilo)[i] == MP_POINTER_SIG)
		{
			mp_pointer = (mp_pointer_t*)&((unsigned int*)lastkilo)[i];
			goto MPPointerFound;
		}
	}
	
	/* Wasn't found in the EBDA or base memory, search the BIOS ROM area 0xF0000 -> 0xFFFFF */
	for (i = 0; i < 16384; i++)
	{
		if (((unsigned int*)bios_rom)[i] == MP_POINTER_SIG)
		{
			mp_pointer = (mp_pointer_t*)&((unsigned int*)bios_rom)[i];
			goto MPPointerFound;
		}		
	}
	
	/* Exhausted list of valid locations for MP Floating Pointer, return error */
	KDEBUG("ERROR: Could not find MP Pointer\n");
	return 1;
	
MPPointerFound:
	KDEBUG("Found MP Pointer at: %p\n", mp_pointer);
	
	if (mp_pointer->Features[0] || !mp_pointer->PhysicalAddr)
	{
		KDEBUG("ERROR: Default MP configurations not supported!");
		return 1;
	}
	
	checksum = 0;
	for (i = 0; i < (mp_pointer->Length * 16); i++)
	{
		checksum += ((unsigned char*)mp_pointer)[i];
	}
	
	if (checksum)
	{
		KDEBUG("WARNING: MP Pointer checksum not zero!, Value: %i\n", (int)checksum);
	}
	
	mp_header = (mp_header_t*)((unsigned long)mp_pointer->PhysicalAddr);
	KDEBUG("MP Header Set: %p\n", mp_header);
	
	if (mp_header->Signature != MP_HEADER_SIG)
	{
		KDEBUG("WARNING: MP Header signature incorrect!, Value: %u\n", mp_header->Signature);
	}
	
	for (i = 0; i < mp_header->BaseTableLength; i++)
	{
		checksum += ((unsigned char*)mp_header)[i];
	}
	
	if (checksum)
	{
		KDEBUG("WARNING: MP base table checksum not zero!, Value: %i\n", (int)checksum);
	}
	
	mp_entry = (void*)((unsigned long)mp_header + 44);
	
	for (i = 0; i < mp_header->EntryCount; i++)
	{
		switch (((unsigned char*)mp_entry)[0])
		{
		case MPENTRY_PROCESSOR:
			mp_processors[processor_count] = (mpentry_processor_t*)mp_entry;
			processor_count++;
			mp_entry = (void*)((unsigned long)mp_entry + 20);
			break;
		case MPENTRY_BUS:
			mp_busses[bus_count] = (mpentry_bus_t*)mp_entry;
			bus_count++;
			mp_entry = (void*)((unsigned long)mp_entry + 8);
			break;
		case MPENTRY_IOAPIC:
			mp_ioapics[ioapic_count] = (mpentry_ioapic_t*)mp_entry;
			ioapic_count++;
			mp_entry = (void*)((unsigned long)mp_entry + 8);
			break;
		case MPENTRY_IOINTASSIGN:
			mp_ioints[ioint_count] = (mpentry_ioint_t*)mp_entry;
			ioint_count++;
			mp_entry = (void*)((unsigned long)mp_entry + 8);
			break;
		case MPENTRY_LOCINTASSIGN:
			mp_localints[localint_count] = (mpentry_localint_t*)mp_entry;
			localint_count++;
			mp_entry = (void*)((unsigned long)mp_entry + 8);
			break;
		}
	}
	
	return 0;
}

extern void *ap_trampoline;
static uint64_t *ap_trampoline_code;
u32 apalive;

void ap_init()
{
	apalive = 1;
	disable_interrupts();
	halt_cpu();
}

#define LAPIC_ICR_31 (u32*)0xFEE00300
#define LAPIC_ICR_63 (u32*)0xFEE00310

void write_lapic(uint32_t* regaddr, uint32_t value)
{
	volatile uint32_t* reg = regaddr;
	*reg = value;
}

uint32_t read_lapic(uint32_t* regaddr)
{
	return *regaddr;
}

int mp_init()
{
	int i;
	unsigned int bsp_localapic = 0;
	
	ap_trampoline_code = (uint64_t*)&ap_trampoline;
	vmm_maptophys((void*)0xFEE00000, (void*)0xFEE00000, 1);
	
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
		else
		{
			KDEBUG("AP Local APIC ID Set: %i\n", (int)mp_processors[i]->LocalApicID);
		}
	}
	
	ap_trampoline_code[12] = (u64)0x1000;
	ap_trampoline_code[14] = (uint64_t)ap_init;
	memcpy((void*)0x1000, ap_trampoline_code, 176);

	for (i = 0; i < processor_count; i++)
	{
		if (mp_processors[i]->LocalApicID == bsp_localapic)
			continue;
		KDEBUG("Initializing CPU with ID: %i ...", (int)mp_processors[i]->LocalApicID);

		apalive = 0;

		write_lapic(LAPIC_ICR_63, (((u32)mp_processors[i]->LocalApicID) << 24));
		write_lapic(LAPIC_ICR_31, 0x4500); // Send INIT IPI

		pit_delay(10); // 10 millaseconds

		write_lapic(LAPIC_ICR_63, (((u32)mp_processors[i]->LocalApicID) << 24));
		write_lapic(LAPIC_ICR_31, 0x4601); // Send SIPI to start at address 0x1000

		if (pit_timeout(2, &apalive))
		{
			kprintf("AP Sucessfully started\n");
			continue;
		}

		write_lapic(LAPIC_ICR_63, (((u32)mp_processors[i]->LocalApicID) << 24));
		write_lapic(LAPIC_ICR_31, 0x4601); // Send SIPI to start at address 0x1000

		if (pit_timeout(2, &apalive))
		{
			kprintf("AP Sucessfully started\n");
			continue;
		}

		kprintf("AP with ID: %i FAILED to start\n", (int)mp_processors[i]->LocalApicID);
	}

	return 0;
}
