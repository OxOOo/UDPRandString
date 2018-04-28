#include <yaml-cpp/yaml.h>
#include <iostream>

using namespace std;

int main()
{
    YAML::Node config = YAML::LoadFile("../config.yml");

    cout << "VERSION = " << config["VERSION"] << endl;

    return 0;
}
