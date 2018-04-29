#ifndef _COMMON_H_
#define _COMMON_H_

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

// 请求
struct Request
{
    int req_version; // 版本,必须为1
    int req_size;
    int req_seq; // 序列号
};

// 响应,一个任务可能会被分为若干个group,每个group内部做差错校验
struct Response
{
    int res_version; // 版本,必须为1
    int res_seq; // 
    int res_size; // 所有响应的大小,可能会补齐
    int res_real_size; // 真实响应的大小,不补齐

    int group_count;
    int group_id;
    int group_curr_offset, group_curr_size;

    int chunk_count;
    int chunk_id;
    int chunk_size;
    int chunk_curr_offset;
};

#define NTOH_INT(ss) \
{ \
    int* s = (int*)&ss; \
    for(int i = 0; i < (int)sizeof(ss)/4; i ++) { \
        s[i] = ntohl(s[i]); \
    } \
}

#define HTON_INT(ss) \
{ \
    int* s = (int*)&ss; \
    for(int i = 0; i < (int)sizeof(ss)/4; i ++) { \
        s[i] = ntohl(s[i]); \
    } \
}

#endif // _COMMON_H_
