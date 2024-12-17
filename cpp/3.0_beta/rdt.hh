/*
    RDT based on UDP
    rdt3.0_beta: base class
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
#include <arpa/inet.h>

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

class Base
{
protected:
    Address addr;
    Address paddr;

    sockaddr sock_addr;
    sockaddr sock_paddr;

    int sockfd;

    int send_seq;
    int recv_seq;

    uint8_t send_buffer[BUFFER_SIZE];
    uint8_t recv_buffer[BUFFER_SIZE];

    bool set_sin_addr(sockaddr_in *sin_addr, const Address &addr);

    double sim_bit_err_rate;
    double sim_pkt_loss_rate;
    double sim_delay_time; // secs

    double timeout;// default 0.3
public:
    Base(const Address addr, const Address paddr);
    ~Base();

    bool rdt_send(const uint8_t *data, const int dsize);
    int rdt_recv(uint8_t *buffer, const int db_size);

    void udt_send(const Pocket *send_pkt_p);
    bool udt_recv(const uint8_t *recv_buffer);

    void set_bit_err_rate(const double rate);
    void set_pkt_loss_rate(const double lrate);
    void set_delay_time(const double time);
    void set_recv_time(const double time);
    void set_timeout(const double time);
};

// class Sender: public Base {

// }