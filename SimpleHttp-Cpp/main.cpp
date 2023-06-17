/**
@Description:
@Author: xad
*/
#include <iostream>
#include <string>
#include <vector>
#include "Server.h"
using namespace std;

int main(int argc, char const *argv[])
{
    Server server(10000);
    server.init();
    server.run();
    return 0;
}
