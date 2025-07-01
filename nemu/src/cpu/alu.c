#include "cpu/cpu.h"


void set_CF_add(uint32_t result, uint32_t src, size_t data_size)
{
    result &= (0xFFFFFFFF >> (32 - data_size));//只留data_size位
    src &= (0xFFFFFFFF >> (32 - data_size));
    cpu.eflags.CF = result < src;
}
void set_CF_adc(uint32_t result, uint32_t src, size_t data_size)
{
    if(cpu.eflags.CF == 0)
    {
        set_CF_add(result, src, data_size);
    }
    else
    {
        result &= (0xFFFFFFFF >> (32 - data_size));//只留data_size位
        src &= (0xFFFFFFFF >> (32 - data_size));
        cpu.eflags.CF = result <= src;      //不分类讨论直接只用result <= src判断的话，当dest == 0,CF == 0时result == src是一定的，但是没有进位。
    }
}
// void set_CF_adc(uint32_t result, size_t data_size)
// {
//     cpu.eflags.CF = (result >> data_size) & 0x00000001;//当src和dest都是32位时，如果有进位，第33位不会保存在result里，会直接被丢弃
// }
void set_CF_sub(uint32_t result, uint32_t dest, size_t data_size)
{
    result &= (0xFFFFFFFF >> (32 - data_size));//只保留data_size位
    dest &= (0xFFFFFFFF >> (32 - data_size));
    cpu.eflags.CF = result > dest;
}
void set_CF_sbb(uint32_t result, uint32_t dest, size_t data_size)
{
    if(cpu.eflags.CF == 0)
    {
        set_CF_sub(result, dest, data_size);
    }
    else
    {
        result &= (0xFFFFFFFF >> (32 - data_size));//只留data_size位
        dest &= (0xFFFFFFFF >> (32 - data_size));
        cpu.eflags.CF = result >= dest;      //不分类讨论直接只用result >= dest判断的话，当src == 0,CF == 0时result == dest是一定的，但是没有借位。
    }
}

void set_PF(uint32_t result)
{
    unsigned char sum = 0;//设定求和变量
    for(char i = 0; i < 8; i ++)
    {
        sum += (result & 0x00000001);//累加最低位的值
        result >>= 1;//右移一位
    }
    cpu.eflags.PF = !(sum % 2);
}

void set_ZF(uint32_t result, size_t data_size)
{
    result &= (0xFFFFFFFF >> (32 - data_size));
    cpu.eflags.ZF = (result == 0);
}

void set_SF(uint32_t result, size_t data_size)
{
    cpu.eflags.SF = (result >> (data_size - 1)) & 1;//直接提取符号位
}

void set_OF_adx(uint32_t result, uint32_t src, uint32_t dest, size_t data_size)//add和adc都可以用
{
    int32_t signed_result = sign_ext(result & (0xFFFFFFFF >> (32 - data_size)), data_size);//先符号位扩展再取出符号位
    int32_t signed_src = sign_ext(src & (0xFFFFFFFF >> (32 - data_size)), data_size);
    int32_t signed_dest = sign_ext(dest & (0xFFFFFFFF >> (32 - data_size)), data_size);
    unsigned char res_sign = ((signed_result >> 31) & 0x00000001);
    unsigned char src_sign = ((signed_src >> 31) & 0x00000001);
    unsigned char dest_sign = ((signed_dest >> 31) & 0x00000001);
    if(src_sign == dest_sign)
    {
        cpu.eflags.OF = (dest_sign != res_sign);
    }
    else
    {
        cpu.eflags.OF = 0;
    }
}
void set_OF_sxb(uint32_t result, uint32_t src, uint32_t dest, size_t data_size)//sub和sbb都可以用
{
    int32_t signed_result = sign_ext(result & (0xFFFFFFFF >> (32 - data_size)), data_size);//先符号位扩展再取出符号位
    int32_t signed_src = sign_ext(src & (0xFFFFFFFF >> (32 - data_size)), data_size);
    int32_t signed_dest = sign_ext(dest & (0xFFFFFFFF >> (32 - data_size)), data_size);
    unsigned char res_sign = ((signed_result >> 31) & 0x00000001);
    unsigned char src_sign = ((signed_src >> 31) & 0x00000001);
    unsigned char dest_sign = ((signed_dest >> 31) & 0x00000001);
    if(src_sign != dest_sign)
    {
        cpu.eflags.OF = (dest_sign != res_sign);
    }
    else
    {
        cpu.eflags.OF = 0;
    }
}

