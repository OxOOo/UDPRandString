#ifndef _UDP_BASE_H_
#define _UDP_BASE_H_

#include "config.h"
#include "common.h"

class UDPBase
{
public:
    UDPBase(const Config& config);
    virtual ~UDPBase();

protected:
    const Config config;

    data_t Payload2Packet(data_t payload);
    data_t Packet2Payload(uint8_t* buf, size_t n);
};

#endif // _UDP_BASE_H_