#include "cpu/instr.h"
/*
Put the implementations of `iret' instructions here.
*/
static void pop(uint32_t *dest)
{
    *dest = vaddr_read(cpu.esp, SREG_SS, 4);
    cpu.esp += 4;
}

make_instr_func(iret)
{
    pop(&cpu.eip);
    cpu.cs.val = vaddr_read(cpu.esp, SREG_SS, 2);
    cpu.esp += 4;
    load_sreg(SREG_CS);
    pop(&cpu.eflags.val);
    return 0;
}