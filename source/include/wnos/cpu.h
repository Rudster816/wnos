#ifndef INCLUDE_WNOS_CPU_H
#define INCLUDE_WNOS_CPU_H

#include <wnos/types.h>

#define MAX_LOGICAL_CPUS 511
#define CPU_DATA_AREA 0xFFFFFF8000000000
#define CPU_DATA_LENGTH 0x200000 // 2 Megabytes

extern int cpu_init();
extern int cpudata_init();
/*  */
typedef struct cpu
{
 	/* APIC ID */
	unsigned int physical_id;
	unsigned int software_id;
	unsigned int package;
	unsigned int core;
	unsigned int thread;
	uint64_t apic_base;
	uint64_t paging_base; // The PHYSICAL address
	void* cpu_data_pages;
} cpu_t;

#endif