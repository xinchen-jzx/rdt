
#include "rdt.hh"
#include "timer.h" // timer in secs

#include <iostream>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <random>
#include <fcntl.h> // IO control
#include <time.h>  // sleep for ms

using namespace std;

Sender::Sender(const Address addr, const Address paddr) : addr(addr), paddr(paddr)
{
    this->seq = 0;
    this->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (this->sockfd == -1)
    {
        cerr << "Failed to create UDP socket." << std::endl;
        return;
    }

    ///// set unblock mode /////
    // int flags = fcntl(this->sockfd, F_GETFL, 0);
    // fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = RECV_TIMEOUT;
    setsockopt(this->sockfd, SOL_SOCKET, SO_RCVTIMEO, (void *)&timeout, sizeof(timeout));
    ///// set unblock mode /////

    memset(&(this->sock_addr), 0, sizeof(this->sock_addr));

    sockaddr_in *sin_addr_p = (sockaddr_in *)&(this->sock_addr); // intenet socket pointer
    sin_addr_p->sin_family = AF_INET;                                 // 设置为IP通信
    sin_addr_p->sin_addr.s_addr = inet_addr(addr.ip.data());          // 设置IP地址
    sin_addr_p->sin_port = htons(addr.port);                          // 设置端口号

    memset(&this->sock_paddr, 0, sizeof(this->sock_paddr));

    sockaddr_in *sin_paddr_p = (sockaddr_in *)&(this->sock_paddr); // intenet socket pointer
    sin_paddr_p->sin_family = AF_INET;                                  // 设置为IP通信
    sin_paddr_p->sin_addr.s_addr = inet_addr(paddr.ip.data());          // 设置IP地址
    sin_paddr_p->sin_port = htons(paddr.port);                          // 设置端口号

    if (bind(this->sockfd, &(this->sock_addr), sizeof(this->sock_addr)) < 0)
    {
        cerr << "Failed to bind UDP socket." << std::endl;
        return;
    }
}

Sender::~Sender()
{
    close(this->sockfd);
}

bool Sender::rdt_send(const uint8_t *data, const int dsize)
{
    cout << "this->seq: " << this->seq << endl;

    make_pkt_beta(DATA_FLAG,
                  this->seq,
                  dsize,
                  data,
                  this->send_buffer,
                  BUFFER_SIZE);

    Pocket *send_pkt_p = (Pocket *)(this->send_buffer);

    cout << "+ SEND" << endl;
    print_pkt(send_pkt_p);

    udt_send(send_pkt_p);

    /////// start timer //////////
    // time_t start_time = time(NULL);
    double start_time, curr_time;
    GET_TIME(start_time);
    /////// start timer //////////

    while (true)
    {
        GET_TIME(curr_time);
        if (curr_time - start_time > TIMEOUT)
        {
            cout << "Time out: resend pkt_" << send_pkt_p->seq << endl;
            udt_send(send_pkt_p);
            GET_TIME(start_time);
        }

        if (udt_recv(this->recv_buffer) == false)
        {
            continue;
        }

        Pocket *recv_pkt_p = (Pocket *)(this->recv_buffer);
        cout << "- RECV" << endl;
        print_pkt(recv_pkt_p);

        if (is_corrupt(recv_pkt_p)) // corrupt
        {
            cout << "1 corrupt pkt!" << endl;
            udt_send(send_pkt_p);
            continue;
        }
        // not corrupt
        if (recv_pkt_p->flag != ACK_FLAG) // not ACK pkt
        {
            cout << "2 recv pkt is not ACK!" << endl;
            udt_send(send_pkt_p);
            continue;
        }
        // ACK pkt
        if (recv_pkt_p->seq != this->seq) // wrong seq
        {
            cout << "3 wrong ack seq: recv(" << recv_pkt_p->seq << "), await(" << this->seq << ")" << endl;
            udt_send(send_pkt_p);
            continue;
        }
        // right pkt
        cout << "!! right ack seq: recv(" << recv_pkt_p->seq << "), await(" << this->seq << ")" << endl;

        cout << "this->seq: " << this->seq << endl;
        cout << "NEXT(this->seq): " << NEXT(this->seq) << endl;
        this->seq = NEXT(this->seq);

        break;
    }

    return true;
}

