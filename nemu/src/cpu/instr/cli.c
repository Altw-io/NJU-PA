#include "cpu/instr.h"
/*
Put the implementations of `cli' instructions here.
*/
make_instr_func(cli)
{
#ifdef IA32_INTR
    cpu.eflags.IF = 0;
    return 1;
#else
    assert(0);
#endif
}