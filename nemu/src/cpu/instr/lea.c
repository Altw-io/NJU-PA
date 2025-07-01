#include "cpu/instr.h"
/*
Put the implementations of `lea' instructions here.
*/
make_instr_func(lea_m2r_v)
{
    OPERAND r, rm;

    r.data_size = rm.data_size = data_size;

    int len = 1;
    len += modrm_r_rm(cpu.eip + 1, &r, &rm);
    r.val = rm.addr;

    operand_write(&r);

    print_asm_2("lea", "", len, &rm, &r);

    return len;
}