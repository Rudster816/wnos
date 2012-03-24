#include <wnos/cpu.h>
#include <wnos/mm.h>
#include <wnos/spinlock.h>

/* cpu_data[0] will always reference the current CPU */
cpu_t** cpu_data;
PRIVATE unsigned int next_id;
PRIVATE spinlock_t nextid_lock = 0;

PUBLIC int cpudata_init()
{
	cpu_data = kmalloc((MAX_LOGICAL_CPUS + 1) * sizeof(void*));
	next_id = 1;
	nextid_lock = 0;

	return 0;
}

PUBLIC int cpu_init()
{
	void* areabase = ((void*)CPU_DATA_AREA + (next_id * CPU_DATA_LENGTH));
	int pages, ourid;

	spinlock_acquire(&nextid_lock);
	ourid = next_id;
	spinlock_release(&nextid_lock);

	pages = sizeof(cpu_t) / PAGE_FRAME_SIZE;
	if (sizeof(cpu_t) % PAGE_FRAME_SIZE)
		pages++;
	vmm_maptofree(areabase, pages);

	return 0;
}