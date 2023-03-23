#pragma once
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

// USER DEFINED IMPORTS
#include "PacketTagTypes.cpp"

using namespace boost::asio;
using ip::tcp;
using std::string;
using std::cout;
using std::cerr;
using std::endl;

typedef unsigned int uint;

struct User
{

    string host;
    string nickname;
    boost::shared_ptr<tcp::socket> tcpSocket;

    User() = default;
    User(const string& host_, const string& nick_) : host{host_}, nickname{nick_} {}

};

class Server
{

    private:

        string m_hostName; // The hostname that this Server object is running on.
        uint m_portNum; // The port number that this Server object is binded to.
        boost::scoped_ptr<tcp::acceptor> m_acceptor; // TCP acceptor scoped pointer.
        boost::scoped_ptr<io_service> m_ioService; // Scoped pointer to the TCP IO Service that is embedded within @see m_acceptor.
        boost::unordered_map<string, User> userPoolMap; // A hashmap that stores the state of each connected user.

        // StartSyncRead(user) starts a synchronous read to check for any incoming data from the TCP socket that is binded to
        // the user object.
        void startSyncRead(User& user)
        {

            if(m_ioService.get() == nullptr) { return; }

            handleSocketRead(user);
            boost::this_thread::sleep(boost::posix_time::milliseconds(250));
            startSyncRead(user);

        }

        // HandleSocketRead(socketPtr, args) performs a synchronous read on the deferenced value of socketPtr. Additionally,
        // this overload provides args to store miscellaneous data (such as nickname inside the scoped_ptr object).
        void handleSocketRead(boost::shared_ptr<tcp::socket>& socketPtr, boost::scoped_ptr<string>& args)
        {

            boost::asio::streambuf buf;

            try
            {

                boost::asio::read_until(*socketPtr, buf, ";");

            }
            catch(const std::exception& e)
            {

                return;

            }

            string data = boost::asio::buffer_cast<const char*>(buf.data());
            string tag = data.substr(0, 3);

            if(tag == PacketTagTypes::PKT_NICKNAME)
            {

                const string& host = socketPtr->local_endpoint().address().to_string();
                const string& nickname = data.substr(3, data.length() - 4);
                User newUser{host, nickname};
                newUser.tcpSocket = socketPtr;
                userPoolMap.emplace(nickname, newUser);
                args.reset(new string{nickname});

            }
        }

        // HandleSocketRead(user) performs a synchronous read on the TCP socket binded to User object, user.
        void handleSocketRead(User& user)
        {

            boost::asio::streambuf buf;
            boost::shared_ptr<tcp::socket>& socketPtr = user.tcpSocket;

            try
            {

                boost::asio::read_until(*socketPtr, buf, ";");

            }
            catch(const std::exception& e)
            {

                return;

            }

            string data = boost::asio::buffer_cast<const char*>(buf.data());
            string tag = data.substr(0, 3);

            if(tag == PacketTagTypes::PKT_MESSAGE)
            {
                
                string content;

                if(data[data.length() - 1] != ';')
                {

                    uint terminatorIndex = data.length() - 1;

                    while(data[--terminatorIndex] != ';') {}
                                                                  
                    content = data.substr(3, terminatorIndex - 3);

                }
                else
                {

                    content = data.substr(3, data.length() - 4);

                }

                cout << content << endl;
                packetSend_Broadcast("", data);
                
            }
            else if(tag == PacketTagTypes::PKT_PM)
            {

                
                uint nicknameNextWSIndex = 3;

                while(data[nicknameNextWSIndex] != ' ')
                {

                    nicknameNextWSIndex++;

                }

                const string targetNickname = data.substr(3, nicknameNextWSIndex - 3);

                if(userPoolMap.count(targetNickname) == 0)
                {

                    // Alert the user (through unicasting) that issued this command that the targeted user is not online (or connected to the server)
                    // to send a private message.
                    const string& offlineMessage = "User '" + targetNickname + "' is not currently online!";
                    cout << offlineMessage << endl;
                    packetSend_Unicast(user, tag + offlineMessage + ";");

                }
                else
                {

                    // Send the private message to the TCP socket of the correct user through unicasting.
                    const string& targetMessage = "From [" + user.nickname + "]: " + data.substr(nicknameNextWSIndex + 1);
                    cout << targetMessage << endl;
                    packetSend_Unicast(userPoolMap[targetNickname], tag + targetMessage + ";");

                }
            }
        }

        // StartAsyncAccept() configures an asynchronous callback for a future connected client and runs the embedded io_service
        // object.
        void startAsyncAccept()
        {

            if(m_ioService.get() == nullptr) { return; }

            boost::shared_ptr<tcp::socket> clientSocket{new tcp::socket{*m_ioService}};
            m_acceptor->async_accept(*clientSocket, bind(&Server::handleAsyncAccept, this, clientSocket));
            m_ioService->run();

        }

        // HandleAsyncAccept(clientSocket) is a callback for the result of an async_accept call. It requests information from
        // clientSocket and broadcasts a notification message to all other connected clients. 
        void handleAsyncAccept(boost::shared_ptr<tcp::socket>& clientSocket)
        {

            if(clientSocket.get() == nullptr) { return; }

            boost::scoped_ptr<string> nickname_ptr;
            handleSocketRead(clientSocket, nickname_ptr); 
            packetSend_Broadcast(*nickname_ptr, PacketTagTypes::PKT_MESSAGE + "[Server]: " + *nickname_ptr + " joined!;");
            boost::thread readerThread{boost::bind(&Server::startSyncRead, this, userPoolMap[*nickname_ptr])};
            cout << "[Server]: " + *nickname_ptr + " joined!" << endl;
            startAsyncAccept();

        }

