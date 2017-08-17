#include<stdio.h>
#include "vector.h"
#include "chunk.h"
#include "atomic_user.h"

void *spt_malloc(unsigned long size)
{
    return malloc(size);
}

void spt_free(void *ptr)
{
    free(ptr);
}

char* spt_alloc_zero_page()
{
    void *p;
    p = malloc(4096);
    if(p != 0)
    {
        memset(p, 0, 4096);
        smp_mb();
    }
    return p;
}

void spt_free_page(void *page)
{
    free(page);
}

void *spt_realloc(void *mem_address, unsigned long newsize)
{
    return realloc(mem_address, newsize);
}


