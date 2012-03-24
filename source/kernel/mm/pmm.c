#include <wnos/mm.h>
#include <arch/asm.h>
#include <wnos/kdbgio.h>
#include <wnos/globals.h>

#include <string.h>

/* The total amount of available RAM in page frames */
uint64_t pmm_pftotal = 0;
uint64_t pmm_maxpaddr = 0;
/* Length of physical frame bitmap in bits */
uint64_t pmm_maplen;
/* The size of the bitmap in 8 bytes */
uint64_t pmm_mapsize;
/* Physical address space bitmap */
uint64_t *pmm_pmap;
uint64_t *pmm_pfreestack;

void pmm_freeregion(uint64_t base, uint64_t len)
{
	uint64_t cur, end;
	
	if ((base + len) > pmm_maxpaddr)
		return;
	
	cur = base / PAGE_FRAME_SIZE;
	end = (base + len) / PAGE_FRAME_SIZE;
	
	while (cur < end)
	{
		pmm_pmap[cur / 64] |= (1 << (cur % 64));
		cur++;
	}
}

void pmm_freepage(uint64_t addr)
{
	uint64_t t = addr / PAGE_FRAME_SIZE;
	pmm_pmap[t / 64] |= (1 << (t % 64));
}

#define FREELIST_PUSH(x) pmm_pfreestack--; *pmm_pfreestack = x
#define FREELIST_POP *pmm_pfreestack; pmm_pfreestack++

int pmm_init()
{
	int i;
	uint64_t t;
	mmap_entry_t* entry = (mmap_entry_t*)(uintptr_t)globals.mbinfo->mmap_addr;

	pmm_maxpaddr = 0;
	while ((void*)entry < ADDPTR_BYTES((u64)globals.mbinfo->mmap_addr, globals.mbinfo->mmap_length))
	{
		if ((entry->addr + entry->len) > pmm_maxpaddr)
			pmm_maxpaddr = entry->addr + entry->len;
		entry = ADDPTR_BYTES(entry, entry->size + 4); // + 4 because the size variable isnt included in the size given
	}

	pmm_maplen = pmm_maxpaddr / PAGE_FRAME_SIZE;
	pmm_mapsize = pmm_maplen / 8; // 8 bits per byte
	if (pmm_maplen % 8)
		pmm_mapsize++;
	
	t = pmm_mapsize / PAGE_FRAME_SIZE;
	if (pmm_mapsize % PAGE_FRAME_SIZE)
		t++;

	pmm_pmap = early_pgalloc(t, t);
	pmm_pfreestack = early_stackalloc(t / 2 , 1);
	
	kprintf("pmm_pfreestack: %p\n", pmm_pfreestack);
	kprintf("pmm_pmap: %p\n", pmm_pmap);

	memset(pmm_pmap, 0, PAGE_FRAME_SIZE * t);
	entry = (mmap_entry_t*)(uintptr_t)globals.mbinfo->mmap_addr;
	while ((void*)entry < ADDPTR_BYTES((uint64_t)globals.mbinfo->mmap_addr, globals.mbinfo->mmap_length))
	{
		if (entry->addr < 0x100000 || entry->type != 1){}
		else
			pmm_freeregion(entry->addr, entry->len);
		
		entry = ADDPTR_BYTES(entry, entry->size + 4);
	}
	
	for (i = 256; i < (globals.kernelpages +  globals.earlypages + 256); i++)
	{
		pmm_pmap[i / 64] &= (~(1 << (i % 64)));
	}

	FREELIST_PUSH(0);
	FREELIST_PUSH(((u64)&pmm_pmap[i / 64]));

	kprintf("pmm_pfreestack: %p\n", pmm_pfreestack);
	kprintf("pmm_pmap: %p\n", pmm_pmap);
	
	return 0;
}

int pmm_free(uint64_t *src, unsigned int pgcount)
{
	if (!pgcount)
		return 0;
}

