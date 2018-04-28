#ifndef _COMMON_H_
#define _COMMON_H_

#undef max
#include <memory>
#include <vector>
#include <cstring>
#include <stdint.h>
#include "crc.h"

using namespace std;

typedef shared_ptr<vector<uint8_t> > data_t;
data_t CreateData();
data_t CreateData(string string_data);
data_t CreateData(const void* ptr, int len);
data_t CloneData(data_t d);
data_t ConcatData(data_t a, data_t b);

// 签名
struct Sign
{
    int timestamp; // 时间戳,4字节
    uint8_t rands[12]; // 随机字符,12字节
    uint8_t encrypted[16]; // 加密后的内容
};

// 包头
struct PacketHeader
{
    int version; // 版本,必须为1
    int payload_size; // 载荷长度,不包括签名,包括16字节补齐
    int payload_real_size; // 载荷真是长度,不包括签名,不包括16字节补齐
    crc payload_crc; // 载荷的crc校验,不包括签名,在载荷加密之后计算

    uint8_t padding;

    uint8_t xorsum;
};

#endif // _COMMON_H_