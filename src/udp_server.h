#ifndef _UDP_SERVER_H_
#define _UDP_SERVER_H_

#include <unp.h>
#include <queue>
#include <memory>
#include "config.h"
#include "common.h"
#include "udp_base.h"

using namespace std;

class UDPServer : public UDPBase
{
public:
    UDPServer(const Config& config);
    ~UDPServer();

    UDPServer(const UDPServer&) = delete;
    const UDPServer operator=(const UDPServer&) = delete;

    void Run();

private:
    struct Task
    {
        data_t cliaddr;
        socklen_t clilen;

        Request req;
        data_t chars;

        int res_size;

        int group_count, group_size;
        int curr_group_id;

        int chunk_count;
        int curr_chunk_id;
        int chunk_size;
    };
    typedef shared_ptr<Task> task_ptr;
    queue<task_ptr> task_que;

    void WaitRW(); // 等待可读或可写
    task_ptr BuildTask(Request req, SA* cliaddr, socklen_t clilen); // 根据请求建立任务
    void ProcessTask(); // 每次取出任务队列的第一个,发送一个chunk,然后将任务放在末尾

private:
    int sockfd;
    socklen_t addrlen;

    data_t RecvPacket(SA* pcliaddr, socklen_t* clilen);
    void SendPacket(data_t payload, const SA* pcliaddr, socklen_t clilen);
};

#endif // _UDP_SERVER_H_
