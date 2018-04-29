#include "udp_server.h"
#include <plog/Log.h>
#include <arpa/inet.h>

UDPServer::UDPServer(const Config& config) : UDPBase(config)
{
    sockfd = -1;
    addrlen = 0;
}

UDPServer::~UDPServer()
{
}

void UDPServer::Run()
{
    sockfd = Udp_server("127.0.0.1", to_string(config.SERVER_PORT).c_str(), &addrlen);
    SA* cliaddr = (SA*)Malloc(addrlen);
    socklen_t clilen = addrlen;

    while(true)
    {
        WaitRW(); // 等待可读或可写

        // process task
        if (!task_que.empty() && Writable_timeo(sockfd, 0)) {
            ProcessTask();
        }

        // read request
        data_t data = nullptr;
        if (task_que.empty() || Readable_timeo(sockfd, 0)) {
            data = RecvPacket(cliaddr, &clilen);
        }
        if (data != nullptr) {
            if (data->size() != sizeof(Request)) {
                // nothing
                continue;
            }
            Request req = *(Request*)data->data();
            NTOH_INT(req);

            if (req.req_version != 1) {
                // nothing
                continue;
            }
            if (req.req_size <= 0) {
                // nothing
                continue;
            }
            if (req.req_size > 10*1024*1024) {
                // nothing
                continue;
            }

            // build task
            task_que.push(BuildTask(req, cliaddr, clilen));
        }
    }
}

data_t UDPServer::RecvPacket(SA* pcliaddr, socklen_t* clilen)
{
    uint8_t* buf = new uint8_t[config.MTU];
    size_t n = Recvfrom(sockfd, buf, config.MTU, 0, pcliaddr, clilen);

    data_t payload = Packet2Payload(buf, n);

    delete[] buf;
    return payload;
}

void UDPServer::SendPacket(data_t payload, const SA* pcliaddr, socklen_t clilen)
{
    data_t finalize = Payload2Packet(payload);
    Sendto(sockfd, finalize->data(), finalize->size(), 0, pcliaddr, clilen);
}

void UDPServer::WaitRW()
{
    fd_set rset, wset;

    FD_ZERO(&rset);
	FD_SET(sockfd, &rset);
	FD_ZERO(&wset);
	FD_SET(sockfd, &wset);

    select(sockfd+1, &rset, &wset, NULL, NULL);
}

UDPServer::task_ptr UDPServer::BuildTask(Request req, SA* cliaddr, socklen_t clilen)
{
    task_ptr task = task_ptr(new Task());
    task->cliaddr = CreateData();
    task->cliaddr->resize(clilen);
    memcpy(task->cliaddr->data(), cliaddr, clilen);
    task->clilen = clilen;

    task->req = req;

    task->res_size = req.req_size;
    while(task->res_size % 10) task->res_size ++; // 10字节对齐,便于拆分chunk

    task->group_size = min(1000, task->res_size);
    task->chunk_count = 10;
    task->chunk_size = task->group_size / task->chunk_count;
    task->curr_chunk_id = 0;

    while(task->res_size % task->group_size) task->res_size ++; // group_size对其
    task->group_count = task->res_size / task->group_size;
    task->curr_group_id = 0;

    task->chars = CreateData();
    task->chars->resize(task->res_size);
    for(int i = 0; i < (int)task->chars->size(); i ++) {
        task->chars->at(i) = 'A' + rand()%26;
    }
    LOG_DEBUG << (char*)task->chars->data();

    return task;
}

// 每次取出任务队列的第一个,发送一个chunk,然后将任务放在末尾
void UDPServer::ProcessTask()
{
    if (task_que.empty()) return;
    task_ptr task = task_que.front(); task_que.pop();

    Response res;
    res.res_version = 1;
    res.res_seq = task->req.req_seq;
    res.res_size = task->res_size;
    res.res_real_size = task->req.req_size;
    res.group_count = task->group_count;
    res.group_id = task->curr_group_id;
    res.group_curr_size = task->group_size;
    res.group_curr_offset = task->group_size*task->curr_group_id;

    res.chunk_count = task->chunk_count;
    res.chunk_id = task->curr_chunk_id;
    res.chunk_size = task->chunk_size;
    res.chunk_curr_offset = task->group_size*task->curr_group_id + task->chunk_size*task->curr_chunk_id;

    data_t data = CreateData();
    data->resize(res.chunk_size);
    memset(data->data(), 0, data->size());
    if (res.chunk_id != res.chunk_count) {
        memcpy(data->data(), task->chars->data()+res.chunk_curr_offset, res.chunk_size);
    } else {
        for(int i = 0; i < res.chunk_count; i ++) {
            int s = task->group_size*task->curr_group_id + task->chunk_size*i;
            int t = s + task->chunk_size;
            for(int j = s; j < t; j ++) {
                data->at(j-s) ^= task->chars->at(j);
            }
        }
    }

    // Response全部有int组成,转成网络序
    HTON_INT(res);

    data_t payload = ConcatData(CreateData(&res, sizeof(res)), data);

    SendPacket(payload, (const SA*)task->cliaddr->data(), task->clilen);

    task->curr_chunk_id ++;
    if (task->curr_chunk_id > task->chunk_count) {
        task->curr_chunk_id = 1;
        task->curr_group_id ++;
    }

    if (task->curr_group_id < task->group_count) {
        task_que.push(task);
    }
}
