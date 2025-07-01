#include "cpu/instr.h"
/*
Put the implementations of `push' instructions here.
*/

static void instr_execute_1op()
{
    operand_read(&opr_src);
    cpu.esp -= data_size / 8;
    
    opr_src.data_size = data_size;
    opr_src.addr = cpu.esp;
    opr_src.type = OPR_MEM;
    opr_src.sreg = SREG_SS;
    operand_write(&opr_src);
}

make_instr_impl_1op(push, r, v)
make_instr_impl_1op(push, rm, v)
make_instr_impl_1op(push, i, b)
make_instr_impl_1op(push, i, v)

static void push(uint32_t dest)
{
    cpu.esp -= 4;
    vaddr_write(cpu.esp, SREG_SS, 4, dest);
}

make_instr_func(pusha)
{
    if(data_size == 16)
    {
        int i;
        uint32_t temp = cpu.gpr[4]._16;//sp传给temp
        for(i = 0; i < 4; i++)
            push(cpu.gpr[i]._16);
        if(i == 4) push(temp);
        for(i = 5; i < 8; i++)
            push(cpu.gpr[i]._16);
    }
    if(data_size == 32)
    {
        int i;
        uint32_t temp = cpu.gpr[4]._32;//esp传给temp
        for(i = 0; i < 4; i++)
            push(cpu.gpr[i]._32);
        if(i == 4) push(temp);
        for(i = 5; i < 8; i++)
            push(cpu.gpr[i]._32);
    }
    return 1;
}






