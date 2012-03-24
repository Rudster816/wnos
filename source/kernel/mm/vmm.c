#include <arch/asm.h>
#include <arch/paging.h>

#include <wnos/mm.h>
#include <wnos/globals.h>
#include <wnos/kdbgio.h>

#include <string.h>

#define _P_PML4E(w) ((uint64_t*)(vmm_PML4[w] & PML4E_ADDRESS))
#define _P_PDPE(w,x) ((uint64_t*)(_P_PML4E(w)[x] & PDPE_ADDRESS))
#define _P_PDE(w,x,y) ((uint64_t*)(_P_PDPE(w,x)[y] & PDE_ADDRESS))
#define _P_PTE(w,x,y,z) ((uint64_t*)(_P_PDE(w,x,y)[z] & PTE_ADDRESS))

#define ADDR_PML4E(x) (((u64)x & 0xFF8000000000) >> 39)
#define ADDR_PDPE(x) (((u64)x & 0x7FC0000000) >> 30)
#define ADDR_PDE(x) (((u64)x & 0x3FE00000) >> 21)
#define ADDR_PTE(x) (((u64)x & 0x1FF000) >> 12)

#define ALLOC_ON_ACCESS 0x2 // Just a place holder until I come up with a better system

PRIVATE inline uint64_t read_pml4e(int w)
{
	return ((uint64_t*)PML4_VADDRBASE)[w];
}
PRIVATE inline uint64_t read_pdpe(int w, int x)
{
	uint64_t *pdpe = (uint64_t*)PDPE_VADDRBASE;
	if (read_pml4e(w) & PML4E_PRESENT)
	{
		return *(pdpe + (512 * w) + x);
	}
	
	return INVALID_ENTRY;
}
PRIVATE inline uint64_t read_pde(int w, int x, int y)
{
	uint64_t *pde = (uint64_t*)PDE_VADDRBASE;
	if (read_pml4e(w) & PML4E_PRESENT)
	{
		if ((read_pdpe(w, x) & (PDPE_PRESENT | PDPE_1GBPAGE)) == 0x1)
		{
			return *(pde + (512 * 512 * w) + (512 * x) + y); 
		}
	}
	
	return INVALID_ENTRY;
}
PRIVATE inline uint64_t read_pte(int w, int x, int y, int z)
{
	uint64_t *pte = (uint64_t*)PTE_VADDRBASE;
	if (read_pml4e(w) & PML4E_PRESENT)
	{
		if ((read_pdpe(w, x) & (PDPE_PRESENT | PDPE_1GBPAGE)) == 0x1)
		{
			if ((read_pde(w, x, y) & (PDE_PRESENT | PDE_2MBPAGE)) == 0x1)
			{
				return *(pte + (512 * 512 * 512 * (uint64_t)w) + (512 * 512 * x) + (512 * y) + z);
			}
		}
	}
	
	return INVALID_ENTRY;
}
PRIVATE inline void write_pml4e(int w, uint64_t value)
{
	((uint64_t*)PML4_VADDRBASE)[w] = value;
}
PRIVATE inline void write_pdpe(int w, int x, uint64_t value)
{
	uint64_t *pdpe = (uint64_t*)PDPE_VADDRBASE;
	if (read_pml4e(w) & PML4E_PRESENT)
	{
		*(pdpe + (512 * w) + x) = value;
	}
}
PRIVATE inline void write_pde(int w, int x, int y, uint64_t value)
{
	uint64_t *pde = (uint64_t*)PDE_VADDRBASE;
	if (read_pml4e(w) & PML4E_PRESENT)
	{
		if ((read_pdpe(w, x) & (PDPE_PRESENT | PDPE_1GBPAGE)) == 0x1)
		{
			*(pde + (512 * 512 * w) + (512 * x) + y) = value; 
		}
	}
}
PRIVATE inline void write_pte(int w, int x, int y, int z, uint64_t value)
{
	uint64_t *pte = (uint64_t*)PTE_VADDRBASE;
	if (read_pml4e(w) & PML4E_PRESENT)
	{
		if ((read_pdpe(w, x) & (PDPE_PRESENT | PDPE_1GBPAGE)) == 0x1)
		{
			if ((read_pde(w, x, y) & (PDE_PRESENT | PDE_2MBPAGE)) == 0x1)
			{
				*(pte + (512 * 512 * 512 * (uint64_t)w) + (512 * 512 * x) + (512 * y) + z) = value;
			}
		}
	}
}
PRIVATE inline uint64_t* addr_pte(int w, int x, int y, int z)
{
	uint64_t *pte = (uint64_t*)PTE_VADDRBASE;
	if (read_pml4e(w) & PML4E_PRESENT)
	{
		if ((read_pdpe(w, x) & (PDPE_PRESENT | PDPE_1GBPAGE)) == 0x1)
		{
			if ((read_pde(w, x, y) & (PDE_PRESENT | PDE_2MBPAGE)) == 0x1)
			{
				return (pte + (512 * 512 * 512 * (uint64_t)w) + (512 * 512 * x) + (512 * y) + z);
			}
		}
	}
	
	return NULL;
}

