#include "udp_client.h"
#include <plog/Log.h>
#include "crc.h"

UDPClient::UDPClient(const Config& config) : UDPBase(config)
{
    sockfd = -1;

    // 检查关键类型的大小,确保内存布局和设计的一致
    assert(sizeof(Sign) == 32);
    assert(sizeof(PacketHeader) == 16);
    assert(sizeof(crc) == 2);
}

UDPClient::~UDPClient()
{
}

void UDPClient::Run()
{
    SA* unused_s;
    socklen_t unused_len;
    sockfd = Udp_client("0.0.0.0", NULL, &unused_s, &unused_len);
    free(unused_s);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(config.SERVER_PORT);
    Inet_pton(AF_INET, config.SERVER_ADDRESS.c_str(), &servaddr.sin_addr);

    crcInit();

    SendPacket(CreateData("Hello World"));

    LOG_DEBUG << "END";
}

void UDPClient::SendPacket(data_t payload)
{
    data_t finalize = Payload2Packet(payload);
    Sendto(sockfd, finalize->data(), finalize->size(), 0, (SA*)&servaddr, sizeof(servaddr));
}
