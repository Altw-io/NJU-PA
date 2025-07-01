#include "cpu/instr.h"
/*
Put the implementations of `sti' instructions here.
*/
make_instr_func(sti)
{
#ifdef IA32_INTR
    cpu.eflags.IF = 1;
    return 1;
#else
    assert(0);
#endif
}