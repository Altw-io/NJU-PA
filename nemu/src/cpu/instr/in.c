#include "cpu/instr.h"
#include "device/port_io.h"
/*
Put the implementations of `in' instructions here.
*/
make_instr_func(in_b)
{
    uint8_t data = pio_read(cpu.gpr[2]._16, 1);
    cpu.gpr[0]._8[0] = data;
    return 1;
}
make_instr_func(in_v)
{
    if(data_size == 16)
    {
        uint16_t data = pio_read(cpu.gpr[2]._16, 2);
        cpu.gpr[0]._16 = data;
    }
    if(data_size == 32)
    {
        uint32_t data = pio_read(cpu.gpr[2]._16, 4);
        cpu.eax = data;
    }
    return 1;
}