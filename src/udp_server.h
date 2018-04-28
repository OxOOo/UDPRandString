#ifndef _UDP_SERVER_H_
#define _UDP_SERVER_H_

#include <unp.h>
#include "config.h"
#include "common.h"
#include "udp_base.h"

class UDPServer : public UDPBase
{
public:
    UDPServer(const Config& config);
    ~UDPServer();

    UDPServer(const UDPServer&) = delete;
    const UDPServer operator=(const UDPServer&) = delete;

    void Run();

private:
    int sockfd;
    socklen_t addrlen;

    data_t RecvPacket(SA* pcliaddr, socklen_t* clilen);
};

#endif // _UDP_SERVER_H_
