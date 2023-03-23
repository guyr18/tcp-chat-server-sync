#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "PacketTagTypes.cpp"

using namespace boost::asio;
using ip::tcp;
using std::string;
using std::cout;
using std::cerr;
using std::endl;

typedef unsigned int uint;

class Client
{

    private:

        string m_hostName; // The host name that this Client object is connected to.
        uint m_portNum; // The port number of the server that this Client object is connected to.
        string m_nickname; // The nickname of this Client object.
        bool m_connected; // The connection status of this Client object.
        boost::scoped_ptr<tcp::socket> m_tcpSocket; // A scoped pointer that refers to the tcp::socket connection of this Client object.

        // Synchronous operations

        // StartPacketRead() is a synchronous blocking method that will block until it encounters an incoming packet.
        // It then invokes the appropriate methods to process and handle each packet.
        void startPacketRead()
        {

            // If this socket is inactive, it cannot possibly receive any incoming packets.
            // So we check to be sure.
            if(m_tcpSocket.get() == nullptr || !isConnected()) { return; }
            
            // Declare a buffer of characters to synchronously receive any incoming packets.
            char buf[256];

            try
            {

                (*m_tcpSocket).receive(boost::asio::buffer(buf));
                handlePacketRead(buf);

            }
            catch(const std::exception& err) {

                cout << endl;
                disconnect();

            }   

            // Recursively invoke this method.
            startPacketRead();

        }

        // HandlePacketRead(buf) handles the data that was synchronously read into buf. It will logically determine
        // if it is important data that the user should see.
        void handlePacketRead(const char* buf)
        {
            
            std::string data(buf); // Convert c-style buffer to a std::string
            const string& tag = data.substr(0, 3); // The packet tag is always the first 3 characters; so let's extract that.

            // We are only interested in displaying messages to the user. We want to ignore other
            // packet types such as ping checks, etc.
            if(tag == PacketTagTypes::PKT_PING || tag == PacketTagTypes::PKT_NICKNAME) { return; }

            // Here we need to find the index of the final semicolon so we can print the contents
            // of the message that precede it.
            uint terminatorIndex = data.length() - 1; // Start at the last index of the string.

            // Run a busy loop that backtracks until it encounters the first ';' character.
            while(data[terminatorIndex] != ';') 
            {

                terminatorIndex--;

            } 

            // Store the content as a substring and print to the standard output stream.
            const string& content = data.substr(3, terminatorIndex - 3);
            cout << content << endl;

        }

    public:


        // Suppress copy semantics.
        Client(const Client& rhs) = delete;
        Client& operator=(const Client& rhs) = delete;

        // Suppress move semantics.
        Client(const Client&& rhs) = delete;
        Client& operator=(const Client&& rhs) = delete;

        // Two-parameter constructor that initializes all properties of this Client object.
        explicit Client(const string& host, const uint& port, const string& nickname) : m_hostName{host}, m_portNum{port}, m_nickname{nickname}, m_tcpSocket{nullptr} {}

        // Destructor to cleanup memory in relation to m_tcpSocket.
        ~Client()
        {

            if(m_tcpSocket.get() != nullptr)
            {

                m_tcpSocket.reset();

            }
        }

        // Connect() trys to establish a TCP Connection at host, m_hostName, and port, m_portNum.
        void connect()
        {

            try
            {
                
                io_service ios;
                m_tcpSocket.reset(new tcp::socket{ios});
                m_tcpSocket->connect(tcp::endpoint(boost::asio::ip::address::from_string(m_hostName), m_portNum));
                cout << "Client successfully connected to [" << m_hostName << ", " << m_portNum << "]" << endl;
                m_connected = true;

                // We need to let the server know what the nickname of this Client is. So send a packet with this information.
                boost::system::error_code param_error;
                sendParamToServer(m_nickname + ";", PacketTagTypes::PKT_NICKNAME, param_error);

                boost::thread syncReadThread{boost::bind(&Client::startPacketRead, this)};

            }
            catch(const std::exception& e)
            {

                cerr << "[Client]: Server is currently down." << endl;
                m_connected = false;

            }
        }
        
        // Disconnect() trys to disconnect from the TCP server at host, m_hostName, and port, m_portNum.
        void disconnect()
        {

            try
            {

                if(m_tcpSocket.get() != nullptr && isConnected())
                {

                    m_tcpSocket->close();
                    cout << "[Client]: Connection to server lost." << endl;
                    m_connected = false;
                    exit(0);

                }
            }
            catch(const std::exception e)
            {

                cerr << e.what() << endl;
            
            }
        }

        // SendParamToServer(message, tag, error) synchronously writes a concatenated buffer of (message + tag) to
        // the server socket. If an error occurs, it will be stored in error.
        void sendParamToServer(const string& message, const string& tag, boost::system::error_code& error)
        {

            // If this socket is inactive, it cannot possibly receive any incoming packets.
            // So we check to be sure.
            if(m_tcpSocket.get() == nullptr || !isConnected()) { return; }

            try
            {

                // Attempt to synchronously write to the server with a message of: tag + message.
                boost::asio::write(*m_tcpSocket, boost::asio::buffer(tag + message), error);

                // Check if an error occurred during socket write, if so we lost connection, so we can clean up
                // our resources on part of the client.
                if(error == boost::asio::error::broken_pipe || error == boost::asio::error::connection_reset)
                {

                    disconnect();

                }

            } 
            catch(const boost::system::system_error& err) {}

        }

        // GetHostName() returns the host name of this Client object.
        const string inline getHostName() const noexcept
        {

            return m_hostName;

        }

        // GetPortNumber() returns the port number of this Client object.
        const uint inline getPortNumber() const noexcept
        {

            return m_portNum;

        }

        // GetNickname() returns the nickname of this Client object.
        const string inline getNickname() const noexcept
        {

            return m_nickname;

        }

        // IsConnected() returns the connection status of this Client object.
        const bool inline isConnected() const noexcept
        {

            return m_connected;

        }
};