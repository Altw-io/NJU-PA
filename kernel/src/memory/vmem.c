#include "common.h"
#include "memory.h"
#include <string.h>

#define VMEM_ADDR 0xa0000
#define SCR_SIZE (320 * 200)
#define NR_PT ((SCR_SIZE + PT_SIZE - 1) / PT_SIZE) // number of page tables to cover the vmem   //即vmem占的一级页表项的多少

PDE *get_updir();
PTE vptable[NR_PT * NR_PTE] align_to_page;  //NR_PT * NR_PTE即vmem占的二级页表项的多少，并声明了一个元素为二级页表项的数组
                                            //元素个数为NR_PT * NR_PTE，用其储存vmem的二级页表项
                                            //此处为方便起见，不同的二级页表仍然依次排列
void create_video_mapping()
{

	/* TODO: create an identical mapping from virtual memory area
	 * [0xa0000, 0xa0000 + SCR_SIZE) to physical memeory area
	 * [0xa0000, 0xa0000 + SCR_SIZE) for user program. You may define
	 * some page tables to create this mapping.
	 */
    PDE *updir = get_updir();//得到一级页表的首地址，在cr3寄存器中储存，是物理地址

	PTE *ptable = (PTE *)va_to_pa(vptable);//得到二级页表首地址对应的物理地址
    //外层循环映射一级页表项，共NR_PT个；内层循环映射二级页表项，每个一级页表对应的二级页表项都是NR_PTE即1024个
	for (uint32_t pdir_idx = 0; pdir_idx < NR_PT; pdir_idx++)
	{
        updir[(VMEM_ADDR >> 22) + pdir_idx].val = make_pde(ptable); //根据二级页表首地址填充一级页表项的内容
        //上一句，一级页表项在一级页表中的索引对应的就是虚拟（线性）地址的22到31位，frame内容是二级页表的首地址（这个无所谓，随便哪里存储）
        //二级页表的索引对应虚拟（线性）地址的12-21位，frame内容就是虚拟（线性）地址（即页的首地址）的12-31位（精华所在！！！）
        //段的基地址都是0，段选择符在段寄存器存储，虚拟地址就是线性地址
		for (uint32_t page = 0; page < NR_PTE; page++)//page计数已经映射过的二级页表项的数目
		{
			ptable->val = make_pte((((VMEM_ADDR >> 22) + pdir_idx)<< 22) + (page << 12));//根据页的首地址填充二级页表项的内容
			ptable++;
		}
	}
}

void video_mapping_write_test()
{
	int i;
	uint32_t *buf = (void *)VMEM_ADDR;
	for (i = 0; i < SCR_SIZE / 4; i++)
	{
		buf[i] = i;
	}
}

void video_mapping_read_test()
{
	int i;
	uint32_t *buf = (void *)VMEM_ADDR;
	for (i = 0; i < SCR_SIZE / 4; i++)
	{
		assert(buf[i] == i);
	}
}

void video_mapping_clear()
{
	memset((void *)VMEM_ADDR, 0, SCR_SIZE);
}
