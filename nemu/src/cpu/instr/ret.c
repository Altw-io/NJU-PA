#include "cpu/instr.h"
/*
Put the implementations of `ret' instructions here.
*/
make_instr_func(ret_near)
{
    OPERAND re;
    re.data_size = data_size;
    re.sreg = SREG_SS;
    re.type = OPR_MEM;
    re.addr = cpu.esp;
    operand_read(&re);
    
    cpu.eip = re.val;
    cpu.esp += data_size / 8;
    return 0;
}

make_instr_func(ret_near_i16)
{
    OPERAND re, imm;
    
    imm.data_size = 16;
    imm.sreg = SREG_CS;
    imm.type = OPR_IMM;
    imm.addr = cpu.eip + 1;
    
    re.data_size = data_size;
    re.sreg = SREG_SS;
    re.type = OPR_MEM;
    re.addr = cpu.esp;
    
    operand_read(&re);
    operand_read(&imm);

    cpu.eip = re.val;
    cpu.esp += imm.val;

    return 0;
    
}