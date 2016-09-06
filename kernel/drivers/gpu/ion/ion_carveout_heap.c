/*
 * drivers/gpu/ion/ion_carveout_heap.c
 *
 * Copyright (C) 2011 Google, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/spinlock.h>

#include <linux/err.h>
#include <linux/genalloc.h>
#include <linux/io.h>
#include <linux/ion.h>
#include <linux/mm.h>
#include <linux/scatterlist.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/seq_file.h>
#include "ion_priv.h"

#include <asm/mach/map.h>

struct ion_carveout_heap {
	struct ion_heap heap;
	struct gen_pool *pool;
	ion_phys_addr_t base;
};

ion_phys_addr_t ion_carveout_allocate(struct ion_heap *heap,
				      unsigned long size,
				      unsigned long align)
{
	struct ion_carveout_heap *carveout_heap =
		container_of(heap, struct ion_carveout_heap, heap);
	unsigned long offset = gen_pool_alloc(carveout_heap->pool, size);

	if (!offset)
	{
        IONMSG("ion_carveout alloc fail! size=0x%x, free=0x%x\n", size, 
            gen_pool_avail(carveout_heap->pool));
		return ION_CARVEOUT_ALLOCATE_FAIL;
	}

	return offset;
}

void ion_carveout_free(struct ion_heap *heap, ion_phys_addr_t addr,
		       unsigned long size)
{
	struct ion_carveout_heap *carveout_heap =
		container_of(heap, struct ion_carveout_heap, heap);

	if (addr == ION_CARVEOUT_ALLOCATE_FAIL)
		return;
	gen_pool_free(carveout_heap->pool, addr, size);
}

static int ion_carveout_heap_phys(struct ion_heap *heap,
				  struct ion_buffer *buffer,
				  ion_phys_addr_t *addr, size_t *len)
{
	*addr = buffer->priv_phys;
	*len = buffer->size;
	return 0;
}

static int ion_carveout_heap_allocate(struct ion_heap *heap,
				      struct ion_buffer *buffer,
				      unsigned long size, unsigned long align,
				      unsigned long flags)
{
    IONMSG("ion_carveout: alloc size=0x%x,align=0x%x,flags=0x%x\n", size,align,flags);
	buffer->priv_phys = ion_carveout_allocate(heap, size, align);
	return buffer->priv_phys == ION_CARVEOUT_ALLOCATE_FAIL ? -ENOMEM : 0;
}

static void ion_carveout_heap_free(struct ion_buffer *buffer)
{
	struct ion_heap *heap = buffer->heap;
    IONMSG("ion_carveout: free size=0x%x\n", buffer->size);
	ion_carveout_free(heap, buffer->priv_phys, buffer->size);
	buffer->priv_phys = ION_CARVEOUT_ALLOCATE_FAIL;
}

struct sg_table *ion_carveout_heap_map_dma(struct ion_heap *heap,
					      struct ion_buffer *buffer)
{
	struct sg_table *table;
	int ret;

	table = kzalloc(sizeof(struct sg_table), GFP_KERNEL);
	if (!table)
		return ERR_PTR(-ENOMEM);
	ret = sg_alloc_table(table, 1, GFP_KERNEL);
	if (ret) {
		kfree(table);
		return ERR_PTR(ret);
	}
	sg_set_page(table->sgl, phys_to_page(buffer->priv_phys), buffer->size,
		    0);
	return table;
}

void ion_carveout_heap_unmap_dma(struct ion_heap *heap,
				 struct ion_buffer *buffer)
{
	sg_free_table(buffer->sg_table);
}

void *ion_carveout_heap_map_kernel(struct ion_heap *heap,
				   struct ion_buffer *buffer)
{
	int mtype = MT_MEMORY_NONCACHED;

	if (buffer->flags & ION_FLAG_CACHED)
		mtype = MT_MEMORY;

	return __arm_ioremap(buffer->priv_phys, buffer->size,
			      mtype);
}

void ion_carveout_heap_unmap_kernel(struct ion_heap *heap,
				    struct ion_buffer *buffer)
{
	__arm_iounmap(buffer->vaddr);
	buffer->vaddr = NULL;
	return;
}

int ion_carveout_heap_map_user(struct ion_heap *heap, struct ion_buffer *buffer,
			       struct vm_area_struct *vma)
{
    IONMSG("ion_carveout: mapuser va=0x%x,size=0x%x\n", vma->vm_start, buffer->size);
	return remap_pfn_range(vma, vma->vm_start,
			       __phys_to_pfn(buffer->priv_phys) + vma->vm_pgoff,
			       vma->vm_end - vma->vm_start,
			       (vma->vm_page_prot));
			       //pgprot_noncached(vma->vm_page_prot));
}

static struct ion_heap_ops carveout_heap_ops = {
	.allocate = ion_carveout_heap_allocate,
	.free = ion_carveout_heap_free,
	.phys = ion_carveout_heap_phys,
        .map_dma = ion_carveout_heap_map_dma,
        .unmap_dma = ion_carveout_heap_unmap_dma,
	.map_user = ion_carveout_heap_map_user,
	//.map_kernel = ion_carveout_heap_map_kernel,
	//.unmap_kernel = ion_carveout_heap_unmap_kernel,
	.map_kernel = ion_heap_map_kernel,
	.unmap_kernel = ion_heap_unmap_kernel,
};

static int ion_carveout_chunk_show(struct gen_pool *pool, 
                struct gen_pool_chunk *chunk, 
                void *data)
{
    int order, nlongs, nbits,i;
    struct seq_file *s = (struct seq_file *)data;


	order = pool->min_alloc_order;
    nbits = (chunk->end_addr - chunk->start_addr) >> order;
	nlongs = BITS_TO_LONGS(nbits);

    seq_printf(s, "phys_addr=0x%x bits=", chunk->phys_addr);

    for(i=0; i<nlongs; i++)
    {
        seq_printf(s, "0x%x ", (unsigned int)chunk->bits[i]);
    }
        
    seq_printf(s, "\n");
    return 0;
}

static void ion_carveout_heap_debug_show(struct ion_heap *heap, struct seq_file *s,
                                      void *unused)
{
    struct ion_carveout_heap *carveout_heap = 
        container_of(heap, struct ion_carveout_heap, heap);
    size_t size_avail, total_size;

    total_size = gen_pool_size(carveout_heap->pool);
    size_avail = gen_pool_avail(carveout_heap->pool);

    seq_printf(s, "total_size=0x%x, free=0x%x, base=0x%x\n", 
        total_size, size_avail, (unsigned int)carveout_heap->base);

    gen_pool_for_each_chunk(carveout_heap->pool, ion_carveout_chunk_show, s);

}

struct ion_heap *ion_carveout_heap_create(struct ion_platform_heap *heap_data)
{
	struct ion_carveout_heap *carveout_heap;

    IONMSG("ion_carveout: base=0x%x,size=0x%x\n", heap_data->base, heap_data->size);

	carveout_heap = kzalloc(sizeof(struct ion_carveout_heap), GFP_KERNEL);
	if (!carveout_heap)
		return ERR_PTR(-ENOMEM);

	carveout_heap->pool = gen_pool_create(12, -1);
	if (!carveout_heap->pool) {
		kfree(carveout_heap);
		return ERR_PTR(-ENOMEM);
	}
	carveout_heap->base = heap_data->base;
	gen_pool_add(carveout_heap->pool, carveout_heap->base, heap_data->size,
		     -1);
	carveout_heap->heap.ops = &carveout_heap_ops;
	carveout_heap->heap.type = ION_HEAP_TYPE_CARVEOUT;
	carveout_heap->heap.debug_show = ion_carveout_heap_debug_show;
	return &carveout_heap->heap;
}

void ion_carveout_heap_destroy(struct ion_heap *heap)
{
	struct ion_carveout_heap *carveout_heap =
	     container_of(heap, struct  ion_carveout_heap, heap);

	gen_pool_destroy(carveout_heap->pool);
	kfree(carveout_heap);
	carveout_heap = NULL;
}
