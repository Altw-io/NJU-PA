#include "cpu/intr.h"
#include "cpu/instr.h"
#include "memory/memory.h"

#ifdef IA32_INTR
static void push(uint32_t dest)
{
    cpu.esp -= 4;
    vaddr_write(cpu.esp, SREG_SS, 4, dest);
}
#endif
void raise_intr(uint8_t intr_no)
{
#ifdef IA32_INTR
    //保护断点和程序状态：依次将EFLAGS、CS、EIP寄存器压栈；
    push(cpu.eflags.val);
    push(cpu.cs.val);
    push(cpu.eip);
    //当异常事件是外部中断时，清除EFLAGS中的IF位；否则不清除；
    GateDesc gate;
    gate.val[0] = laddr_read(cpu.idtr.base + intr_no * 8, 4);
    gate.val[1] = laddr_read(cpu.idtr.base + intr_no * 8 + 4, 4);
    assert(gate.present);
    if(gate.type == 0xe)
    {
        cpu.eflags.IF = 0;
    }
    //根据指令或硬件给出的异常和中断类型号，查询中断描述符表（IDT）得到处理程序的入口地址并跳转执行；
    uint32_t offset = (gate.offset_31_16 << 16) | gate.offset_15_0;
    cpu.cs.val = gate.selector;
    load_sreg(SREG_CS);
    cpu.eip = segment_translate(offset, SREG_CS);
    //eip里面存储的只能是线性地址。
    //CPU的设计就是：当取指令时，使用eip中的线性地址，然后通过MMU（内存管理单元）进行页表转换得到物理地址。
#endif
}

void raise_sw_intr(uint8_t intr_no)
{
	// return address is the next instruction
	cpu.eip += 2;
	raise_intr(intr_no);
}
