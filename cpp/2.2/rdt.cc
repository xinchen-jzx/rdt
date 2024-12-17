
#include "rdt.hh"

#include <iostream>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <random>

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
    Pocket *send_pkt_p;
    cout << "this->seq: " << this->seq << endl;
    if (!make_pkt(DATA_FLAG, this->seq, dsize, data, send_pkt_p))
    {
        // make pkt false
        cout << "make pkt false!" << endl;
        return false;
    }
    cout << "+ send checksum: " << send_pkt_p->checksum << endl;
    cout << "+ send seq: " << send_pkt_p->seq << endl;
    cout << "+ send size: " << send_pkt_p->size << endl;

    if (sendto(this->sockfd, (void *)send_pkt_p, sizeof(Pocket), 0, &(this->sock_paddr), sizeof(this->sock_paddr)) < 0)
    {
        cerr << "Error sending packet" << std::endl;
        return false;
    }
    uint8_t *buffer_p = new uint8_t[RECV_BUFFER_SIZE];
    Pocket *p;
    socklen_t paddr_len = sizeof(this->sock_paddr);

    while (true)
    {
        // sleep(0.7);
        int len = recvfrom(this->sockfd, buffer_p, RECV_BUFFER_SIZE, 0, &(this->sock_paddr), &paddr_len);
        if (len < 0)
        {
            cerr << "Failed to recv msg." << std::endl;
            return false;
        }
        p = (Pocket *)buffer_p;
        cout << "- recv checksum: " << p->checksum << endl;
        cout << "- recv seq: " << p->seq << endl;
        cout << "- recv size: " << p->size << endl;
        if (is_corrupt(p)) // corrupt
        {
            cout << "1 corrupt pkt!" << endl;
            sendto(this->sockfd, (void *)send_pkt_p, sizeof(Pocket), 0, &(this->sock_paddr), sizeof(this->sock_paddr));
            continue;
        }
        // not corrupt
        if (p->flag != ACK_FLAG) // not ACK pkt
        {
            cout << "2 recv pkt is not ACK!" << endl;
            sendto(this->sockfd, (void *)send_pkt_p, sizeof(Pocket), 0, &(this->sock_paddr), sizeof(this->sock_paddr));
            continue;
        }
        // ACK pkt
        if (p->seq != this->seq) // wrong seq
        {
            cout << "3 wrong ack seq: recv(" << p->seq << "), await(" << this->seq << ")" << endl;
            sendto(this->sockfd, (void *)send_pkt_p, sizeof(Pocket), 0, &(this->sock_paddr), sizeof(this->sock_paddr));
            continue;
        }
        // right pkt
        cout << "!! right ack seq: recv(" << p->seq << "), await(" << this->seq << ")" << endl;

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
    uint8_t *buffer_p = new uint8_t[RECV_BUFFER_SIZE];
    Pocket *p;
    socklen_t paddr_len = sizeof(this->sock_paddr);
    Pocket *send_pkt_p;

    while (true)
    {
        // sleep(0.7);
        int len = recvfrom(this->sockfd, buffer_p, RECV_BUFFER_SIZE, 0, &(this->sock_paddr), &paddr_len);
        if (len < 0)
        {
            cerr << "Failed to recv msg." << std::endl;
            return false;
        }
        p = (Pocket *)buffer_p;

        cout << "- recv checksum: " << p->checksum << endl;
        cout << "- recv seq: " << p->seq << endl;
        cout << "- recv size: " << p->size << endl;
        ///////////////////
        random_bits_err(p);
        ///////////////////
        if (is_corrupt(p)) // corrupt
        {
            cout << "1 corrupt pkt! send: ACK_" << NEXT(this->seq) << ", need pkt_" << this->seq << endl;

            make_pkt(ACK_FLAG, NEXT(this->seq), 0, 0, send_pkt_p); // ACK pkt
            sendto(this->sockfd, (void *)send_pkt_p, sizeof(Pocket), 0, &(this->sock_paddr), sizeof(this->sock_paddr));

            cout << "+ send checksum: " << send_pkt_p->checksum << endl;
            cout << "+ send seq: " << send_pkt_p->seq << endl;
            cout << "+ send size: " << send_pkt_p->size << endl;
            continue;
        }
        // not corrupt
        if (p->flag != DATA_FLAG) // not data pkt
        {
            cout << "2 recv pkt is not data pkt!" << endl;

            make_pkt(ACK_FLAG, NEXT(this->seq), 0, 0, send_pkt_p); // ACK pkt
            sendto(this->sockfd, (void *)send_pkt_p, sizeof(Pocket), 0, &(this->sock_paddr), sizeof(this->sock_paddr));

            cout << "+ send checksum: " << send_pkt_p->checksum << endl;
            cout << "+ send seq: " << send_pkt_p->seq << endl;
            cout << "+ send size: " << send_pkt_p->size << endl;
            continue;
        }
        // data pkt
        if (p->seq != this->seq) // wrong seq
        {
            cout << "3 wrong data seq: recv(" << p->seq << "), await(" << this->seq << ")" << endl;

            make_pkt(ACK_FLAG, NEXT(this->seq), 0, 0, send_pkt_p); // ACK pkt
            sendto(this->sockfd, (void *)send_pkt_p, sizeof(Pocket), 0, &(this->sock_paddr), sizeof(this->sock_paddr));

            cout << "+ send checksum: " << send_pkt_p->checksum << endl;
            cout << "+ send seq: " << send_pkt_p->seq << endl;
            cout << "+ send size: " << send_pkt_p->size << endl;
            continue;
        }
        // right seq pkt
        if (p->size > db_size)
        {
            cerr << "data size > db_size!" << endl;
            break;
        }

        cout << "!! right data pkt: recv(" << p->seq << "), await(" << this->seq << ")" << endl;

        make_pkt(ACK_FLAG, this->seq, 0, 0, send_pkt_p); // ACK pkt
        sendto(this->sockfd, (void *)send_pkt_p, sizeof(Pocket), 0, &(this->sock_paddr), sizeof(this->sock_paddr));

        cout << "+ send checksum: " << send_pkt_p->checksum << endl;
        cout << "+ send seq: " << send_pkt_p->seq << endl;
        cout << "+ send size: " << send_pkt_p->size << endl;

        cout << "this->seq: " << this->seq << endl;
        cout << "NEXT(this->seq): " << NEXT(this->seq) << endl;
        this->seq = NEXT(this->seq);

        // copy data to data_buffer
        memcpy(data_buffer, p->data, p->size);
        data_buffer[p->size] = 0; // string end '\0'

        break;
    }

    return true;
}

///// Helper Functions /////

bool make_pkt(
    const uint32_t flag,
    const uint32_t seq,
    const uint32_t size,
    const uint8_t *data,
    Pocket *&p)
{
    if (size > DATA_BUFFER_SIZE)
    {
        return false;
    }

    p = new Pocket;

    memset(p, 0, sizeof(p));

    p->flag = flag;
    p->seq = seq;
    p->size = size;
    memcpy(p->data, data, size);

    p->checksum = cal_checksum(p);
    return true;
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