uint32_t alu_add(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_add(src, dest, data_size);
#else
    dest &= (0xFFFFFFFF >> (32 - data_size));
    src &= (0xFFFFFFFF >> (32 - data_size));
    uint32_t res = 0;
    res = dest + src;  //获取计算结果
    
    set_CF_add(res, src, data_size); //设置标志位
    set_PF(res);
    set_ZF(res, data_size);
    set_SF(res, data_size);
    set_OF_adx(res, src, dest, data_size);
// 	printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
// 	fflush(stdout);
// 	assert(0);
// 	return 0;
    return res & (0xFFFFFFFF >> (32 - data_size)); // 高位清零并返回
#endif
}

uint32_t alu_adc(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_adc(src, dest, data_size);
#else
    dest &= (0xFFFFFFFF >> (32 - data_size));
    src &= (0xFFFFFFFF >> (32 - data_size));
    uint32_t res = 0;
    res = dest + src + cpu.eflags.CF;//获取计算结果
    
    set_CF_adc(res, src, data_size); //设置标志位
    set_PF(res);
    set_ZF(res, data_size);
    set_SF(res, data_size);
    set_OF_adx(res, src, dest, data_size);
    return res & (0xFFFFFFFF >> (32 - data_size));// 高位清零并返回
// 	printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
// 	fflush(stdout);
// 	assert(0);
// 	return 0;
#endif
}

uint32_t alu_sub(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_sub(src, dest, data_size);
#else
    dest &= (0xFFFFFFFF >> (32 - data_size));
    src &= (0xFFFFFFFF >> (32 - data_size));
    uint32_t res = 0;
    res = dest - src ;
    
    set_CF_sub(res, dest, data_size); //设置标志位
    set_PF(res);
    set_ZF(res, data_size);
    set_SF(res, data_size);
    set_OF_sxb(res, src, dest, data_size);
    return res & (0xFFFFFFFF >> (32 - data_size));// 高位清零并返回
// 	printf("\e[0;31mPlease implement me at sub-alu.c\e[0m\n");
// 	fflush(stdout);
// 	assert(0);
// 	return 0;
#endif
}

uint32_t alu_sbb(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_sbb(src, dest, data_size);
#else
    dest &= (0xFFFFFFFF >> (32 - data_size));
    src &= (0xFFFFFFFF >> (32 - data_size));
    uint32_t res = 0;
    res = dest - src - cpu.eflags.CF;
    
    set_CF_sbb(res, dest, data_size); //设置标志位
    set_PF(res);
    set_ZF(res, data_size);
    set_SF(res, data_size);
    set_OF_sxb(res, src, dest, data_size);
    return res & (0xFFFFFFFF >> (32 - data_size));// 高位清零并返回
// 	printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
// 	fflush(stdout);
// 	assert(0);
// 	return 0;
#endif
}

uint64_t alu_mul(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_mul(src, dest, data_size);
#else
    dest &= (0xFFFFFFFF >> (32 - data_size));
    src &= (0xFFFFFFFF >> (32 - data_size));
    uint64_t res = 0;
    //res = dest * src;//这句乘法操作因为操作数都是32位，所以输出结果只保留32位，然后把得到的1赋值给res，故而出错，要保证操作数都是64位才能得到正确结果。
    res = (uint64_t)dest * src;//只需要提高一个操作数的精度，另一个操作数会跟着提高；
    uint32_t i =0;
    i = res >> data_size;
    if(i != 0)//设置标志位
    {
        cpu.eflags.CF = 1;
        cpu.eflags.OF = 1;
    }
    else
    {
        cpu.eflags.CF = 0;
        cpu.eflags.OF = 0;
    }
    return res;//乘法结果本身就要储存在2*data_size的空间里，不需要再截取高位
// 	printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
// 	fflush(stdout);
// 	assert(0);
// 	return 0;
#endif
}

int64_t alu_imul(int32_t src, int32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_imul(src, dest, data_size);
#else
    src = sign_ext(src, data_size);//先符号位扩展到32位
	dest = sign_ext(dest, data_size);
	
	int64_t res = 0;
	res = (int64_t)src * dest;

    return res;
// 	printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
// 	fflush(stdout);
// 	assert(0);
// 	return 0;
#endif
}

