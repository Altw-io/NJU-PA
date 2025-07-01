#include "cpu/instr.h"
/*
Put the implementations of `call' instructions here.
*/

make_instr_func(call_direct)
{
    OPERAND imm, next;

    imm.data_size = data_size;
    imm.addr = eip + 1;
    imm.sreg = SREG_CS;
    imm.type = OPR_IMM;
    operand_read(&imm);

    cpu.esp -= data_size / 8;
    next.addr = cpu.esp;
    next.type = OPR_MEM;
    next.sreg = SREG_SS; 
    next.data_size = data_size;
    next.val = cpu.eip + 1 + data_size / 8;
    operand_write(&next);
    
    cpu.eip += sign_ext(imm.val, data_size);
    
    return 1 + data_size / 8;
}
make_instr_func(call_indirect)
{
    OPERAND rm, next;
    rm.data_size = data_size;
    int len = modrm_rm(eip + 1, &rm);
    operand_read(&rm);

    cpu.esp -= data_size / 8;
    next.addr = cpu.esp;
    next.type = OPR_MEM;
    next.sreg = SREG_SS;
    next.data_size = data_size;
    next.val = cpu.eip + 1 + len;
    operand_write(&next);

    cpu.eip = sign_ext(rm.val, data_size);
    return 0;
}













