#include "common.h"
#include "memory.h"
#include "string.h"

#include <elf.h>

#ifdef HAS_DEVICE_IDE
#define ELF_OFFSET_IN_DISK 0
#endif

#define STACK_SIZE (1 << 20)

void ide_read(uint8_t *, uint32_t, uint32_t);

void create_video_mapping();
uint32_t get_ucr3();

uint32_t loader()
{
	Elf32_Ehdr *elf;
	Elf32_Phdr *ph, *eph;

#ifdef HAS_DEVICE_IDE
	uint8_t buf[4096];
	ide_read(buf, ELF_OFFSET_IN_DISK, 4096);
	elf = (void *)buf;
	Log("ELF loading from hard disk.");
#else
	elf = (void *)0x0;
	Log("ELF loading from ram disk.");
#endif

	/* Load each program segment */
	ph = (void *)elf + elf->e_phoff;        //ph直接指向程序头表
	eph = ph + elf->e_phnum;                //eph指向程序头表结束的下一个位置
	for (; ph < eph; ph++)
	{
		if (ph->p_type == PT_LOAD)
		{
            uint32_t paddr;

#ifdef IA32_PAGE
            paddr = mm_malloc(ph->p_vaddr, ph->p_memsz);//在虚拟地址ph->p_vaddr后面分配了ph->p_memsz长度的物理地址空间，返回ph->p_vaddr对应的物理地址
                                                        //nemu里面实现分页之后的物理地址存储位置靠mm_malloc来分配，对应的页目录和页表内容靠mm_malloc来填写
#else
            paddr = ph->p_vaddr;
#endif
            // if (ph == NULL) Log("ph is NULL!");
            // Log("paddr: 0x%x, vaddr: 0x%x", paddr, ph->p_vaddr);
            //单独写上面一行的Log会核心转储，但是搭配上一行的if语句就不会，可是if语句也没有运行，因为ph均非NULL
            //把paddr: 0x%x, vaddr: 0x%x在两个log分别写就不会核心转储，不知道原因
            // Log("paddr: 0x%x", paddr);
            // Log("vaddr: 0x%x", ph->p_vaddr);
/* TODO: copy the segment from the ELF file to its proper memory area */

#ifdef HAS_DEVICE_IDE
            ide_read((void *)paddr, ELF_OFFSET_IN_DISK + ph->p_offset, ph->p_filesz);//这里的第二个参数即offset是磁盘上的偏移量
#else
            memcpy((void *)paddr, (void *)elf + ph->p_offset, ph->p_filesz);//这里第二个参数直接是nemu主存中的偏移量
#endif
            
/* TODO: zeror the memory area [vaddr + file_sz, vaddr + mem_sz) */
            memset((void *)(paddr + ph->p_filesz), 0, ph->p_memsz - ph->p_filesz);
            
#ifdef IA32_PAGE
			/* Record the program break for future use */
			extern uint32_t brk;
			uint32_t new_brk = ph->p_vaddr + ph->p_memsz - 1;
			if (brk < new_brk)
			{
				brk = new_brk;
			}
#endif
		}
	}

	volatile uint32_t entry = elf->e_entry; 

#ifdef IA32_PAGE
	mm_malloc(KOFFSET - STACK_SIZE, STACK_SIZE);
#ifdef HAS_DEVICE_VGA
	create_video_mapping();
#endif
	write_cr3(get_ucr3());
#endif
	return entry;
}
