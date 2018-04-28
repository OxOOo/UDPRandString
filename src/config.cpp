#include "config.h"
#include "aes.h"
#include <yaml-cpp/yaml.h>
#include <cassert>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <ctime>

using namespace std;

Config::Config()
{

}

Config::~Config()
{

}

void Config::Init(const string& filename)
{
    YAML::Node config = YAML::LoadFile(filename);
    
    assert(config["SERVER_ADDRESS"] && config["SERVER_ADDRESS"].IsScalar());
    SERVER_ADDRESS = config["SERVER_ADDRESS"].as<string>();

    assert(config["SERVER_PORT"] && config["SERVER_PORT"].IsScalar());
    SERVER_PORT = config["SERVER_PORT"].as<int>();

    assert(config["SECRET_KEY"] && config["SECRET_KEY"].IsScalar());
    SECRET_KEY = config["SECRET_KEY"].as<string>();
    
    assert(config["MTU"] && config["MTU"].IsScalar());
    MTU = config["MTU"].as<int>();

    // calc aes key
    string key_string = "";
    while(key_string.length() < sizeof(aes_key)) key_string += SECRET_KEY;
    memset(aes_key, 0, sizeof(aes_key));
    for(int i = 0; i < (int)key_string.length(); i ++) {
        aes_key[i] ^= (uint8_t)key_string[i];
    }
}

Sign Config::CreateSign() const
{
    Sign s;
    s.timestamp = htonl(time(NULL));
    for(int i = 0; i < (int)sizeof(s.rands); i ++) {
        s.rands[i] = rand() % 256;
    }

    aes_encode(aes_key, sizeof(aes_key), (uint8_t*)&s, sizeof(Sign)/2, s.encrypted);

    return s;
}

Config::SignError Config::CheckSign(Sign s) const
{
    uint8_t encrypted[sizeof(s.encrypted)];
    aes_encode(aes_key, sizeof(aes_key), (uint8_t*)&s, sizeof(Sign)/2, encrypted);
    if (memcmp(encrypted, s.encrypted, sizeof(s.encrypted)) != 0) return ERR_ENCRYPT;

    s.timestamp = ntohl(s.timestamp);
    if (abs((long long)s.timestamp - (long long)time(NULL)) > 10) return ERR_TIME;

    return ERR_NONE;
}

data_t Config::AESEncode(data_t data) const
{
    data_t d = CreateData();
    d->resize(data->size());

    aes_encode(aes_key, sizeof(aes_key), data->data(), data->size(), d->data());

    return d;
}

data_t Config::AESDecode(data_t data) const
{
    data_t d = CreateData();
    d->resize(data->size());

    aes_decode(aes_key, sizeof(aes_key), data->data(), data->size(), d->data());

    return d;
}