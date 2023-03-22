#include <iostream>
#include "Server.cpp"
#include <stdlib.h>

using std::cout;
using std::cerr;
using std::endl;

int main(int argc, char* argv[])
{

    if(argc != 3)
    {

        cerr << "Usage: <host> <port>" <<endl;
        return 1;
        
    }

    char* port_ptr;
    cout << argv[0] << endl;
    Server server{argv[1], static_cast<unsigned int>(strtol(argv[2], &port_ptr, 10))};
    server.connect();

    while(server.isConnected());
    
    return 0;

}