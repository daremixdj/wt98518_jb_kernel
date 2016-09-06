/*
 * drivers/gpu/ion/ion_priv.h
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

#ifndef _ION_PRIV_H
#define _ION_PRIV_H

#include <linux/ion.h>
#include <linux/kref.h>
#include <linux/mm_types.h>
#include <linux/mutex.h>
#include <linux/rbtree.h>
#include <linux/sched.h>
#include <linux/shrinker.h>
#include <linux/types.h>
#include <linux/miscdevice.h>
#include <linux/ion_drv.h>


struct ion_buffer *ion_handle_buffer(struct ion_handle *handle);
bool ion_handle_validate(struct ion_client *client, struct ion_handle *handle);

/**
 * struct ion_device - the metadata of the ion device node
 * @dev:		the actual misc device
 * @buffers:		an rb tree of all the existing buffers
 * @buffer_lock:	lock protecting the tree of buffers
 * @lock:		rwsem protecting the tree of heaps and clients
 * @heaps:		list of all the heaps in the system
 * @user_clients:	list of all the clients created from userspace
 */
struct ion_device {
	struct miscdevice dev;
	struct rb_root buffers;
	struct mutex buffer_lock;
	struct rw_semaphore lock;
	struct plist_head heaps;
	long (*custom_ioctl) (struct ion_client *client, unsigned int cmd,
			      unsigned long arg);
	struct rb_root clients;
	struct dentry *debug_root;
};


/**
 * struct ion_buffer - metadata for a particular buffer
 * @ref:		refernce count
 * @node:		node in the ion_device buffers tree
 * @dev:		back pointer to the ion_device
 * @heap:		back pointer to the heap the buffer came from
 * @flags:		buffer specific flags
 * @size:		size of the buffer
 * @priv_virt:		private data to the buffer representable as
 *			a void *
 * @priv_phys:		private data to the buffer representable as
 *			an ion_phys_addr_t (and someday a phys_addr_t)
 * @lock:		protects the buffers cnt fields
 * @kmap_cnt:		number of times the buffer is mapped to the kernel
 * @vaddr:		the kenrel mapping if kmap_cnt is not zero
 * @dmap_cnt:		number of times the buffer is mapped for dma
 * @sg_table:		the sg table for the buffer if dmap_cnt is not zero
 * @pages:		flat array of pages in the buffer -- used by fault
 *			handler and only valid for buffers that are faulted in
 * @vmas:		list of vma's mapping this buffer
 * @handle_count:	count of handles referencing this buffer
 * @task_comm:		taskcomm of last client to reference this buffer in a
 *			handle, used for debugging
 * @pid:		pid of last client to reference this buffer in a
 *			handle, used for debugging
*/
struct ion_buffer {
	struct kref ref;
	union {
		struct rb_node node;
		struct list_head list;
	};
	struct ion_device *dev;
	struct ion_heap *heap;
	unsigned long flags;
	size_t size;
	union {
		void *priv_virt;
		ion_phys_addr_t priv_phys;
	};
	struct mutex lock;
	int kmap_cnt;
	void *vaddr;
	int dmap_cnt;
	struct sg_table *sg_table;
	struct page **pages;
	struct list_head vmas;
	/* used to track orphaned buffers */
	int handle_count;
	char task_comm[TASK_COMM_LEN];
	pid_t pid;
};

/**
 * struct ion_client - a process/hw block local address space
 * @node:               node in the tree of all clients
 * @dev:                backpointer to ion device
 * @handles:            an rb tree of all the handles in this client
 * @lock:               lock protecting the tree of handles
 * @name:               used for debugging
 * @task:               used for debugging
 *
 * A client represents a list of buffers this client may access.
 * The mutex stored here is used to protect both handles tree
 * as well as the handles themselves, and should be held while modifying either.
 */
struct ion_client {
        struct rb_node node;
        struct ion_device *dev;
        struct rb_root handles;
        struct mutex lock;
        const char *name;
        struct task_struct *task;
        pid_t pid;
        struct dentry *debug_root;
};

