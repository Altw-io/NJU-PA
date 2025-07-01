#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "common.h"

#ifdef IA32_PAGE
#define KOFFSET 0xC0000000
#else
#define KOFFSET 0
#endif

#define va_to_pa(addr) ((void *)(((uint32_t)(addr)) - KOFFSET))
#define pa_to_va(addr) ((void *)(((uint32_t)(addr)) + KOFFSET))

/* the maxinum loader size is 16MB */
#define KMEM (16 * 1024 * 1024)

/* NEMU has 128MB physical memory  */
#define PHY_MEM (128 * 1024 * 1024)

#define make_invalid_pde() 0
#define make_invalid_pte() 0
#define make_pde(addr) ((((uint32_t)(addr)) & 0xfffff000) | 0x7)//一级页表项的frame是二级页表的首地址高二十位，make_pde(二级页表项地址)得到对应的一级页表项的内容
#define make_pte(addr) ((((uint32_t)(addr)) & 0xfffff000) | 0x7)//二级页表项的frame是paddr的高二十位，make_pte(paddr)得到对应的二级页表项的内容

uint32_t mm_malloc(uint32_t, int len);

#endif