Receiver::Receiver(const Address addr, const Address paddr) : addr(addr), paddr(paddr)
{
    this->seq = 0;
    this->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (this->sockfd == -1)
    {
        cerr << "Failed to create UDP socket." << std::endl;
        return;
    }
    memset(&(this->sock_addr), 0, sizeof(this->sock_addr));

    sockaddr_in *sin_addr_p = (struct sockaddr_in *)&(this->sock_addr); // intenet socket pointer
    sin_addr_p->sin_family = AF_INET;                                 // 设置为IP通信
    sin_addr_p->sin_addr.s_addr = inet_addr(addr.ip.data());          // 设置IP地址
    sin_addr_p->sin_port = htons(addr.port);                          // 设置端口号

    memset(&this->sock_paddr, 0, sizeof(this->sock_paddr));

    sockaddr_in *sin_paddr_p = (struct sockaddr_in *)&(this->sock_paddr); // intenet socket pointer
    sin_paddr_p->sin_family = AF_INET;                                  // 设置为IP通信
    sin_paddr_p->sin_addr.s_addr = inet_addr(paddr.ip.data());          // 设置IP地址
    sin_paddr_p->sin_port = htons(paddr.port);                          // 设置端口号

    if (bind(this->sockfd, &(this->sock_addr), sizeof(this->sock_addr)) < 0)
    {
        cerr << "Failed to bind UDP socket." << std::endl;
        return;
    }
}

Receiver::~Receiver()
{
    close(this->sockfd);
}

bool Receiver::rdt_recv(uint8_t *data_buffer, const int db_size)
{

    while (true)
    {
        if (udt_recv(this->recv_buffer) == false)
        {
            continue;
        }
        Pocket *recv_pkt_p = (Pocket *)(this->recv_buffer);
        cout << "- RECV" << endl;
        print_pkt(recv_pkt_p);
        ///////////////////
        // random_bits_err(recv_pkt_p);
        // sleep(1);
        ///////////////////
        if (is_corrupt(recv_pkt_p)) // corrupt
        {
            cout << "1 corrupt pkt! send: ACK_" << NEXT(this->seq) << ", need pkt_" << this->seq << endl;

            make_pkt_beta(ACK_FLAG, NEXT(this->seq), 0, 0, this->send_buffer, BUFFER_SIZE);
            Pocket *send_pkt_p = (Pocket *)(this->send_buffer);

            udt_send(send_pkt_p);

            cout << "+ SEND" << endl;
            print_pkt(send_pkt_p);
            continue;
        }
        // not corrupt
        if (recv_pkt_p->flag != DATA_FLAG) // not data pkt
        {
            cout << "2 recv pkt is not data pkt!" << endl;

            make_pkt_beta(ACK_FLAG, NEXT(this->seq), 0, 0, this->send_buffer, BUFFER_SIZE);
            Pocket *send_pkt_p = (Pocket *)(this->send_buffer);
            udt_send(send_pkt_p);

            cout << "+ SEND" << endl;
            print_pkt(send_pkt_p);
            continue;
        }
        // data pkt
        if (recv_pkt_p->seq != this->seq) // wrong seq
        {
            cout << "3 wrong data seq: recv(" << recv_pkt_p->seq << "), await(" << this->seq << "), send(" << NEXT(this->seq) << ")" << endl;

            make_pkt_beta(ACK_FLAG, NEXT(this->seq), 0, 0, this->send_buffer, BUFFER_SIZE);
            Pocket *send_pkt_p = (Pocket *)(this->send_buffer);
            udt_send(send_pkt_p);

            cout << "+ SEND" << endl;
            print_pkt(send_pkt_p);
            continue;
        }
        // right seq pkt
        if (recv_pkt_p->size > db_size)
        {
            cerr << "data size > db_size!" << endl;
            break;
        }

        /// sleep to trigger timeout ///
        // sleep(1); // 1s
        timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = RECV_SLEEP;
        nanosleep(&ts, NULL);
        /// sleep to trigger timeout ///

        cout << "!! right data pkt: recv(" << recv_pkt_p->seq << "), await(" << this->seq << ")" << endl;

        make_pkt_beta(ACK_FLAG, this->seq, 0, 0, this->send_buffer, BUFFER_SIZE);
        Pocket *send_pkt_p = (Pocket *)(this->send_buffer);

        udt_send(send_pkt_p);

        cout << "+ SEND" << endl;
        print_pkt(send_pkt_p);

        cout << "this->seq: " << this->seq << endl;
        cout << "NEXT(this->seq): " << NEXT(this->seq) << endl;
        this->seq = NEXT(this->seq);

        // copy data to data_buffer
        memcpy(data_buffer, recv_pkt_p->data, recv_pkt_p->size);
        data_buffer[recv_pkt_p->size] = 0; // string end '\0'

        break;
    }

    return true;
}