struct ion_handle_debug {
    pid_t pid;
    pid_t tgid;
    unsigned int backtrace[BACKTRACE_SIZE];
    unsigned int backtrace_num;
};

/**
 * ion_handle - a client local reference to a buffer
 * @ref:                reference count
 * @client:             back pointer to the client the buffer resides in
 * @buffer:             pointer to the buffer
 * @node:               node in the client's handle rbtree
 * @kmap_cnt:           count of times this client has mapped to kernel
 * @dmap_cnt:           count of times this client has mapped for dma
 *
 * Modifications to node, map_cnt or mapping should be protected by the
 * lock in the client.  Other fields are never changed after initialization.
 */
struct ion_handle {
        struct kref ref;
        struct ion_client *client;
        struct ion_buffer *buffer;
        struct rb_node node;
        unsigned int kmap_cnt;
#if ION_RUNTIME_DEBUGGER
        struct ion_handle_debug dbg;
#endif
};

void ion_buffer_destroy(struct ion_buffer *buffer);

/**
 * struct ion_heap_ops - ops to operate on a given heap
 * @allocate:           allocate memory
 * @free:               free memory
 * @phys                get physical address of a buffer (only define on
 *                      physically contiguous heaps)
 * @map_dma             map the memory for dma to a scatterlist
 * @unmap_dma           unmap the memory for dma
 * @map_kernel          map memory to the kernel
 * @unmap_kernel        unmap memory to the kernel
 * @map_user            map memory to userspace
 */
struct ion_heap_ops {
        int (*allocate) (struct ion_heap *heap,
                         struct ion_buffer *buffer, unsigned long len,
                         unsigned long align, unsigned long flags);
        void (*free) (struct ion_buffer *buffer);
        int (*phys) (struct ion_heap *heap, struct ion_buffer *buffer,
                     ion_phys_addr_t *addr, size_t *len);
        struct sg_table *(*map_dma) (struct ion_heap *heap,
                                        struct ion_buffer *buffer);
        void (*unmap_dma) (struct ion_heap *heap, struct ion_buffer *buffer);
        void * (*map_kernel) (struct ion_heap *heap, struct ion_buffer *buffer);
        void (*unmap_kernel) (struct ion_heap *heap, struct ion_buffer *buffer);
        int (*map_user) (struct ion_heap *mapper, struct ion_buffer *buffer,
                         struct vm_area_struct *vma);
        void (*add_freelist) (struct ion_buffer *buffer);
};

/**
 * heap flags - flags between the heaps and core ion code
 */
#define ION_HEAP_FLAG_DEFER_FREE (1 << 0)

/**
 * struct ion_heap - represents a heap in the system
 * @node:               rb node to put the heap on the device's tree of heaps
 * @dev:                back pointer to the ion_device
 * @type:               type of heap
 * @ops:                ops struct as above
 * @flags:              flags
 * @id:                 id of heap, also indicates priority of this heap when
 *                      allocating.  These are specified by platform data and
 *                      MUST be unique
 * @name:               used for debugging
 * @shrinker:           a shrinker for the heap, if the heap caches system
 *                      memory, it must define a shrinker to return it on low
 *                      memory conditions, this includes system memory cached
 *                      in the deferred free lists for heaps that support it
 * @free_list:          free list head if deferred free is used
 * @free_list_size      size of the deferred free list in bytes
 * @lock:               protects the free list
 * @waitqueue:          queue to wait on from deferred free thread
 * @task:               task struct of deferred free thread
 * @debug_show:         called when heap debug file is read to add any
 *                      heap specific debug info to output
 *
 * Represents a pool of memory from which buffers can be made.  In some
 * systems the only heap is regular system memory allocated via vmalloc.
 * On others, some blocks might require large physically contiguous buffers
 * that are allocated from a specially reserved heap.
 */
