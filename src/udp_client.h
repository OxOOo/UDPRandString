#ifndef _UDP_CLIENT_H_
#define _UDP_CLIENT_H_

#include <unp.h>
#include "config.h"
#include "common.h"
#include "udp_base.h"

class UDPClient : public UDPBase
{
public:
    UDPClient(const Config& config);
    ~UDPClient();

    void Run();

private:
    int sockfd;
    struct sockaddr_in servaddr;

    void SendPacket(data_t data);
};

#endif // _UDP_CLIENT_H_
