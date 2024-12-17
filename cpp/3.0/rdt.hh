/*
    RDT based on UDP
    rdt3.0:
        - checksum
        - ACK
        - resend
        - timeout
        + random_bits_err
*/

#pragma once
#include <string>
#include <unistd.h>
#include <sys/socket.h>

// in bytes
#define DATA_MAX_SIZE 512 // pocket max data size
#define BUFFER_SIZE 1024  // Sender or Receiver buffer size
#define CHECKSUM_START 4

// can extend
#define ACK_FLAG 1
#define DATA_FLAG 0

// only for rdt 2.2
#define NEXT(x) (1 - x)

// timeout for rdt 3.0
#define TIMEOUT 0.3                   // s
#define RECV_TIMEOUT (0.1 * 1000000)  // us
#define RECV_SLEEP (0.9 * 1000000000) // ns

// Example:
//      Address send_addr = {ip : "127.0.0.1", port : 1234};
struct Address
{
    std::string ip;
    int port;
};

// all = 4 * 4 + 512 = 528 bytes
struct Pocket
{
    uint32_t checksum;           // 32bits checksum
    uint32_t flag;               // flag 1 is ACK
    uint32_t seq;                // seq number
    uint32_t size;               // pocket bytes sum
    uint8_t data[DATA_MAX_SIZE]; // data, bytes stream
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
    uint8_t send_buffer[BUFFER_SIZE];
    uint8_t recv_buffer[BUFFER_SIZE];

public:
    Sender(const Address addr, const Address paddr);
    ~Sender();

    bool rdt_send(const uint8_t *data, const int size);
    void udt_send(const Pocket *send_pkt_p);
    bool udt_recv(const uint8_t *recv_buffer);
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
    uint8_t send_buffer[BUFFER_SIZE];
    uint8_t recv_buffer[BUFFER_SIZE];

public:
    Receiver(const Address addr, const Address paddr);
    ~Receiver();
    bool rdt_recv(uint8_t *buffer, const int db_size);
    void udt_send(const Pocket *send_pkt_p);
    bool udt_recv(const uint8_t *recv_buffer);
};

///// Helper Functions /////



/// @brief calculate pkt checksum
/// @param p Pocket *
/// @return checksum in uint32_t
uint32_t cal_checksum(const Pocket *p);


/// @brief make pkt in buffer
/// @param flag
/// @param seq
/// @param size
/// @param data
/// @param buffer
/// @param buffer_size
/// @return true or false
bool make_pkt_beta(
    const uint32_t flag,
    const uint32_t seq,
    const uint32_t size,
    const uint8_t *data,
    const uint8_t *buffer,
    const int buffer_size);

void print_pkt(Pocket *p);

bool is_corrupt(const Pocket *p);

void random_bits_err(Pocket *&p);