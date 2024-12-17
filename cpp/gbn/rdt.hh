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
#include <thread>

// in bytes
#define DATA_MAX_SIZE 512 // pocket max data size
#define BUFFER_SIZE 1024  // Sender or Receiver buffer size
#define CHECKSUM_START 4

// can extend
#define ACK_FLAG 1
#define DATA_FLAG 0

// winsize
#define WINDOW_SIZE 10

// #define NEXT(x) ((x + 1) % WINDOW_SIZE)
// #define PRE(x) ((x - 1 + WINDOW_SIZE) % WINDOW_SIZE)
#define NEXT(x) (x + 1)
#define PRE(x) (x - 1)
#define INDEX(x) (x % WINDOW_SIZE)
// Example:
//      Address send_addr = {ip : "127.0.0.1", port : 1234};
struct Address
{
    std::string ip;
    int port;
};

// all = 4 * 4 + 512 = 528 bytes
struct Packet
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

    int base;     // init 0
    int nseq;     // init 0
    // for receiver
    int eseq;

    Packet pkt_buf[WINDOW_SIZE + 1]; // 0 empty

    uint8_t send_buffer[BUFFER_SIZE];
    uint8_t recv_buffer[BUFFER_SIZE];

    double sim_bit_err_rate;
    double sim_pkt_loss_rate;
    double sim_delay_time; // secs <1.0!!

    double timeout; // default 0.3s

    // if start time < 0, means stop the timer
    // euqal to pkt buffer is empty,
    // if start time > 0, pkt buffer must not empty
    double start_time;

    std::thread listen_th;
    bool listen_stop; // tell listen thread that send task is over
    // just need to finish current work

    bool set_sin_addr(sockaddr_in *sin_addr, const Address &addr);

public:
    Base(const Address addr, const Address paddr);
    ~Base();

    void listen();
    void start_listen();
    void stop_listen();
    bool rdt_send(const uint8_t *data, const int dsize);
    int rdt_recv(uint8_t *buffer, const int db_size);

    void udt_send(const Packet *send_pkt_p);
    bool udt_recv(const uint8_t *recv_buffer);

    void set_bit_err_rate(const double rate);
    void set_pkt_loss_rate(const double lrate);
    void set_delay_time(const double time);
    void set_recv_time(const double time);
    void set_timeout(const double time);
};

// class Sender: public Base {

// }