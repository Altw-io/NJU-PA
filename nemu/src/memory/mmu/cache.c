#include "memory/mmu/cache.h"
#include "memory/memory.h"
#include <time.h>
#include <stdlib.h>

#ifdef CACHE_ENABLED
// init the cache
void init_cache()
{
    srand(time(0));
	for(int i = 0; i < SETS; i++)
	    for(int j = 0; j < WAYS; j ++)
	    {
	        cache[i].cache_group[j].valid = 0;
	    }
}

// write data to cache
void cache_write(paddr_t paddr, size_t len, uint32_t data)
{
    uint32_t block_address = paddr & 0x3f;
    
    if(block_address + len > BLOCK_SIZE_B)//把前一部分的写进去再把后一部分的写进去，然后拼起来
    {
        cache_write(paddr, BLOCK_SIZE_B - block_address, data & ((1 << ((BLOCK_SIZE_B - block_address) << 3)) - 1));
        cache_write(paddr + BLOCK_SIZE_B - block_address, block_address + len - BLOCK_SIZE_B,  data >> ((BLOCK_SIZE_B - block_address) << 3));
    }
    else
    {
        uint32_t group_order = (paddr >> 6) % SETS;
        uint32_t tag = (paddr >> 6) / SETS;
        
        int i = 0;
        for(i = 0; i < WAYS; i++)
        {
            if(cache[group_order].cache_group[i].valid && tag == cache[group_order].cache_group[i].tag) break;
        }
        
        if(i == WAYS)//未命中
        {
            memcpy(hw_mem + paddr, &data, len);
        }
        else//命中
        {
            memcpy(hw_mem + paddr, &data, len);
            memcpy(cache[group_order].cache_group[i].data + block_address, &data, len);
        }
    }
}

// read data from cache
uint32_t cache_read(paddr_t paddr, size_t len)
{
    uint32_t block_address = paddr & 0x3f;
    
    if(block_address + len > BLOCK_SIZE_B)//把前一部分的取出来再把后一部分的取出来，然后拼起来
    {
        uint32_t val1 = cache_read(paddr, BLOCK_SIZE_B - block_address);
        uint32_t val2 = cache_read(paddr + BLOCK_SIZE_B - block_address, block_address + len - BLOCK_SIZE_B);
        return (val2 << ((BLOCK_SIZE_B - block_address) << 3))  | val1;
    }
    
    uint32_t group_order = (paddr >> 6) % SETS;
    uint32_t tag = (paddr >> 6) / SETS;
    
    int i = 0;
    for(i = 0; i < WAYS; i++)
    {
        if(cache[group_order].cache_group[i].valid && tag == cache[group_order].cache_group[i].tag) break;
    }
    if(i == WAYS)//未命中
    {
        // //判断：是valid全0
        // //还是已满
        // //还是valid=1的部分里没有目标，但仍有valid=0
        // int cnt = 0;
        // for(int j = 0; j < WAYS; j++)
        // {
        //     if(cache[group_order].cache_group[j].valid == 1)
        //     {
        //         cnt ++;
        //     }
        // }
        
        // if(cnt == WAYS - 1)//若已满，valid全1但无匹配，随机替换
        // {
        //     uint32_t replace_random = rand() % WAYS;
        //     cache[group_order].cache_group[replace_random].valid = 1;
        //     cache[group_order].cache_group[replace_random].tag = tag;
        //     memcpy(cache[group_order].cache_group[replace_random].data, hw_mem + paddr, BLOCK_SIZE_B);
            
        //     uint32_t ret = 0;
        //     memcpy(&ret, cache[group_order].cache_group[replace_random].data + block_address, len);
        //     return ret;
        // }
        // else//valid=1的部分里没有目标但仍有valid=0 或 valid全0
        // {
        //     int k = 0;
        //     for(k = 0; k < WAYS; k++)
        //     {
        //         if(cache[group_order].cache_group[k].valid == 0) break;
        //     }
        //     cache[group_order].cache_group[k].valid = 1;
        //     cache[group_order].cache_group[k].tag = tag;
        //     memcpy(cache[group_order].cache_group[k].data, hw_mem + paddr, BLOCK_SIZE_B);  //这里有问题，应该从BLOCK_SIZE_B划分的起始点开始
            
        //     uint32_t ret = 0;
        //     memcpy(&ret, cache[group_order].cache_group[k].data + block_address, len);
        //     return ret;
        // }
        uint32_t replace_random = rand() % WAYS;
        cache[group_order].cache_group[replace_random].valid = 1;
        cache[group_order].cache_group[replace_random].tag = tag;
        memcpy(cache[group_order].cache_group[replace_random].data, hw_mem + (paddr ^ block_address), BLOCK_SIZE_B);
        
        uint32_t ret = 0;
        memcpy(&ret, cache[group_order].cache_group[replace_random].data + block_address, len);
        return ret;
    }
    else//命中
    {
        uint32_t ret = 0;
        memcpy(&ret, cache[group_order].cache_group[i].data + block_address, len);
        return ret;
    }
}
#endif
