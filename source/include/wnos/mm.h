#ifndef _KERNEL_INCLUDE_MM_H
#define _KERNEL_INCLUDE_MM_H

#include <wnos/types.h>

#define BYTE (u64)1
#define KILOBYTE ((u64)BYTE * 1024)
#define MEGABYTE ((u64)KILOBYTE * 1024)
#define GIGABYTE ((u64)MEGABYTE * 1024)
#define TERABYTE ((u64)GIGABYTE * 1024)

#define BYTES(x) (u64)x
#define KILOBYTES(x) ((u64)x * KILOBYTE)
#define MEGABYTES(x) ((u64)x * MEGABYTE)
#define GIGABYTES(x) ((u64)x * GIGABYTE)
#define TERABYTES(x) ((u64)x * TERABYTE)

#define ADDPTR_BYTES(p,x) (((void*)p) + x)

#ifndef NULL
#define NULL 0
#endif

#define PAGE_FRAME_SIZE KILOBYTES(4)
#define NORMAL_PAGE_SIZE KILOBYTES(4)
#define BIG_PAGE_SIZE MEGABYTES(2)
#define HUGE_PAGE_SIZE GIGABYTES(1)
#define KERNEL_BASE (u64)0xFFFFFFFF00000000
#define KERNEL_START (u64)0xFFFFFFFF00100000

/* Defined in kernel/mm/pmm.c */

extern uint64_t pmm_maxpaddr;
extern uint64_t pmm_pftotal;
extern int pmm_init();
extern int pmm_free(uint64_t* src, unsigned int pgcount);
extern int pmm_alloc(uint64_t* dest, unsigned int pgcount);

/* Defined in kernel/mm/vmm.c */
extern int vmm_initpagesetup();
extern int vmm_appagesetup();
extern int vmm_maptophys(void* vaddr, void* paddr, unsigned int pages);
extern int vmm_maptofree(void* vaddr, unsigned int pages);
extern int vmm_maptovalue(void* vaddr, uint64_t value, unsigned int pages);
extern int vmm_init();

extern void* early_pgalloc(int pgreserve, int pgcommit);
extern void* early_stackalloc(int pgreserve, int pgcommit);

/* Defined in kernel/mm/kmalloc.c */
extern int kmalloc_init();
extern void* kmalloc(size_t size);

#endif