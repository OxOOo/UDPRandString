#include "udp_client.h"
#include <plog/Log.h>
#include <iostream>
#include "crc.h"

UDPClient::UDPClient(const Config& config) : UDPBase(config)
{
    sockfd = -1;
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

    int req_seq = 0;

    while(true)
    {
        cout << "input a number:";
        int number;
        cin >> number;
        if (number <= 0) {
            cout << "number must > 0" << endl;
            continue;
        }
        if (number > 10*1024*1024) {
            cout << "number must <= " << 10*1024*1024 << endl;
            continue;
        }

        Request req;
        req.req_version = 1;
        req.req_size = number;
        req.req_seq = ++req_seq;
        HTON_INT(req);

        SendPacket(CreateData(&req, sizeof(Request)));

        data_t chars = nullptr;
        vector<vector<int> > groups_mark;
        vector<int> groups_recved;
        vector<data_t> groups_xorsum;
        int remain_groups;
        while(true)
        {
            if (!Readable_timeo(sockfd, 1)) { // 如果1秒钟内都没有收到,则认为断线了
                cout << "error : no response from server" << endl;
                break;
            }
            data_t data = RecvPacket();
            if (data->size() < sizeof(Response)) {
                LOG_WARNING << "recv a small packet";
                continue;
            }
            Response res = *(Response*)data->data();
            NTOH_INT(res);
            if (res.res_version != 1) {
                LOG_WARNING << "wrong version";
                continue;
            }
            if (res.res_seq != req_seq) {
                LOG_WARNING << "wrong res_seq";
                continue;
            }
            if (res.chunk_size + sizeof(Response) != data->size()) {
                LOG_WARNING << "wrong packet size";
                continue;
            }

            if (chars == nullptr) {
                chars = CreateData();
                chars->resize(res.res_size);
                remain_groups = res.group_count;
                groups_mark.resize(res.group_count);
                groups_recved.resize(res.group_count);
                groups_xorsum.resize(res.group_count);
                for(int i = 0; i < res.group_count; i ++) {
                    groups_mark[i].clear();
                    groups_recved[i] = 0;
                    groups_xorsum[i] = nullptr;
                }
            }
            if (groups_recved[res.group_id] == 0) {
                groups_mark[res.group_id].resize(res.chunk_count);
                for(int i = 0; i < res.chunk_count; i ++) {
                    groups_mark[res.group_id][i] = 0;
                }
            }

            if (res.chunk_id == res.chunk_count) {
                groups_xorsum[res.group_id] = CreateData(data->data()+sizeof(Response), res.chunk_size);
            } else {
                groups_mark[res.group_id][res.chunk_id] = 1;
                memcpy(chars->data()+res.chunk_curr_offset, data->data()+sizeof(Response), res.chunk_size);
            }
            groups_recved[res.group_id] ++;
            
            // 如果收到了足够多的chunk则该group接受完毕
            if (groups_recved[res.group_id] == res.chunk_count) {
                remain_groups --;

                if (groups_xorsum[res.group_id] != nullptr) { // 差错校验
                    int loss_chunk_id;
                    for(loss_chunk_id = 0; groups_mark[res.group_id][loss_chunk_id]; loss_chunk_id ++);
                    int loss_offset = res.group_curr_offset + loss_chunk_id*res.chunk_size;
                    memcpy(chars->data()+loss_offset, groups_xorsum[res.group_id]->data(), res.chunk_size);
                    for(int id = 0; id < res.chunk_count; id ++) {
                        int offset = res.group_curr_offset + id*res.chunk_size;
                        if (id != loss_chunk_id) {
                            for(int i = 0; i < res.chunk_size; i ++) {
                                chars->at(loss_offset+i) ^= chars->at(offset+i);
                            }
                        }
                    }
                }
            }

            if (remain_groups == 0) {
                chars->resize(number+1);
                chars->at(number) = '\0';
                cout << (char*)chars->data() << endl; // recv success
                break;
            }
        }
    }
}

data_t UDPClient::RecvPacket()
{
    uint8_t* buf = new uint8_t[config.MTU];
    size_t n = Recvfrom(sockfd, buf, config.MTU, 0, NULL, NULL);

    data_t payload = Packet2Payload(buf, n);

    delete[] buf;
    return payload;
}

void UDPClient::SendPacket(data_t payload)
{
    data_t finalize = Payload2Packet(payload);
    Sendto(sockfd, finalize->data(), finalize->size(), 0, (SA*)&servaddr, sizeof(servaddr));
}
