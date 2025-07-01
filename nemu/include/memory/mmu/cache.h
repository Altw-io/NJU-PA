#ifndef __CACHE_H__
#define __CACHE_H__

#include "nemu.h"

#ifdef CACHE_ENABLED


#define BLOCK_SIZE_B 64                                 //缓存块大小64B
#define WAYS 8                                          //8路组相联
#define SETS 128                                        //共128组


typedef struct {
    uint8_t valid; // 有效位
    uint32_t tag; // 标签
    uint8_t data[BLOCK_SIZE_B]; // 缓存数据
} CacheLine;                                            //一个cache行的结构

typedef struct {
    CacheLine cache_group[WAYS];                        //一个8行cache组
} CacheSet;

// cache结构
CacheSet cache[SETS];

// init the cache
void init_cache();

// write data to cache
void cache_write(paddr_t paddr, size_t len, uint32_t data);

// read data from cache
uint32_t cache_read(paddr_t paddr, size_t len);

#endif

#endif
