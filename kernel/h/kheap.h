/**
 * @version		$Id: kheap.h 36 2008-11-09 16:57:55Z nasmussen $
 * @author		Nils Asmussen <nils@script-solution.de>
 * @copyright	2008 Nils Asmussen
 */

#ifndef KHEAP_H_
#define KHEAP_H_

#include "../h/common.h"

/**
 * Inits the kernel-heap
 */
void kheap_init(void);

/**
 * Note that this function is intended for debugging-purposes only!
 *
 * @return the number of free bytes
 */
u32 kheap_getFreeMem(void);

/**
 * Allocates <size> bytes in kernel-space and returns the pointer to the beginning of
 * the allocated memory. If there is not enough memory the function returns NULL.
 *
 * @param size the number of bytes to allocate
 * @return the address of the memory or NULL
 */
void *kheap_alloc(u32 size);

/**
 * Frees the via kmalloc() allocated area starting at <addr>.
 */
void kheap_free(void *addr);

#if DEBUGGING

/**
 * Prints the kernel-heap data-structure
 */
void kheap_dbg_print(void);

#endif

#endif /* KHEAP_H_ */
