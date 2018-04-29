#include "udp_base.h"
#include <plog/Log.h>

UDPBase::UDPBase(const Config& config) : config(config)
{
    crcInit();

    // 检查关键类型的大小,确保内存布局和设计的一致
    assert(sizeof(Sign) == 32);
    assert(sizeof(PacketHeader) == 16);
    assert(sizeof(crc) == 2);
    assert(sizeof(Request) == 12);
    assert(sizeof(Response) == 48);
}

UDPBase::~UDPBase()
{

}

data_t UDPBase::Payload2Packet(data_t payload)
{
    PacketHeader header;

    header.version = htonl(1);
    header.payload_real_size = htonl(payload->size());
    while(payload->size() % 16 != 0) payload->push_back(0);
    header.payload_size = htonl(payload->size());

    payload = config.AESEncode(payload); // 加密载荷
    header.payload_crc = htons(crcFast(payload->data(), payload->size()));

    header.padding = 0; // 此句是不必要的

    header.xorsum = 0;
    uint8_t *h = (uint8_t*)&header;
    for(int i = 0; i < (int)sizeof(PacketHeader)-1; i ++) {
        header.xorsum ^= h[i];
    }

    Sign s = config.CreateSign();

    data_t finalize = CreateData();
    finalize = ConcatData(finalize, CreateData(&header, sizeof(header)));
    finalize = ConcatData(finalize, CreateData(&s, sizeof(s)));
    finalize = ConcatData(finalize, payload);

    if ((int)finalize->size() > config.MTU) {
        LOG_FATAL << "packet size = " << finalize->size() << " is large than MTU = " << config.MTU;
        exit(1);
    }

    return finalize;
}

data_t UDPBase::Packet2Payload(uint8_t* buf, size_t n)
{
    if (n < sizeof(PacketHeader)) {
        LOG_WARNING << "recved a packet, size = " << n << ", small size";
        return nullptr;
    }

    PacketHeader header = *(PacketHeader*)buf;

    header.version = ntohl(header.version);
    header.payload_real_size = ntohl(header.payload_real_size);
    header.payload_size = ntohl(header.payload_size);
    header.payload_crc = ntohs(header.payload_crc);

    if (header.version != 1) {
        LOG_WARNING << "recved a packet, version = " << header.version << ", wrong version";
        return nullptr;
    }

    uint8_t *h = (uint8_t*)&header;
    for(int i = 0; i < (int)sizeof(PacketHeader)-1; i ++) {
        header.xorsum ^= h[i];
    }
    if (header.xorsum != 0) {
        LOG_WARNING << "recved a packet, wrong xorsum";
        return nullptr;
    }

    if (n < sizeof(PacketHeader) + sizeof(Sign)) {
        LOG_WARNING << "recved a packet, size = " << n << ", small size";
        return nullptr;
    }

    Sign s = *(Sign*)(buf+sizeof(PacketHeader));
    Config::SignError err = config.CheckSign(s);
    if (err != Config::ERR_NONE) {
        LOG_WARNING << "recved a packet, sign error = " << err << ", sign error";
        return nullptr;
    }

    if (n != sizeof(PacketHeader) + sizeof(Sign) + header.payload_size) {
        LOG_WARNING << "recved a packet, size = " << n << ", wrong size";
        return nullptr;
    }

    data_t payload = CreateData();
    payload->resize(header.payload_size);
    memcpy(payload->data(), buf+sizeof(PacketHeader)+sizeof(Sign), header.payload_size);
    if (header.payload_crc != crcFast(payload->data(), payload->size())) {
        LOG_WARNING << "recved a packet, crc error";
        return nullptr;
    }
    payload = config.AESDecode(payload);
    payload->resize(header.payload_real_size);

    return payload;
}
