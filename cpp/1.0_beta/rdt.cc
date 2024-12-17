#include "rdt.hh"

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
using namespace std;

Sender::Sender(const Address addr, const Address paddr) : addr(addr), paddr(paddr)
{
    this->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (this->sockfd == -1)
    {
        std::cerr << "Failed to create UDP socket." << std::endl;
        return;
    }

    memset(&(this->sock_addr), 0, sizeof(this->sock_addr));

    sockaddr_in *sin_addr_p = (struct sockaddr_in *)&this->sock_addr; // intenet socket pointer
    sin_addr_p->sin_family = AF_INET;                                 // 设置为IP通信
    sin_addr_p->sin_addr.s_addr = inet_addr(addr.ip.data());          // 设置IP地址
    sin_addr_p->sin_port = htons(addr.port);                          // 设置端口号

    memset(&this->sock_paddr, 0, sizeof(this->sock_paddr));

    sockaddr_in *sin_paddr_p = (struct sockaddr_in *)&this->sock_paddr; // intenet socket pointer
    sin_paddr_p->sin_family = AF_INET;                                  // 设置为IP通信
    sin_paddr_p->sin_addr.s_addr = inet_addr(paddr.ip.data());          // 设置IP地址
    sin_paddr_p->sin_port = htons(paddr.port);                          // 设置端口号

    if (bind(this->sockfd, &(this->sock_addr), sizeof(this->sock_addr)) < 0)
    {
        std::cerr << "Failed to bind UDP socket." << std::endl;
        return;
    }
}

Sender::~Sender()
{
    close(this->sockfd);
}

bool Sender::rdt_send(string &data)
{
    if (sendto(this->sockfd, data.data(), data.length(), 0, &(this->sock_paddr), sizeof(this->sock_paddr)) < 0)
    {
        std::cerr << "Error sending packet" << std::endl;
        return false;
    }

    return true;
}

Receiver::Receiver(const Address addr, const Address paddr) : addr(addr), paddr(paddr)
{
    this->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (this->sockfd == -1)
    {
        std::cerr << "Failed to create UDP socket." << std::endl;
        return;
    }
    memset(&(this->sock_addr), 0, sizeof(this->sock_addr));

    sockaddr_in *sin_addr_p = (struct sockaddr_in *)&this->sock_addr; // intenet socket pointer
    sin_addr_p->sin_family = AF_INET;                                 // 设置为IP通信
    sin_addr_p->sin_addr.s_addr = inet_addr(addr.ip.data());          // 设置IP地址
    sin_addr_p->sin_port = htons(addr.port);                          // 设置端口号

    memset(&this->sock_paddr, 0, sizeof(this->sock_paddr));

    sockaddr_in *sin_paddr_p = (struct sockaddr_in *)&this->sock_paddr; // intenet socket pointer
    sin_paddr_p->sin_family = AF_INET;                                  // 设置为IP通信
    sin_paddr_p->sin_addr.s_addr = inet_addr(paddr.ip.data());          // 设置IP地址
    sin_paddr_p->sin_port = htons(paddr.port);                          // 设置端口号

    if (bind(this->sockfd, &(this->sock_addr), sizeof(this->sock_addr)) < 0)
    {
        std::cerr << "Failed to bind UDP socket." << std::endl;
        return;
    }
}

Receiver::~Receiver()
{
    close(this->sockfd);
}

bool Receiver::rdt_recv(char *buffer, const int size)
{
    socklen_t paddr_len = sizeof(this->sock_paddr);
    int len = recvfrom(this->sockfd, buffer, size, 0, &(this->sock_paddr), &paddr_len);
    if (len < 0)
    {
        std::cerr << "Failed to bind UDP socket." << std::endl;
        return false;
    }
    buffer[len] = 0;

    return true;
}