PRIVATE inline void alloc_pde(int w, int x, int y, uint64_t);
PRIVATE inline void alloc_pdpe(int w, int x, uint64_t flags);
PRIVATE inline void alloc_pml4e(int w, uint64_t flags);

PRIVATE inline void alloc_pde(int w, int x, int y, uint64_t flags)
{
	u64 t;

	if (read_pde(w, x, y) & PDE_PRESENT)
		return;

	alloc_pdpe(w, x, PDPE_PRESENT | PDPE_WRITE);

	pmm_alloc(&t, 1);
	write_pde(w, x, y, t | flags);
	write_pte(510, w, x, y, t | PTE_PRESENT | PTE_WRITE);
}
PRIVATE inline void alloc_pdpe(int w, int x, uint64_t flags)
{
	u64 t;

	if (read_pdpe(w, x) & PDPE_PRESENT)
		return;

	alloc_pml4e(w, PML4E_PRESENT | PML4E_WRITE);
	pmm_alloc(&t, 1);
	write_pdpe(w, x, t | flags);
	write_pte(509, 2, w, x, t | PTE_PRESENT | PTE_WRITE);

	alloc_pde(510, w, x, PDE_PRESENT | PDE_WRITE);
}
PRIVATE inline void alloc_pml4e(int w, uint64_t flags)
{
	u64 t;

	if (read_pml4e(w) & PML4E_PRESENT)
		return;
	
	pmm_alloc(&t, 1);
	write_pml4e(w, t | flags);
	write_pte(509, 1, 0, w, t | PTE_PRESENT | PTE_WRITE);
	
	alloc_pde(509, 2, w, PDE_PRESENT | PDE_WRITE);
	alloc_pdpe(510, w, PDPE_PRESENT | PDPE_WRITE);
}

PRIVATE uint64_t* vmm_PML4;
PRIVATE uint64_t nextaddr = 0;

PRIVATE uint64_t inline early_nextfree()
{
	globals.earlypages++;
	nextaddr += 4096;
	return nextaddr;
}

void* early_pgalloc(int pgreserve, int pgcommit)
{
	int pde, pte;
	u64 start = nextaddr + 4096 + KERNEL_BASE;

	pgreserve -= pgcommit;
	pde = ADDR_PDE(start);
	pte = ADDR_PTE(start);

	while (pgcommit)
	{
		write_pte(511, 508, pde, pte, early_nextfree() | PTE_PRESENT | PTE_WRITE);
		
		pte++;
		if (pte >= 512)
		{
			pde++;
			write_pde(511, 508, pde, early_nextfree() | PDE_PRESENT | PDE_WRITE);
			write_pte(510, 511, 508, pde, (read_pde(511, 508, pde) & PDE_ADDRESS) | PTE_PRESENT | PTE_WRITE );
			pte = 0;
		}

		pgcommit--;
	}

	while (pgreserve)
	{
		write_pte(511, 508, pde, pte, ALLOC_ON_ACCESS);
		
		pte++;
		if (pte >= 512)
		{
			pde++;
			write_pde(511, 508, pde, early_nextfree() | PDE_PRESENT | PDE_WRITE);
			write_pte(510, 511, 508, pde, (read_pde(511, 508, pde) & PDE_ADDRESS) | PTE_PRESENT | PTE_WRITE );
			pte = 0;
		}

		pgreserve--;
	}

	return (void*)start;
}

