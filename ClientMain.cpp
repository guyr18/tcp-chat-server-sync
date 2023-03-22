#include <iostream>
#include "Client.cpp"
#include <stdlib.h>

using std::cout;
using std::cin;
using std::cerr;
using std::endl;

int main(int argc, char* argv[])
{

    if(argc != 4)
    {

        cerr << "Usage: <host> <port> <nickname>" << endl;
        return 1;

    }

    char* port_ptr;
    cout << argv[0] << endl;
    Client client{argv[1], static_cast<unsigned int>(strtol(argv[2], &port_ptr, 10)), argv[3]};
    client.connect();

    while(client.isConnected())
    {

        string input;
        std::getline(std::cin, input);
        cout << '\r';
        
        if(input != "")
        {

            boost::system::error_code ec;
            client.sendParamToServer("[" + client.getNickname() + "]: " + input + ";", PacketTagTypes::PKT_MESSAGE, ec);

        }
    }
    
    return 0;

}