        // Synchronous handlers

        // StartSyncPing() broadcasts a synchronous ping packet to all connected clients. It executes every 250ms
        // while the application is running to maintain real-time connectivity updates. 
        void startSyncPing()
        {

            if(m_ioService.get() == nullptr) { return; }

            packetSend_Broadcast("", PacketTagTypes::PKT_PING + ";");
            boost::this_thread::sleep(boost::posix_time::milliseconds(250));
            startSyncPing();

        }

        // Packet casting methods.

        // PacketSend_Unicast(targetClient, message) writes a single packet to the targetClient with a content of message.
        void packetSend_Unicast(User& user, const string& message)
        {

            try
            {

                boost::system::error_code status;
                boost::asio::write(*user.tcpSocket, boost::asio::buffer(message), status);

                if(status == boost::asio::error::connection_reset || status == boost::asio::error::broken_pipe)
                {

                    if(userPoolMap.count(user.nickname) > 0)
                    {

                        userPoolMap[user.nickname].tcpSocket.reset();
                        userPoolMap.erase(user.nickname);
                    }
                }
            }
            catch(const boost::system::system_error& err)
            { }

        }

        // PacketSend_Broadcast(nickname, message) writes a packet containing the content of message to every peer, except the peer
        // named nickname.
        void packetSend_Broadcast(const string& nickname, const string& message)
        {

            std::vector<string> blacklist_nicknames; // A string vector of clients that have lost connection. These users should be removed
                                                     // from userPoolMap and resource handles should appropriately be deallocated.

            // Iterate through key-value pairs contained within userPoolMap.
            for(auto& p : userPoolMap)
            {

                // If the nickname of this user is the one we wish to include (nickname), then skip this iteration.
                if(p.first == nickname) { continue; }

                try
                {

                    // Attempt to synchronously write to this users tcp socket.
                    boost::system::error_code status;
                    boost::asio::write(*p.second.tcpSocket, boost::asio::buffer(message), status);

                    // Check if an error occurred, if so this user should be removed from userPoolMap following this loop.
                    if(status == boost::asio::error::connection_reset || status == boost::asio::error::broken_pipe)
                    {
                        
                        cout << "[Server]: " << p.first << " has left." << endl;
                        blacklist_nicknames.push_back(p.first);

                    }
                }
                catch(const boost::system::system_error& err)
                { }
            }

            // Any user contained in blacklist_nicknames should be removed from the server's cache.
            // Iterate through this list and do this accordingly.
            for(auto itr = blacklist_nicknames.begin(); itr != blacklist_nicknames.end(); itr++)
            {

                const string& nickname = *itr;
               
                if(userPoolMap.count(nickname) > 0)
                {

                    userPoolMap[nickname].tcpSocket.reset();
                    userPoolMap.erase(nickname);

                }
            }
        }

    public:

        // Suppress copy semantics.
        Server(const Server& rhs) = delete;
        Server& operator=(const Server& rhs) = delete;

        // Suppress move semantics.
        Server(const Server&& rhs) = delete;
        Server& operator=(const Server&& rhs) = delete;

        // Two-parameter constructor that accepts a host name and port number as input; these values are
        // initialized to the appropriate variable.
        explicit Server(const string& host, const uint& port) noexcept : m_hostName{host}, m_portNum{port}, m_acceptor{nullptr}, m_ioService{nullptr} {}

        // Destructor for cleaning up resources.
        ~Server()
        {

            m_acceptor.reset();
            m_ioService.reset();

            for(auto& p : userPoolMap)
            {

                if(p.second.tcpSocket.get() != nullptr)
                {

                    p.second.tcpSocket.reset();

                }
            }
        }

        // Connect() trys to establish a connection to host, m_hostName, and port, m_portNum.
        void connect()
        {

            try
            {
                
                
                m_ioService.reset(new io_service);
                m_acceptor.reset(new tcp::acceptor{*m_ioService, tcp::endpoint(boost::asio::ip::address::from_string(m_hostName), m_portNum)});
                cout << "Connection established at [" << m_hostName << ", " << m_portNum << "]" << endl;

                // Start worker thread to check for incoming client connections asynchronously.
                boost::thread asyncAcceptThread{boost::bind(&Server::startAsyncAccept, this)};

                // Start worker thread to ping tcp sockets cached in userPoolMap. If we are unable to ping a user
                // they've lost connection.
                boost::thread syncPingThread{boost::bind(&Server::startSyncPing, this)};

            }
            catch(const std::exception& e)
            {
                cerr << e.what() << '\n';
            }
            
        }

        // Disconnect() trys to disconnect from host, m_hostName, and port, m_portNum.
        void disconnect()
        {

            try
            {

                if(m_acceptor.get() != nullptr)
                {

                    m_acceptor->close();
                    m_acceptor.reset();

                }

                if(m_ioService.get() != nullptr)
                {

                    m_ioService->stop();
                    m_ioService.reset();

                }
            }
            catch(const std::exception e)
            {

                cerr << "[Server]: Server has shutdown." << endl;
            
            }

            cout << "[Server]: Server has shutdown." << endl;

        }

        // GetHostName() returns the host name of this Server object.
        const string inline getHostName() const noexcept
        {

            return m_hostName;

        }

        // GetPortNumber() returns the port number of this Server object.
        const uint inline getPortNumber() const noexcept
        {

            return m_portNum;

        }

        // IsConnected() returns the connection status of this Server object.
        const bool inline isConnected() const noexcept
        {

            return m_acceptor.get() != nullptr && m_acceptor->is_open();

        }

        // GetNumConnections() returns the number of active connections.
        const uint inline getNumConnections() const noexcept
        {

            return userPoolMap.size();

        }
};