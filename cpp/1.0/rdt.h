#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
using namespace std;

#ifndef _RDT_1_
#define _RDT_1_

struct Address
{
    char ip[1024];
    int port;
};

// 发送端 (客户端)
class Sender {
private:
    Address receiver_addr;

public:
    Sender(Address receiver_addr); // 构造函数
    void rdt_send(char* data);
    void make_pkt(char* data);
    void udt_send(char* data);
};

// 接收端 (服务端)
class Receiver
{
private:
    int port;

public:
    Receiver(int port); // 构造函数
    std::string udt_recv();
    std::string extract(std::string data);
    std::string deliver_data();
};

#endif