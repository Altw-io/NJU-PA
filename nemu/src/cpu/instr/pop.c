#include "cpu/instr.h"
/*
Put the implementations of `pop' instructions here.
*/
static void instr_execute_1op()
{
    OPERAND re;
    re.addr = cpu.esp;
    re.type = OPR_MEM;
    re.sreg = SREG_SS;
    re.data_size = data_size;
    operand_read(&re);
    
    opr_src.val = re.val;
    operand_write(&opr_src);
    
    cpu.esp += data_size / 8;
}

make_instr_impl_1op(pop, r, v)

static void pop_32(uint32_t *dest)
{
    *dest = vaddr_read(cpu.esp, SREG_SS, 4);
    cpu.esp += 4;
}
static void pop_16(uint16_t *dest)
{
    *dest = vaddr_read(cpu.esp, SREG_SS, 2);
    cpu.esp += 4;
}
make_instr_func(popa)
{
    if(data_size == 16)
    {
        for(int i = 7; i > 4; i--)
            pop_16(&cpu.gpr[i]._16);
        cpu.esp += 4;
        for(int i = 3; i >= 0; i--)
            pop_16(&cpu.gpr[i]._16);
    }
    if(data_size == 32)
    {
        for(int i = 7; i > 4; i--)
            pop_32(&cpu.gpr[i]._32);
        cpu.esp += 4;
        for(int i = 3; i >= 0; i--)
            pop_32(&cpu.gpr[i]._32);
    }
    return 1;
}