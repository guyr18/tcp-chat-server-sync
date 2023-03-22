# TCP Chat Server (Synchronous)

As the title suggests, this repository represents a very simple implementation of a synchronous, multi-threaded TCP Chat Server. 
This application was written in **C++** and **Boost**, primarily inheriting it's networking capabilities from the **Boost.Asio** 
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

The message terminator is always the ';' character. When reading from a particular socket stream, this allows the application determine the end
of a packet. This is important, because we could endlessly read overlapping packets as ping packets are sent quite frequently for real-time 
connectivity checks.    
