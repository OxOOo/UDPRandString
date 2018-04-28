#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <string>
#include <stdint.h>
#include <arpa/inet.h>
#include "common.h"

using namespace std;

class Config
{
public:
    Config();
    ~Config();

    enum SignError { ERR_NONE, ERR_TIME, ERR_ENCRYPT };

    string SERVER_ADDRESS;
    int SERVER_PORT;
    string SECRET_KEY;
    int MTU;

    uint8_t aes_key[32];

    void Init(const string& filename);
    
    Sign CreateSign() const;
    SignError CheckSign(Sign s) const;

    data_t AESEncode(data_t data) const;
    data_t AESDecode(data_t data) const;
};

#endif // _CONFIG_H_
