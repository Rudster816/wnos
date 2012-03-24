#include <arch/asm.h>

#include <wnos/kdbgio.h>
#include <wnos/interrupt.h>
#include <wnos/mm.h>
#include <wnos/smp.h>
#include <wnos/globals.h>
#include <wnos/cpu.h>

global_values_t globals;
extern unsigned long kernel_image_end; // Defined in the linker script, linker.ld

void kmain(multiboot_info_t* mbd, unsigned int magic)
{
	unsigned long ksize;

	kprintf_cls();
	kprintf("Starting WNOS...\n\n");

	globals.mbinfo = mbd;

	ksize = ((unsigned long)&kernel_image_end) - KERNEL_START;
	globals.kernelpages = ksize / PAGE_FRAME_SIZE;
	if ((ksize % PAGE_FRAME_SIZE) > 0)
		globals.kernelpages++;
	
	exceptions_init();	
	vmm_initpagesetup();
	kmalloc_init();
	pmm_init();
	//cpudata_init();
	//cpu_init();
	mp_init();

	kprintf("\nSo far so good!!!\n");
	disable_interrupts();
	halt_cpu();
}
