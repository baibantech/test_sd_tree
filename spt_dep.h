#ifndef _SPT_DEP_H
#define _SPT_DEP_H
#include <assert.h>

#define spt_assert(expr) \
            assert(expr)

#define spt_print printf

#define spt_debug(f, a...)	{ \
					spt_print ("LFORD DEBUG (%s, %d): %s:", \
						__FILE__, __LINE__, __func__); \
				  	spt_print (f, ## a); \
					}


void *spt_malloc(unsigned long size);

void spt_free(void *ptr);

char* spt_alloc_zero_page();

void spt_free_page(void *page);

void *spt_realloc(void *mem_address, unsigned long newsize);

#endif

