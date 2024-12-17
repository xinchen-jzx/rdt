#include "rdt.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
using namespace std;

// 发送端
Sender::Sender(Address receiver_addr) {
    strcpy(this->receiver_addr.ip, receiver_addr.ip);
    this->receiver_addr.port = receiver_addr.port;
}

void Sender::rdt_send(char* data) {
    cout << "rdt_send: " << data << endl;
    Sender::make_pkt(data);
    Sender::udt_send(data);
}

std::string Sender::make_pkt(std::string data)
{
    return data;
}

void Sender::udt_send(std::string &data)
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        cerr << "Error creating socket" << endl;
        return;
    }

    sockaddr_in remote_addr; // 网络地址结构体
    memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;                       // 设置为IP通信
    remote_addr.sin_addr.s_addr = inet_addr("127.0.0.1");   // 设置IP地址
    remote_addr.sin_port = htons(this->receiver_addr.port); // 设置端口号

    if (sendto(sockfd, data.data(), data.length(), 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) < 0)
    {
        std::cerr << "Error sending packet" << std::endl;
    }

    close(sockfd);
}

// 接收端
Receiver::Receiver(int port)
{
    this->port = port;
}

std::string Receiver::udt_recv()
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        cerr << "Error creating socket" << endl;
        return NULL;
    }

    sockaddr_in serv_addr; // 网络地址结构体
    sockaddr_in remote_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; // 设置为IP通信
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    ;                                       // 设置IP地址
    serv_addr.sin_port = htons(this->port); // 设置端口号

    // 将套接字绑定到服务器的网络地址上
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) < 0)
    {
        perror("bind error");
        return NULL;
    }
    socklen_t sin_size = sizeof(struct sockaddr_in);
    int len;

    char buffer[1024];
    // len = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&remote_addr, &sin_size);
    // cout << "buffer: " << buffer << endl;

    // 接收客户端的数据并将其发送给客户端 -- recvfrom是无连接的
    if ((len = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&remote_addr, &sin_size)) < 0)
    {
        
        perror("recvfrom error");
        return NULL;
    }
    buffer[len] = '\0';
    close(sockfd);
    std::string data(buffer);
    cout << "data:" << data.data() << ", " << data << endl;
    return data;
}

std::string Receiver::extract(std::string data)
{
    return data;
}

std::string Receiver::deliver_data()
{
    string packet = udt_recv();
    string data = extract(packet);
    return data;
}