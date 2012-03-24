#ifndef _KERNEL_INCLUDE_GLOBALS_H
#define _KERNEL_INCLUDE_GLOBALS_H

#include <standards/multiboot.h>
#include <wnos/types.h>

typedef struct global_values
{
	multiboot_info_t *mbinfo;
	uint8_t bios_bda[256];
	unsigned long kernelpages;
	unsigned long earlypages;
} global_values_t;

extern global_values_t globals;

#endif