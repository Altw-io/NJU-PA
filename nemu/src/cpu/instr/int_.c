#include "cpu/intr.h"
#include "cpu/instr.h"

/*
Put the implementations of `int' instructions here.

Special note for `int': please use the instruction name `int_' instead of `int'.
*/
make_instr_func(int_)
{
    uint32_t intr_no = vaddr_read(eip + 1, SREG_CS, 1);
    raise_sw_intr(intr_no);
    return 0;
}