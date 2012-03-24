#include <arch/asm.h>

#include <wnos/mm.h>
#include <wnos/kdbgio.h>

#include <string.h>

#define CHUNK_STATUS_FREE		0x01
#define CHUNK_STATUS_RESERVED	0x02
#define CHUNK_STATUS_ALLOCATED	0x03

struct chunk_node;
typedef struct chunk_node chunk_node_t;

typedef struct chunk_node
{
	chunk_node_t* next;
	uint64_t start;
	uint64_t length;
	unsigned int status;
} chunk_node_t;

PRIVATE chunk_node_t* chunk_pool;
PRIVATE uint16_t* chunk_pool_stack;
#define CHUNK_POOL_INDEX_PUSH(x) chunk_pool_stack--; *chunk_pool_stack = x
#define CHUNK_POOL_INDEX_POP() *chunk_pool_stack; chunk_pool_stack++
#define CHUNK_POOL_PUSH(x) CHUNK_POOL_INDEX_PUSH((((uint64_t)x) - (uint64_t)chunk_pool) / 2)
#define CHUNK_POOL_POP() &chunk_pool[*chunk_pool_stack]; chunk_pool_stack++

PRIVATE chunk_node_t* kernel_heap;

#define KERNEL_HEAP_START KERNEL_BASE + GIGABYTES(1)
#define KERNEL_HEAP_LENGTH GIGABYTES(2)
#define KERNEL_HEAP_END (KERNEL_HEAP_START + KERNEL_HEAP_LENGTH)

PUBLIC int kmalloc_init()
{
	int i;

	chunk_pool_stack = early_stackalloc(KILOBYTES(128) / PAGE_FRAME_SIZE, KILOBYTES(128) / PAGE_FRAME_SIZE);
	chunk_pool = early_pgalloc((65536 * sizeof(chunk_node_t)) / PAGE_FRAME_SIZE, 1);

	for (i = 65535; i >= 0; i--)
	{
		CHUNK_POOL_INDEX_PUSH(i);
	}

	kernel_heap = CHUNK_POOL_POP();
	kernel_heap->start = KERNEL_HEAP_START;
	kernel_heap->length = KERNEL_HEAP_LENGTH;
	kernel_heap->status = CHUNK_STATUS_FREE;
	kernel_heap->next = NULL;

	return 0;
}

inline
PRIVATE chunk_node_t* findfreenode(size_t size)
{
	chunk_node_t* node;

	for (node = kernel_heap; node != NULL; node = node->next)
	{
		if (node->status & CHUNK_STATUS_ALLOCATED)
			continue;
		else if (node->status & CHUNK_STATUS_RESERVED)
			continue;
		else if (node->length < size)
			continue;

		return node;
	}

	return NULL;
}

PUBLIC void* kmalloc(size_t size)
{
	chunk_node_t* node;
	chunk_node_t* newnode;
	size_t t;

	if (!size)
		return NULL;

	t = size / PAGE_FRAME_SIZE;
	if (t % PAGE_FRAME_SIZE)
		t++;
	size = t * 4096;

	node = findfreenode(size);
	if (node == NULL)
		return NULL;

	if (node->length > size)
	{
		newnode = CHUNK_POOL_POP();
		memcpy(newnode, node, sizeof(chunk_node_t));

		node->length = size;
		node->status = CHUNK_STATUS_ALLOCATED;
		node->next = newnode;
		newnode->length -= node->length;
		newnode->start += node->length;
	}

	vmm_maptofree((void*)node->start, t);
	
	return (void*)node->start;
}
