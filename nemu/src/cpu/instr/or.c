#include "cpu/instr.h"
/*
Put the implementations of `or' instructions here.
*/
static void instr_execute_2op() {
    operand_read(&opr_src);
    operand_read(&opr_dest);
    opr_dest.val = alu_or(sign_ext(opr_src.val, opr_src.data_size),
                          sign_ext(opr_dest.val, opr_dest.data_size),
                          data_size);
    operand_write(&opr_dest);
}

make_instr_impl_2op(or, r, rm, b)
make_instr_impl_2op(or, r, rm, v)
make_instr_impl_2op(or, rm, r, b)
make_instr_impl_2op(or, rm, r, v)
make_instr_impl_2op(or, i, a, b)
make_instr_impl_2op(or, i, a, v)
make_instr_impl_2op(or, i, rm, b)
make_instr_impl_2op(or, i, rm, v)
make_instr_impl_2op(or, i, rm, bv)