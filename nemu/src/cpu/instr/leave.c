#include "cpu/instr.h"
/*
Put the implementations of `leave' instructions here.
*/
make_instr_func(leave)
{
    OPERAND re;
    cpu.esp = cpu.ebp;

    re.data_size = data_size;
    re.type = OPR_MEM;
    re.sreg = SREG_SS;
    re.addr = cpu.ebp;
    operand_read(&re);
    
    cpu.ebp = re.val;
    
    cpu.esp += data_size / 8;
    return 1;
}