struct ion_heap {
        struct plist_node node;
        struct ion_device *dev;
        enum ion_heap_type type;
        struct ion_heap_ops *ops;
        unsigned long flags;
        unsigned int id;
        const char *name;
        struct shrinker shrinker;
        struct list_head free_list;
        size_t free_list_size;
        struct rt_mutex lock;
        wait_queue_head_t waitqueue;
        struct task_struct *task;
        int (*debug_show)(struct ion_heap *heap, struct seq_file *, void *);
};

/**
 * ion_buffer_cached - this ion buffer is cached
 * @buffer:             buffer
 *
 * indicates whether this ion buffer is cached
 */
bool ion_buffer_cached(struct ion_buffer *buffer);

/**
 * ion_buffer_fault_user_mappings - fault in user mappings of this buffer
 * @buffer:             buffer
 *
 * indicates whether userspace mappings of this buffer will be faulted
 * in, this can affect how buffers are allocated from the heap.
 */
bool ion_buffer_fault_user_mappings(struct ion_buffer *buffer);

/**
 * ion_device_create - allocates and returns an ion device
 * @custom_ioctl:       arch specific ioctl function if applicable
 *
 * returns a valid device or -PTR_ERR
 */
struct ion_device *ion_device_create(long (*custom_ioctl)
                                     (struct ion_client *client,
                                      unsigned int cmd,
                                      unsigned long arg));

/**
 * ion_device_destroy - free and device and it's resource
 * @dev:                the device
 */
void ion_device_destroy(struct ion_device *dev);

/**
 * ion_device_add_heap - adds a heap to the ion device
 * @dev:                the device
 * @heap:               the heap to add
 */
void ion_device_add_heap(struct ion_device *dev, struct ion_heap *heap);

/**
 * some helpers for common operations on buffers using the sg_table
 * and vaddr fields
 */
void *ion_heap_map_kernel(struct ion_heap *, struct ion_buffer *);
void ion_heap_unmap_kernel(struct ion_heap *, struct ion_buffer *);
int ion_heap_map_user(struct ion_heap *, struct ion_buffer *,
			struct vm_area_struct *);
int ion_heap_buffer_zero(struct ion_buffer *buffer);

/**
 * ion_heap_alloc_pages - allocate pages from alloc_pages
 * @buffer:		the buffer to allocate for, used to extract the flags
 * @gfp_flags:		the gfp_t for the allocation
 * @order:		the order of the allocatoin
 *
 * This funciton allocations from alloc pages and also does any other
 * necessary operations based on the buffer->flags.  For buffers which
 * will be faulted in the pages are split using split_page
 */
struct page *ion_heap_alloc_pages(struct ion_buffer *buffer, gfp_t gfp_flags,
				  unsigned int order);

/**
 * ion_heap_init_deferred_free -- initialize deferred free functionality
 * @heap:		the heap
 *
 * If a heap sets the ION_HEAP_FLAG_DEFER_FREE flag this function will
 * be called to setup deferred frees. Calls to free the buffer will
 * return immediately and the actual free will occur some time later
 */
int ion_heap_init_deferred_free(struct ion_heap *heap);

/**
 * ion_heap_freelist_add - add a buffer to the deferred free list
 * @heap:		the heap
 * @buffer: 		the buffer
 *
 * Adds an item to the deferred freelist.
 */
void ion_heap_freelist_add(struct ion_heap *heap, struct ion_buffer *buffer);

/**
 * ion_heap_freelist_drain - drain the deferred free list
 * @heap:		the heap
 * @size:		ammount of memory to drain in bytes
 *
 * Drains the indicated amount of memory from the deferred freelist immediately.
 * Returns the total amount freed.  The total freed may be higher depending
 * on the size of the items in the list, or lower if there is insufficient
 * total memory on the freelist.
 */
size_t ion_heap_freelist_drain(struct ion_heap *heap, size_t size);

/**
 * ion_heap_freelist_size - returns the size of the freelist in bytes
 * @heap:		the heap
 */