void Sender::udt_send(const Pocket *send_pkt_p)
{
    if (sendto(this->sockfd,
               (void *)send_pkt_p,
               sizeof(Pocket),
               0,
               &(this->sock_paddr),
               sizeof(this->sock_paddr)) < 0)
    {
        cerr << "Error sending packet" << std::endl;
    }
}

bool Sender::udt_recv(const uint8_t *recv_buffer)
{
    socklen_t paddr_len = sizeof(this->sock_paddr);

    int len = recvfrom(this->sockfd,
                       &(this->recv_buffer),
                       BUFFER_SIZE,
                       0,
                       &(this->sock_paddr),
                       &paddr_len);
    if (len == -1)
    {
        if (errno == EAGAIN)
        {
            // cout << "? recv time out!" << endl;
            return false;
        }
        cerr << "Failed to recv msg." << std::endl;
        return false;
    }
    return true;
}

void Receiver::udt_send(const Pocket *send_pkt_p)
{
    if (sendto(this->sockfd,
               (void *)send_pkt_p,
               sizeof(Pocket),
               0,
               &(this->sock_paddr),
               sizeof(this->sock_paddr)) < 0)
    {
        cerr << "Error sending packet" << std::endl;
    }
}
bool Receiver::udt_recv(const uint8_t *recv_buffer)
{
    {
        socklen_t paddr_len = sizeof(this->sock_paddr);

        int len = recvfrom(this->sockfd,
                           &(this->recv_buffer),
                           BUFFER_SIZE,
                           0,
                           &(this->sock_paddr),
                           &paddr_len);
        if (len == -1)
        {
            if (errno == EAGAIN)
            {
                // cout << "? recv time out!" << endl;
                return false;
            }
            cerr << "Failed to recv msg." << std::endl;
            return false;
        }
        return true;
    }
}

///// Helper Functions /////

bool make_pkt_beta(
    const uint32_t flag,
    const uint32_t seq,
    const uint32_t size,
    const uint8_t *data,
    const uint8_t *buffer,
    const int buffer_size)
{
    if ((size > DATA_MAX_SIZE) || (buffer_size < sizeof(Pocket)))
    {
        // data too big || buffer too small
        return false;
    }
    memset((void *)buffer, 0, buffer_size);

    Pocket *p = (Pocket *)buffer;

    p->flag = flag;
    p->seq = seq;
    p->size = size;
    memcpy(p->data, data, size);

    p->checksum = cal_checksum(p);
    return true;
}

void print_pkt(Pocket *p)
{
    if (is_corrupt(p))
    {
        cout << "pkt corrupt!" << endl;
    }
    cout << "== pkt checksum: " << p->checksum << endl;
    cout << "== pkt seq: " << p->seq << endl;
    cout << "== pkt size: " << p->size << endl;
}

uint32_t cal_checksum(const Pocket *p)
{
    uint32_t checksum = 0;
    uint8_t *pp = (uint8_t *)p;
    for (int i = CHECKSUM_START; i < sizeof(Pocket); i++)
    {
        checksum += pp[i];
    }
    return checksum;
}

bool is_corrupt(const Pocket *p)
{
    if (cal_checksum(p) == p->checksum)
    {
        return false;
    }
    return true;
}

#define RAND_THRES 9

void random_bits_err(Pocket *&p)
{
    random_device r;
    default_random_engine gen(r());
    uniform_int_distribution<int> udist(0, 10);
    int randint = udist(gen);
    // int randint = rand() % 10 + 1;
    // cout << "randint: " << randint << endl;
    if (RAND_THRES < randint)
    {
        return;
    }
    uint8_t *pp = (uint8_t *)p;
    pp[1] = 7 - pp[1];
    return;
}