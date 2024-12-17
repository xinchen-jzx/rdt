#pragma once
#include <stdio.h>
#include <sys/socket.h>
#include <string>
#include <cstring>
struct Address
{
    std::string ip;
    int port;
};

struct pkt
{
    char checksum[4]; // 32bits checksum
    int seq;          // seq number
    std::string data; // data
};
class Sender
{
private:
    Address addr;
    Address paddr;
    sockaddr sock_addr;
    sockaddr sock_paddr;
    int sockfd;

public:
    Sender(const Address addr, const Address paddr);
    ~Sender();

    bool rdt_send(std::string &data);
};

class Receiver
{
private:
    Address addr;
    Address paddr;
    sockaddr sock_addr;
    sockaddr sock_paddr;
    int sockfd;

public:
    Receiver(const Address addr, const Address paddr);
    ~Receiver();
    bool rdt_recv(char *buffer, const int size);
};