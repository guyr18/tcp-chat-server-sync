# TCP Chat Server (Synchronous)

As the title suggests, this repository represents a very simple implementation of a synchronous, multi-threaded TCP Chat Server. 
This application was written in **C++** and **Boost 1.81.0**, primarily inheriting it's networking capabilities from the **Boost.Asio** 
framework. Below, I provide implementation insight and provide practical examples showing how you may compile and run this application.  

## What is Synchronous?

In the context of this application, synchronous refers to reading / writing to an arbritrary socket stream by means of a blocking call. In order  
to proceed further, the synchronous operation must be completed entirely. There are no gaps in transmission as you might see in an asynchronous  
implementation.  

# Insight into Packet Transmission  

This application transmits packets through a standardized string format that is expressed in the following diagram; (assumption: char = 1 byte of memory)   
  
  
![Screenshot from 2023-03-22 09-38-48](https://user-images.githubusercontent.com/46636441/226922122-27d152e5-07cf-4e26-bb96-8ea296e3ddc9.png)

I will attempt to reiterate and describe each section as listed in the above diagram.

### Packet Tags  

Packet tags allow the server (and client, in some cases) to determine what the intent or purpose of a specific packet is. Using this information,
it is able to efficiently discard packets that are not viewed as valuable information. At the time of this writing, there are currently three
packet tags.

  **%n%** => This packet tag is used to initially transmit the nickname of a client to the server.  
  **%m%** => This packet tag is used to declare a message that is sent from another client.  
  **%p%** => This packet tag represents a ping packet. These packets are transmitted in a parallel thread that consistently checks for connectivity  
         of each client. I will discuss this in more detail in a later section.  
         
Packet tags will always consist of three characters. **X** is replaced by a single character currently.

### Message Content  

Message content is self-explanatory and contains the content of a particular message. For ping packets, **M = 0** bytes (and is the empty string).  

### Message Terminator  

The message terminator is always the ';' character. When reading from a particular socket stream, this allows the application to determine the end
of a packet. This is important, because we could endlessly read overlapping packets as ping packets are sent quite frequently for real-time 
connectivity checks.  

More generally, it can be said that a single packet is comprised of **M+4** bytes.  

## Multi-Threading Aspect

In this section, I will describe the multi-threading design of this application as it relates to the server and client.

### Server Multi-Threaded Infrastructure

The main thread of the server application is an I/O thread that will block while the server is active and running, displaying new client 
activity as it becomes available. From this thread, two additional threads are spawned:  

  **Asynchronous Accept Thread**: While this application is mostly composed of synchronous networking methods, there is one asynchronous    
  thread running at all times. Given that their are gaps in transmission of users joining the server, we need a non-blocking method  
  to accept a new user at any given time. That is the purpose of this thread. When accepting a new user, the client will send a nickname  
  packet, allowing the server to identify different users. This is stored within an unordered hashmap. Furthermore, a 1:1 thread is  
  spawned for each user. This 1:1 will be discussed next.
  
  **Synchronous 1:1 User Thread**: This thread represents a 1:1 relationship, meaning that for each client we have a single thread to handle 
  synchronous read operations. This allows the server to check for any incoming data that is written from the client socket to the server 
  socket. Moreover,  it is the server's responsibility to transmit this packet to the correct subset of peers (i.e.: unicast, multicast, broadcast).  
  
  **Synchronous Ping Thread**: The responsibility of this thread is to write a ping packet to the TCP socket of each user that is currently cached  
  within the server. If a transmission error occurs, such as a broken pipe or a connection reset, the user can safely be removed from cache as they  
  are no longer connected.  
  
### Client Multi-Threaded Infrastructure  

Similarly, the main thread of the client application is an I/O thread that will block while the client is connected to the TCP server.  
In addition to this, it allows the user to type a message in the chat. This thread will also spawn a single thread that is responsible  
for synchronous reads.  

  **Synchronous Read Thread**: As previously mentioned, the server will transmit packets to the correct subset of peers. If a client  
  receives incoming data from another TCP socket it also needs to be able read that information. This thread processes these packets
  by reading incoming data that has been written to their corresponding TCP socket. It will then display this information to the  
  standard output stream as needed.  
  
## Compilation and Running Process  

This application has only been tested on a **Ubuntu 22.04** OS. The client and server may be compiled from the CLI as follows: 

```g++ ServerMain.cpp -lboost_thread -o smain```

```g++ ClientMain.cpp -lboost_thread -o cmain```

From here, you may run each executable as follows:  

```./smain <host> <port>```

```./cmain <host> <port> <nickname>```  

The server must be running first for a client to successfully connect to it. In the future, I would like to extend functionality  
to include features that include designated commands such as private messaging, user adminstration, and more.  