// need to implement alu_mod before testing
uint32_t alu_div(uint64_t src, uint64_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_div(src, dest, data_size);
#else
    dest &= (0xFFFFFFFFFFFFFFFF >> (64 - data_size));
    src &= (0xFFFFFFFFFFFFFFFF >> (64 - data_size));
    uint32_t res = 0;
    assert(src != 0);//当除数为0时程序报错并退出
    
    res = dest / src;//64位的结果直接赋值给32位res，高位自动截断
    return res;//这里只测试了data_size == 32时的结果，因此直接返回即可
// 	printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
// 	fflush(stdout);
// 	assert(0);
// 	return 0;
#endif
}

// need to implement alu_imod before testing
int32_t alu_idiv(int64_t src, int64_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_idiv(src, dest, data_size);
#else
    assert(src != 0);//当除数为0时程序报错并退出
    
	if (data_size == 8)
	{
		int8_t src8_t = src, dest8_t = dest;
		return dest8_t / src8_t;
	}
	else if (data_size == 16)
	{
		int16_t src16_t = src, dest16_t = dest;
		return dest16_t / src16_t;
	}
	else if (data_size == 32)
	{
		int32_t src32_t = src, dest32_t = dest;
		return dest32_t / src32_t;
	}
	else
	{
	    return (int32_t)dest / src;
	}
// 	printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
// 	fflush(stdout);
// 	assert(0);
// 	return 0;
#endif
}

uint32_t alu_mod(uint64_t src, uint64_t dest)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_mod(src, dest);
#else
    uint32_t res = 0;
    assert(src != 0);//当除数为0时程序报错并退出
    
    res = dest % src;//64位的结果直接赋值给32位res，高位自动截断
    return res;//这里只测试了data_size == 32时的结果，因此直接返回即可
// 	printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
// 	fflush(stdout);
// 	assert(0);
// 	return 0;
#endif
}

int32_t alu_imod(int64_t src, int64_t dest)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_imod(src, dest);
#else
    int32_t res = 0;
    assert(src != 0);//当除数为0时程序报错并退出
    
    res = dest % src;//64位的结果直接赋值给32位res，高位自动截断
    return res;//这里只测试了data_size == 32时的结果，因此直接返回即可
// 	printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
// 	fflush(stdout);
// 	assert(0);
// 	return 0;
#endif
}

uint32_t alu_and(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_and(src, dest, data_size);
#else
    dest &= (0xFFFFFFFF >> (32 - data_size));
    src &= (0xFFFFFFFF >> (32 - data_size));
    uint32_t res = 0;
    res = src & dest;
    
    //设置标志位
    cpu.eflags.CF = 0;
    cpu.eflags.OF = 0;
    set_PF(res);
    set_ZF(res, data_size);
    set_SF(res, data_size);
    return res & (0xFFFFFFFF >> (32 - data_size));// 高位清零并返回
// 	printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
// 	fflush(stdout);
// 	assert(0);
// 	return 0;
#endif
}

uint32_t alu_xor(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_xor(src, dest, data_size);
#else
    dest &= (0xFFFFFFFF >> (32 - data_size));
    src &= (0xFFFFFFFF >> (32 - data_size));
    uint32_t res = 0;
    res = src ^ dest;
    
    //设置标志位
    cpu.eflags.CF = 0;
    cpu.eflags.OF = 0;
    set_PF(res);
    set_ZF(res, data_size);
    set_SF(res, data_size);
    return res & (0xFFFFFFFF >> (32 - data_size));// 高位清零并返回
// 	printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
// 	fflush(stdout);
// 	assert(0);
// 	return 0;
#endif
}

uint32_t alu_or(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_or(src, dest, data_size);
#else
    dest &= (0xFFFFFFFF >> (32 - data_size));
    src &= (0xFFFFFFFF >> (32 - data_size));
    uint32_t res = 0;
    res = src | dest;
    
    //设置标志位
    cpu.eflags.CF = 0;
    cpu.eflags.OF = 0;
    set_PF(res);
    set_ZF(res, data_size);
    set_SF(res, data_size);
    return res & (0xFFFFFFFF >> (32 - data_size));// 高位清零并返回
// 	printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
// 	fflush(stdout);
// 	assert(0);
// 	return 0;
#endif
}

