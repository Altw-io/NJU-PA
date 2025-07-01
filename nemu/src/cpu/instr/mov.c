#include "cpu/instr.h"

static void instr_execute_2op() 
{
	operand_read(&opr_src);
	opr_dest.val = opr_src.val;
	operand_write(&opr_dest);
}

make_instr_impl_2op(mov, r, rm, b)
make_instr_impl_2op(mov, r, rm, v)
make_instr_impl_2op(mov, rm, r, b)
make_instr_impl_2op(mov, rm, r, v)
make_instr_impl_2op(mov, rm, r, w)
make_instr_impl_2op(mov, i, rm, b)
make_instr_impl_2op(mov, i, rm, v)
make_instr_impl_2op(mov, i, r, b)
make_instr_impl_2op(mov, i, r, v)
make_instr_impl_2op(mov, a, o, b)
make_instr_impl_2op(mov, a, o, v)
make_instr_impl_2op(mov, o, a, b)
make_instr_impl_2op(mov, o, a, v)

make_instr_func(mov_zrm82r_v) {
	int len = 1;
	OPERAND r, rm;
	r.data_size = data_size;
	rm.data_size = 8;
	len += modrm_r_rm(eip + 1, &r, &rm);
	
	operand_read(&rm);
	r.val = rm.val;
	operand_write(&r);

	print_asm_2("mov", "", len, &rm, &r);
	return len;
}

make_instr_func(mov_zrm162r_l) {
        int len = 1;
        OPERAND r, rm;
        r.data_size = 32;
        rm.data_size = 16;
        len += modrm_r_rm(eip + 1, &r, &rm);

        operand_read(&rm);
        r.val = rm.val;
        operand_write(&r);
	print_asm_2("mov", "", len, &rm, &r);
        return len;
}

make_instr_func(mov_srm82r_v) {
        int len = 1;
        OPERAND r, rm;
        r.data_size = data_size;
        rm.data_size = 8;
        len += modrm_r_rm(eip + 1, &r, &rm);
        
	operand_read(&rm);
        r.val = sign_ext(rm.val, 8);
        operand_write(&r);
	print_asm_2("mov", "", len, &rm, &r);
        return len;
}

make_instr_func(mov_srm162r_l) {
        int len = 1;
        OPERAND r, rm;
        r.data_size = 32;
        rm.data_size = 16;
        len += modrm_r_rm(eip + 1, &r, &rm);
        operand_read(&rm);
        r.val = sign_ext(rm.val, 16);
        operand_write(&r);

	print_asm_2("mov", "", len, &rm, &r);
        return len;
}

make_instr_func(mov_c2r_l) {
#ifdef IA32_SEG
    int len = 1;
    OPERAND rm, cr;
    uint8_t op;
    rm.data_size = data_size;
    len += modrm_opcode_rm(eip + 1, &op, &rm);
    assert(op == 0);

    cr.type = OPR_CREG;
    cr.addr = op;    //只需要确定op == 0，即确定是cr0寄存器，这里op存储的是reg_opcode，代表cr0等寄存器，r/m代表通用寄存器，一般是000即rax

    rm.val = cpu.cr0.val;
    operand_write(&rm);

    print_asm_2("mov", "", len, &cr, &rm);

    return len;
#else
    assert(0);
#endif
}

make_instr_func(mov_r2c_l)
{
#ifdef IA32_SEG
    int len = 1;
    OPERAND rm, cr;
    uint8_t op;
    rm.data_size = data_size;
    len += modrm_opcode_rm(eip + 1, &op, &rm);
    assert(op == 0 || op == 3);

    cr.type = OPR_CREG;
    cr.addr = op;    //同样不需要对cr操作，只需要确定op的值，以此得到对应的是cr0还是cr3寄存器，然后读取rm的值，把值给cr0或者cr3寄存器

    operand_read(&rm);
    if (op == 0)
    {
        cpu.cr0.val = rm.val;
    }
    else if (op == 3)
    {
#ifdef IA32_PAGE
        cpu.cr3.val = rm.val;
#else
        assert(0);
#endif
    }

    print_asm_2("mov", "", len, &rm, &cr);

    return len;
#else
    assert(0);
#endif
}