void* early_stackalloc(int pgreserve, int pgcommit)
{
	int pde, pte;
	u64 start = nextaddr + 4096 + KERNEL_BASE;

	pde = ADDR_PDE(start);
	pte = ADDR_PTE(start);
	start += PAGE_FRAME_SIZE * pgreserve;
	pgreserve -= pgcommit;

	while (pgreserve)
	{
		write_pte(511, 508, pde, pte, ALLOC_ON_ACCESS);
		
		pte++;
		if (pte >= 512)
		{
			pde++;
			write_pde(511, 508, pde, early_nextfree() | PDE_PRESENT | PDE_WRITE);
			write_pte(510, 511, 508, pde, (read_pde(511, 508, pde) & PDE_ADDRESS) | PTE_PRESENT | PTE_WRITE );
			pte = 0;
		}

		pgreserve--;
	}

	while (pgcommit)
	{
		write_pte(511, 508, pde, pte, early_nextfree() | PTE_PRESENT | PTE_WRITE);
		
		pte++;
		if (pte >= 512)
		{
			pde++;
			write_pde(511, 508, pde, early_nextfree() | PDE_PRESENT | PDE_WRITE);
			write_pte(510, 511, 508, pde, (read_pde(511, 508, pde) & PDE_ADDRESS) | PTE_PRESENT | PTE_WRITE );
			pte = 0;
		}

		pgcommit--;
	}

	return (void*)start;
}


