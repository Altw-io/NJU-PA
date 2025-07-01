#include "cpu/instr.h"
/*
Put the implementations of `lgdt' instructions here.
*/
make_instr_func(lgdt)
{
#ifdef IA32_SEG
    int len = 1;
    OPERAND m;
    len += modrm_rm(eip + 1, &m);
    
    m.data_size = 16;
    operand_read(&m);
    cpu.gdtr.limit = m.val;
    
    m.data_size = 32;
    m.addr = m.addr + 2;
    operand_read(&m);
    cpu.gdtr.base = m.val;
    
    return len;
#else
    assert(0);
#endif
}