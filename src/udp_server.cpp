#include "udp_server.h"
#include <plog/Log.h>

UDPServer::UDPServer(const Config& config) : UDPBase(config)
{
    sockfd = -1;
    addrlen = 0;

    // 检查关键类型的大小,确保内存布局和设计的一致
    assert(sizeof(Sign) == 32);
    assert(sizeof(PacketHeader) == 16);
    assert(sizeof(crc) == 2);
}

UDPServer::~UDPServer()
{
}

void UDPServer::Run()
{
    sockfd = Udp_server("127.0.0.1", to_string(config.SERVER_PORT).c_str(), &addrlen);
    crcInit();

    SA* cliaddr = (SA*)Malloc(addrlen);
    socklen_t clilen = addrlen;
    data_t data = RecvPacket(cliaddr, &clilen);

    LOG_DEBUG << (char*)data->data();
    LOG_DEBUG << data->size();
}

data_t UDPServer::RecvPacket(SA* pcliaddr, socklen_t* clilen)
{
    uint8_t* buf = new uint8_t[config.MTU];
    size_t n = Recvfrom(sockfd, buf, config.MTU, 0, pcliaddr, clilen);

    data_t payload = Packet2Payload(buf, n);

    delete[] buf;
    return payload;
}