PUBLIC int vmm_initpagesetup()
{
	unsigned long i, j, k, pde, pte, paddr;
	
	memcpy(globals.bios_bda, (const void*)0x400, 256);
	// Subtract one because we increment nextaddr before returning it
	nextaddr = ((globals.kernelpages - 1) * PAGE_FRAME_SIZE) + 0x100000;
	globals.earlypages = 0;


	vmm_PML4 = (uint64_t*)early_nextfree();
	memset(vmm_PML4, 0, 4096);
	
	vmm_PML4[0] = early_nextfree() | PML4E_PRESENT | PML4E_WRITE;
	vmm_PML4[511] = early_nextfree() | PML4E_PRESENT | PML4E_WRITE;
	memset(_P_PML4E(0), 0, 4096);
	memset(_P_PML4E(511), 0, 4096);
	
	/* Identity map first 1MB of memory excpet the first 4kb (so we cant dereference NULL)*/
	_P_PML4E(0)[0] = early_nextfree() | PDPE_PRESENT | PDPE_WRITE;
	memset(_P_PDPE(0, 0), 0, 4096);
	_P_PDPE(0, 0)[0] = early_nextfree() | PDE_PRESENT | PDE_WRITE;
	memset(_P_PDE(0, 0, 0), 0, 4096);	
	for (i = 1, paddr = 4096; i < 256; i++, paddr += 4096)
		_P_PDE(0, 0, 0)[i] = paddr | PTE_PRESENT | PTE_WRITE;
	
	/* Map kernel image */
	_P_PML4E(511)[508] = early_nextfree() | PDPE_PRESENT | PDPE_WRITE;
	memset(_P_PDPE(511, 508), 0, 4096);

	pde = ADDR_PDE(KERNEL_START);
	pte = ADDR_PTE(KERNEL_START);
	
	for (i = 0, paddr = 0x100000; i < globals.kernelpages; i++, paddr += 4096)
	{
		if (!(_P_PDPE(511, 508)[pde] & PDE_PRESENT))
			_P_PDPE(511, 508)[pde] = early_nextfree() | PDE_PRESENT | PDE_WRITE;
			
		_P_PDE(511, 508, pde)[pte] = paddr | PTE_PRESENT | PTE_WRITE | PTE_USER;
			
		pte++;
		if (pte >= 512)
		{
			pte = 0;
			pde++;
		}
	}
	
	/* Map paging structures */
	vmm_PML4[509] = early_nextfree() | PML4E_PRESENT | PML4E_WRITE | PML4E_USER;
	vmm_PML4[510] = early_nextfree() | PML4E_PRESENT | PML4E_WRITE | PML4E_USER;
	memset(_P_PML4E(509), 0, 4096);
	memset(_P_PML4E(510), 0, 4096);
	
	_P_PML4E(510)[510] = early_nextfree() | PDPE_PRESENT | PDPE_WRITE;
	_P_PDPE(510, 510)[510] = early_nextfree() | PDE_PRESENT | PDE_WRITE;
	
	/* Maps all PML4E's */
	_P_PML4E(509)[0] = early_nextfree() | PDPE_PRESENT | PDPE_WRITE;
	memset(_P_PDPE(509, 0), 0, 4096);
	_P_PDPE(509, 0)[0] = early_nextfree() | PDE_PRESENT | PDE_WRITE;
	memset(_P_PDE(509, 0, 0), 0, 4096);
	_P_PDE(509, 0, 0)[0] = (uint64_t)vmm_PML4 | PTE_PRESENT | PTE_WRITE;
	
	/* Maps all PDPE's for present PML4E's */
	_P_PML4E(509)[1] = early_nextfree() | PDPE_PRESENT | PDPE_WRITE;
	memset(_P_PDPE(509, 1), 0, 4096);
	_P_PDPE(509, 1)[0] = early_nextfree() | PDE_PRESENT | PDE_WRITE;
	memset(_P_PDE(509, 1, 0), 0, 4096);
	for (i = 0; i < 512; i++)
	{
		if (vmm_PML4[i] & PML4E_PRESENT)
			_P_PDE(509, 1, 0)[i] = (uint64_t)_P_PML4E(i) | PTE_PRESENT | PTE_WRITE;
	}
	
	/* Map all PDE's for all present PDPE's */
	_P_PML4E(509)[2] = early_nextfree() | PDPE_PRESENT | PDPE_WRITE;
	memset(_P_PDPE(509, 2), 0, 4096);	
	for (i = 0; i < 512; i++)
	{
		if (vmm_PML4[i] & PML4E_PRESENT)
		{
			_P_PDPE(509, 2)[i] = early_nextfree() | PDE_PRESENT | PDE_WRITE;
			for (j = 0; j < 512; j++)
			{
				if (_P_PML4E(i)[j] & PDPE_PRESENT)
					_P_PDE(509, 2, i)[j] = (uint64_t)_P_PDPE(i, j) | PTE_PRESENT | PTE_WRITE;
			}
		}
	}
	
	/* Map all PTE's for all present PDE's */
	for (i = 0; i < 512; i++)
	{
		if (vmm_PML4[i] & PML4E_PRESENT)
		{
			_P_PML4E(510)[i] = early_nextfree() | PDPE_PRESENT | PDPE_WRITE;
			_P_PDE(509, 2, 510)[i] = (uint64_t)_P_PDPE(510, i) | PTE_PRESENT | PTE_WRITE;
			for (j = 0; j < 512; j++)
			{
				if (_P_PML4E(i)[j] & PDPE_PRESENT)
				{
					_P_PDPE(510, i)[j] = early_nextfree() | PDE_PRESENT | PDE_WRITE;
					
					if (!(_P_PDPE(510, 510)[i] & PDE_PRESENT))
					{
						_P_PDPE(510, 510)[i] = early_nextfree() | PDE_PRESENT | PDE_WRITE;
						_P_PDE(510, 510, 510)[i] = (uint64_t)_P_PDE(510, 510, i) | PTE_PRESENT | PTE_WRITE;
					}
					
					_P_PDE(510, 510, i)[j] = (uint64_t)_P_PDE(510, i, j) | PTE_PRESENT | PTE_WRITE;
					
					for (k = 0; k < 512; k++)
					{
						if (_P_PDPE(i, j)[k] & PDE_PRESENT)
							_P_PDE(510, i, j)[k] = (uint64_t)_P_PDE(i, j, k) | PTE_PRESENT | PTE_WRITE;
					}
				}
			}
		}
	}
	
	write_cr3((uint64_t)vmm_PML4);

	return 0;
}

