#include "nemu.h"
#include "cpu/cpu.h"
#include "memory/memory.h"
#include "device/mm_io.h"
#include <memory.h>
#include <stdio.h>

uint8_t hw_mem[MEM_SIZE_B];

uint32_t hw_mem_read(paddr_t paddr, size_t len)
{
	uint32_t ret = 0;
	memcpy(&ret, hw_mem + paddr, len);
	return ret;
}

void hw_mem_write(paddr_t paddr, size_t len, uint32_t data)
{
	memcpy(hw_mem + paddr, &data, len);
}

uint32_t paddr_read(paddr_t paddr, size_t len) 
{
	uint32_t ret = 0;
    int map_NO = is_mmio(paddr);//如果是内存映射I/O，就返回映射号，否则返回-1
    if(map_NO == -1)
    {
#ifdef CACHE_ENABLED
		ret = cache_read(paddr, len);     // 通过cache进行读
#else
		ret = hw_mem_read(paddr, len);
#endif
    }
    else
    {
        ret = mmio_read(paddr, len, map_NO);
    }
    return ret;
}

void paddr_write(paddr_t paddr, size_t len, uint32_t data) 
{
    int map_NO = is_mmio(paddr);//如果是内存映射I/O，就返回映射号，否则返回-1
    if(map_NO == -1)
    {
#ifdef CACHE_ENABLED
		cache_write(paddr, len, data);    // 通过cache进行写
#else
		hw_mem_write(paddr, len, data);
#endif   
    }
    else
    {
        mmio_write(paddr, len, data, map_NO);
    }
}

uint32_t laddr_read(laddr_t laddr, size_t len)
{
#ifdef IA32_PAGE
    uint32_t page_addr = laddr & 0xfff;
    if(page_addr + len > 0x1000)
    {
        uint32_t temp0 = laddr_read(laddr, 0x1000 - page_addr);
        uint32_t temp1 = laddr_read(laddr + 0x1000 - page_addr, len - (0x1000 - page_addr));
        return temp1 << (len - (0x1000 - page_addr)) | temp0;
    }
    else
    {
        if(cpu.cr0.pg == 1)
        {
            laddr = page_translate(laddr);
        }
        return paddr_read(laddr, len);
    }
#else
	return paddr_read(laddr, len);
#endif
}

void laddr_write(laddr_t laddr, size_t len, uint32_t data)
{
#ifdef IA32_PAGE
    uint32_t page_addr = laddr & 0xfff;
    if(page_addr + len > 0x1000)
    {
        laddr_write(laddr, 0x1000 - page_addr, data & ((1 << ((0x1000 - page_addr) << 3)) - 1));
        laddr_write(laddr + 0x1000 - page_addr, len - (0x1000 - page_addr), data >> ((0x1000 - page_addr) << 3));
    }
    else
    {
        if(cpu.cr0.pg == 1)
        {
            laddr = page_translate(laddr);
        }
        paddr_write(laddr, len, data);
    }
#else
	paddr_write(laddr, len, data);
#endif
}

uint32_t vaddr_read(vaddr_t vaddr, uint8_t sreg, size_t len)
{
	assert(len == 1 || len == 2 || len == 4);
#ifdef IA32_SEG	
    if(cpu.cr0.pe == 1)
    {
        vaddr = segment_translate(vaddr, sreg);
    }
	return laddr_read(vaddr, len);
#else
	return laddr_read(vaddr, len);
#endif
}

void vaddr_write(vaddr_t vaddr, uint8_t sreg, size_t len, uint32_t data)
{
	assert(len == 1 || len == 2 || len == 4);
#ifdef IA32_SEG	
    if(cpu.cr0.pe == 1)
    {
        vaddr = segment_translate(vaddr, sreg);
    }
	laddr_write(vaddr, len, data);
#else
	laddr_write(vaddr, len, data);
#endif
	
}

void init_mem()
{
	// clear the memory on initiation
	memset(hw_mem, 0, MEM_SIZE_B);
	
#ifdef CACHE_ENABLED
    init_cache();           //初始化cache
#endif

#ifdef TLB_ENABLED
	make_all_tlb();
	init_all_tlb();
#endif
}

uint32_t instr_fetch(vaddr_t vaddr, size_t len)
{
	assert(len == 1 || len == 2 || len == 4);
	return vaddr_read(vaddr, SREG_CS, len);
}

uint8_t *get_mem_addr()
{
	return hw_mem;
}
