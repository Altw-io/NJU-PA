#include "cpu/instr.h"
#include "device/port_io.h"
/*
Put the implementations of `out' instructions here.
*/
make_instr_func(out_b)
{
    uint8_t data = cpu.gpr[0]._8[0];
    pio_write(cpu.gpr[2]._16, 1, data);
    return 1;
}
make_instr_func(out_v)
{
    if(data_size == 16)
    {
        uint16_t data = cpu.gpr[0]._16;
        pio_write(cpu.gpr[2]._16, 2, data);
    }
    if(data_size == 32)
    {
        uint32_t data = cpu.eax;
        pio_write(cpu.gpr[2]._16, 4, data);
    }
    return 1;
}