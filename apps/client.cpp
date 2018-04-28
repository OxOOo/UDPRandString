#include <plog/Log.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <loop.h>

using namespace std;

int main()
{
    static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
    plog::init(plog::debug, &consoleAppender);

    LOG_INFO << "Client Start";
    loop_client("../config.yml");

    return 0;
}
