#include "loop.h"
#include "config.h"
#include "udp_server.h"
#include "udp_client.h"

void loop_server(const string& config_filename)
{
    Config config;
    config.Init(config_filename);

    UDPServer server(config);
    server.Run();
}

void loop_client(const string& config_filename)
{
    Config config;
    config.Init(config_filename);

    UDPClient client(config);
    client.Run();
}