size_t ion_heap_freelist_size(struct ion_heap *heap);


/**
 * functions for creating and destroying the built in ion heaps.
 * architectures can add their own custom architecture specific
 * heaps as appropriate.
 */

struct ion_heap *ion_heap_create(struct ion_platform_heap *);
void ion_heap_destroy(struct ion_heap *);
struct ion_heap *ion_system_heap_create(struct ion_platform_heap *);
void ion_system_heap_destroy(struct ion_heap *);

struct ion_heap *ion_system_contig_heap_create(struct ion_platform_heap *);
void ion_system_contig_heap_destroy(struct ion_heap *);

struct ion_heap *ion_carveout_heap_create(struct ion_platform_heap *);
void ion_carveout_heap_destroy(struct ion_heap *);

struct ion_heap *ion_chunk_heap_create(struct ion_platform_heap *);
void ion_chunk_heap_destroy(struct ion_heap *);

struct ion_heap *ion_mm_heap_create(struct ion_platform_heap *);
void ion_mm_heap_destroy(struct ion_heap *);

struct ion_heap *ion_cma_heap_create(struct ion_platform_heap *);
void ion_cma_heap_destroy(struct ion_heap *);

/**
 * kernel api to allocate/free from carveout -- used when carveout is
 * used to back an architecture specific custom heap
 */
ion_phys_addr_t ion_carveout_allocate(struct ion_heap *heap, unsigned long size,
                                      unsigned long align);
void ion_carveout_free(struct ion_heap *heap, ion_phys_addr_t addr,
                       unsigned long size);
/**
 * The carveout heap returns physical addresses, since 0 may be a valid
 * physical address, this is used to indicate allocation failed
 */
#define ION_CARVEOUT_ALLOCATE_FAIL -1

/**
 * functions for creating and destroying a heap pool -- allows you
 * to keep a pool of pre allocated memory to use from your heap.  Keeping
 * a pool of memory that is ready for dma, ie any cached mapping have been
 * invalidated from the cache, provides a significant peformance benefit on
 * many systems */

/**
 * struct ion_page_pool - pagepool struct
 * @high_count:         number of highmem items in the pool
 * @low_count:          number of lowmem items in the pool
 * @high_items:         list of highmem items
 * @low_items:          list of lowmem items
 * @shrinker:           a shrinker for the items
 * @mutex:              lock protecting this struct and especially the count
 *                      item list
 * @alloc:              function to be used to allocate pageory when the pool
 *                      is empty
 * @free:               function to be used to free pageory back to the system
 *                      when the shrinker fires
 * @gfp_mask:           gfp_mask to use from alloc
 * @order:              order of pages in the pool
 * @list:               plist node for list of pools
 *
 * Allows you to keep a pool of pre allocated pages to use from your heap.
 * Keeping a pool of pages that is ready for dma, ie any cached mapping have
 * been invalidated from the cache, provides a significant peformance benefit
 * on many systems
 */
struct ion_page_pool {
        int high_count;
        int low_count;
        struct list_head high_items;
        struct list_head low_items;
        struct mutex mutex;
        gfp_t gfp_mask;
        unsigned int order;
        struct plist_node list;
};

struct ion_page_pool *ion_page_pool_create(gfp_t gfp_mask, unsigned int order);
void ion_page_pool_destroy(struct ion_page_pool *);
void *ion_page_pool_alloc(struct ion_page_pool *);
void ion_page_pool_free(struct ion_page_pool *, struct page *);

/** ion_page_pool_shrink - shrinks the size of the memory cached in the pool
 * @pool:               the pool
 * @gfp_mask:           the memory type to reclaim
 * @nr_to_scan:         number of items to shrink in pages
 *
 * returns the number of items freed in pages
 */
int ion_page_pool_shrink(struct ion_page_pool *pool, gfp_t gfp_mask,
                          int nr_to_scan);

#endif /* _ION_PRIV_H */