PUBLIC int vmm_appagesetup()
{
	/* Start out with the EXACT same paging structures as the BSP */
	write_cr3((uint64_t)vmm_PML4);
}

PUBLIC int vmm_maptofree(void* vaddr, unsigned int pages)
{
	int pml4e, pdpe, pde, pte, i;
	unsigned int pgcount;

	while (pages)
	{
		pml4e = ADDR_PML4E(vaddr);
		pdpe = ADDR_PDPE(vaddr);
		pde = ADDR_PDE(vaddr);
		pte = ADDR_PTE(vaddr);

		alloc_pde(pml4e, pdpe, pde, PDE_PRESENT | PDE_WRITE);
		pgcount = pages > (512 - pte) ? (512 - pte) : pages;
		pmm_alloc(addr_pte(pml4e, pdpe, pde, pte), pgcount);

		for (i = pte; i < (pte + pgcount); i++)
			write_pte(pml4e, pdpe, pde, i, read_pte(pml4e, pdpe, pde, i) | PTE_PRESENT | PTE_WRITE);

		vaddr = (void*)((u64)vaddr + pgcount * PAGE_FRAME_SIZE);
		pages -= pgcount;
	}

	return 0;
}

PUBLIC int vmm_maptophys(void* vaddr, void* paddr, unsigned int pages)
{
	int pml4e, pdpe, pde, pte, i;
	unsigned int pgcount;
	u64 uintpaddr = (u64)paddr;

	while (pages)
	{
		pml4e = ADDR_PML4E(vaddr);
		pdpe = ADDR_PDPE(vaddr);
		pde = ADDR_PDE(vaddr);
		pte = ADDR_PTE(vaddr);

		alloc_pde(pml4e, pdpe, pde, PDE_PRESENT | PDE_WRITE);
		pgcount = pages > (512 - pte) ? (512 - pte) : pages;

		for (i = pte; i < (pte + pgcount); i++, uintpaddr += 4096)
			write_pte(pml4e, pdpe, pde, i, uintpaddr | PTE_PRESENT | PTE_WRITE);

		vaddr = (void*)((u64)vaddr + (pgcount * PAGE_FRAME_SIZE));
		pages -= pgcount;
	}

	return 0;
}

PUBLIC int vmm_maptovalue(void* vaddr, uint64_t value, unsigned int pages)
{
	int pml4e, pdpe, pde, pte, i;
	unsigned int pgcount;

	while (pages)
	{
		pml4e = ADDR_PML4E(vaddr);
		pdpe = ADDR_PDPE(vaddr);
		pde = ADDR_PDE(vaddr);
		pte = ADDR_PTE(vaddr);

		alloc_pde(pml4e, pdpe, pde, PDE_PRESENT | PDE_WRITE);
		pgcount = pages > (512 - pte) ? (512 - pte) : pages;

		for (i = pte; i < (pte + pgcount); i++)
			write_pte(pml4e, pdpe, pde, i, value);

		vaddr = (void*)((u64)vaddr + (pgcount * PAGE_FRAME_SIZE));
		pages -= pgcount;
	}

	return 0;
}

PUBLIC int vmm_init()
{
	return 0;
}
