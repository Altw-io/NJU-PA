#include "cpu/cpu.h"
#include "memory/memory.h"

#ifdef IA32_PAGE
// translate from linear address to physical address
paddr_t page_translate(laddr_t laddr)
{
#ifndef TLB_ENABLED
	PDE pde;
	PTE pte;
	PageAddr *addr = (void *)&laddr;
	paddr_t pdir_base = cpu.cr3.val & ~PAGE_MASK;  //页目录表的32位基址，低12位是页目录表中的2^12位的索引，因此不能直接写pdir_base = cpu.cr3.page_directory_base
	                                               //这样没有低12位的0值占位。之所以是12位，也是和整体分割方式保持一致。但实际上只有2^10个页目录和2^10个页
	pde.val = paddr_read(pdir_base + addr->pdir_idx * 4, 4);
	assert(pde.present);
	paddr_t pt_base = pde.val & ~PAGE_MASK;
	
	pte.val = paddr_read(pt_base + addr->pt_idx * 4, 4);
	assert(pte.present);
	paddr_t paddr = (pte.val & ~PAGE_MASK) | addr->pf_off;
	return paddr;
#else
	return tlb_read(laddr) | (laddr & PAGE_MASK);
#endif
}
#endif