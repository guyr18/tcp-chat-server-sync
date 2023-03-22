#include <iostream>
#include "Client.cpp"
#include "Command.cpp"
#include "CommandManager.cpp"
#include "CommandNames.cpp"
#include "Parameter.cpp"
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

    // Build command list for this build of the client.

    // Private messaging command.
    std::vector<Parameter> privateMessageParams;
    privateMessageParams.push_back(Parameter("pm_user"));
    privateMessageParams.push_back(Parameter("pm_content"));
    Command privateMessage{CommandNames::PRIV_MSG, privateMessageParams, "<user> <message>", 2};

    // Append to CommandManager instance.
    CommandManager::getInstance().addCommand(privateMessage);

    // Pass executable arguments to Client object.
    char* port_ptr;
    cout << argv[0] << endl;
    Client client{argv[1], static_cast<unsigned int>(strtol(argv[2], &port_ptr, 10)), argv[3]};
    client.connect();

    // Block while client is connected to TCP server.
    while(client.isConnected())
    {

        string input;
        std::getline(std::cin, input);
        
        if(input != "")
        {

            boost::system::error_code ec;

            // Is this a command? All commands must begin with a '/' character.
            if(input[0] == '/')
            {

                size_t nameEndIndex = 1; // The name must start after our '/' prefix, so set to index 1.

                // Loop until we find the first whitespace or reach the end of the string.
                while(nameEndIndex < input.length() && input[nameEndIndex] != ' ')
                {

                    nameEndIndex++;

                }

                string name = input.substr(1, nameEndIndex - 1); // Store the name of this command.
                string content = input.substr(nameEndIndex); // Store the content following the name.
                bool inputIsValid0 = CommandManager::getInstance().isValidCommandName(name); // Validate command name.
                bool inputIsValid1 = CommandManager::getInstance().inputMatchesCommandParamList(content, name); // Validate command input.
                
                // If the input entered is not valid, inform the client only.
                if(!(inputIsValid0 && inputIsValid1))
                {

                    cout << endl <<  "The command you entered does not exist. Current commands are:" << endl;

                    // Print valid commands to client.
                    for(const Command& cmd : CommandManager::getInstance().getCommandList())
                    {

                        cout << "/" << cmd.getCommandName() << " " << cmd.getCommandUsage() << endl;

                    }

                }
                else
                {

                    client.sendParamToServer(input.substr(nameEndIndex + 1) + ";", PacketTagTypes::PKT_PM, ec);

                }
            }
            else
            {

                client.sendParamToServer("[" + client.getNickname() + "]: " + input + ";", PacketTagTypes::PKT_MESSAGE, ec);

            }
        }
    }
    
    return 0;

}