#include "cpu/instr.h"
/*
Put the implementations of `lidt' instructions here.
*/
make_instr_func(lidt)
{
#ifdef IA32_INTR
    int len = 1;
    OPERAND m;
    len += modrm_rm(eip + 1, &m);
    
    m.data_size = 16;
    operand_read(&m);
    cpu.idtr.limit = m.val;
    
    m.addr += 2;
    m.data_size = 32;
    operand_read(&m);
    cpu.idtr.base = m.val;
    
    return len;
#else
    assert(0);
#endif
}