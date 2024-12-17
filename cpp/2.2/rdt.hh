/*
    RDT based on UDP
    rdt2.2:
        - checksum
        - ACK
        - resend
        + random_bits_err
*/

#pragma once
#include <string>
#include <unistd.h>
#include <sys/socket.h>

#define DATA_BUFFER_SIZE 512
#define CHECKSUM_START 4
#define RECV_BUFFER_SIZE 1024
#define ACK_FLAG 1
#define DATA_FLAG 0

#define NEXT(x) (1 - x)

struct Address
{
    std::string ip;
    int port;
};

// all = 4 * 4 + 512 = 528 bytes
struct Pocket
{
    uint32_t checksum;              // 32bits checksum
    uint32_t flag;                  // flag 1 is ACK
    uint32_t seq;                   // seq number
    uint32_t size;                  // pocket bytes sum
    uint8_t data[DATA_BUFFER_SIZE]; // data, bytes stream
};

// Example:
//      Address send_addr = {ip : "127.0.0.1", port : 1234};
//      Address recv_addr = {ip : "127.0.0.1", port : 4321};
//      Sender sender = Sender(send_addr, recv_addr);
//      sender.rdt_send("hello world!");
class Sender
{
private:
    Address addr;
    Address paddr;
    sockaddr sock_addr;
    sockaddr sock_paddr;
    int sockfd;
    int seq;

public:
    Sender(const Address addr, const Address paddr);
    ~Sender();

    bool rdt_send(const uint8_t *data, const int size);
};
// Example:
//      Address send_addr = {ip : "127.0.0.1", port : 1234};
//      Address recv_addr = {ip : "127.0.0.1", port : 4321};
//      Receiver receiver = Receiver(recv_addr, send_addr);
//      char buffer[1024];
//      receiver.rdt_recv(buffer, 1024);
class Receiver
{
private:
    Address addr;
    Address paddr;
    sockaddr sock_addr;
    sockaddr sock_paddr;
    int sockfd;
    int seq;

public:
    Receiver(const Address addr, const Address paddr);
    ~Receiver();
    bool rdt_recv(uint8_t *buffer, const int db_size);
};

///// Helper Functions /////
uint32_t cal_checksum(const Pocket *p);

bool make_pkt(
    const uint32_t flag,
    const uint32_t seq,
    const uint32_t size,
    const uint8_t *data,
    Pocket *&p);

bool is_corrupt(const Pocket *p);

void random_bits_err(Pocket *&p);