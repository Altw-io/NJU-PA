#include "cpu/instr.h"

static void instr_execute_1op()
{
    operand_read(&opr_src);

    int offset = sign_ext(opr_src.val, opr_src.data_size);

    cpu.eip += offset;
}

make_instr_impl_1op(jmp, i, near)
make_instr_impl_1op(jmp, i, short_)


make_instr_func(jmp_rm_v)
{
        int len = 1;
        OPERAND rm;
        rm.data_size = data_size;
        // decode_operand_rm
        len += modrm_rm(eip + 1, &rm);
        print_asm_1("jmp", "rm", len, &rm);
        operand_read(&rm);
        
        // jmp_v
        cpu.eip = rm.val;

        return 0;
}
make_instr_func(jmp_ptr)
{
#ifdef IA32_SEG
    cpu.segReg[1].val = paddr_read(cpu.eip + 5, 2);
    load_sreg(1);
#endif 
    OPERAND ptr;
    ptr.data_size = data_size;
    ptr.addr = cpu.eip + 1;
    ptr.sreg = SREG_CS;
    ptr.type = OPR_IMM;
    
    operand_read(&ptr);
    cpu.eip = ptr.val;
    return 0;
}