uint32_t alu_shl(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_shl(src, dest, data_size);
#else
    dest &= (0xFFFFFFFF >> (32 - data_size));
    src &= (0xFFFFFFFF >> (32 - data_size));
    if(data_size < src)//分data_size和src的不同大小关系来讨论
    {
        cpu.eflags.CF = 0;
        set_PF(0);
        set_ZF(0, data_size);
        set_SF(0, data_size);
        return 0;
    }
    else
    {
        cpu.eflags.CF = (dest >> (data_size - src)) & 0x00000001;//先设置CF
    
        uint32_t res = 0;
        res = dest << src;
        
        //设置其他标志位
        set_PF(res);
        set_ZF(res, data_size);
        set_SF(res, data_size);
        return res & (0xFFFFFFFF >> (32 - data_size));// 高位清零并返回
    }
// 	printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
// 	fflush(stdout);
// 	assert(0);
// 	return 0;
#endif
}

uint32_t alu_shr(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_shr(src, dest, data_size);
#else
    dest &= (0xFFFFFFFF >> (32 - data_size));
    src &= (0xFFFFFFFF >> (32 - data_size));//如果这里不加这两句截断操作，那么dest = 0xFFFFFFFF的测试用例就无法通过
    //因为src较小时移位前后的res低八位都是FF，所以移位的结果res一直是0x000000FF即255
    //在这里发现之后我在所有的alu函数里都先加入了对dest 和src 的截断操作；
    if(data_size < src)
    {
        cpu.eflags.CF = 0;
        set_PF(0);
        set_ZF(0, data_size);
        set_SF(0, data_size);
        return 0;
    }
    else
    {
        cpu.eflags.CF = (dest >> (src - 1)) & 0x00000001;
    
        uint32_t res = 0;
        res = dest >> src;
        //设置其他标志位
        set_PF(res);
        set_ZF(res, data_size);
        set_SF(res, data_size);
        return res & (0xFFFFFFFF >> (32 - data_size));// 高位清零并返回
    }
// 	printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
// 	fflush(stdout);
// 	assert(0);
// 	return 0;
#endif
}

uint32_t alu_sar(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_sar(src, dest, data_size);
#else
    int32_t signed_dest = sign_ext(dest & (0xFFFFFFFF >> (32 - data_size)) , data_size);//进行符号扩展并把结果储存在int32_t类型的变量里
    unsigned char dest_sign = ((signed_dest >> 31) & 0x00000001);
    if(data_size < src)
    {
        cpu.eflags.CF = dest_sign;
        set_PF(0xFFFFFFFF);
        set_ZF(0xFFFFFFFF, data_size);
        set_SF(0xFFFFFFFF, data_size);
        return 0;
    }
    else
    {
        cpu.eflags.CF = (signed_dest >> (src - 1)) & 0x00000001;
    
        uint32_t res = 0;
        int32_t signed_res = signed_dest >> src;
        res = signed_res & (0xFFFFFFFF >> (32 - data_size));
        //设置其他标志位
        set_PF(res);
        set_ZF(res, data_size);
        set_SF(res, data_size);
        return res;
    }
// 	printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
// 	fflush(stdout);
// 	assert(0);
// 	return 0;
#endif
}

uint32_t alu_sal(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_sal(src, dest, data_size);
#else
    dest &= (0xFFFFFFFF >> (32 - data_size));
    src &= (0xFFFFFFFF >> (32 - data_size));
    if(data_size < src)
    {
        cpu.eflags.CF = 0;
        set_PF(0);
        set_ZF(0, data_size);
        set_SF(0, data_size);
        return 0;
    }
    else
    {
        cpu.eflags.CF = (dest >> (data_size - src)) & 0x00000001;//先设置CF
    
        uint32_t res = 0;
        res = dest << src;
        
        //设置其他标志位
        set_PF(res);
        set_ZF(res, data_size);
        set_SF(res, data_size);
        return res & (0xFFFFFFFF >> (32 - data_size));// 高位清零并返回
    }
// 	printf("\e[0;31mPlease implement me at alu.c\e[0m\n");
// 	fflush(stdout);
// 	assert(0);
// 	return 0;
#endif
}
