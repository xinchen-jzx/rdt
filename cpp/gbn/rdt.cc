
#include "rdt.hh"
#include "utils.hh"

#include "timer.h" // timer in secs

#include <iostream>
#include <unistd.h>
#include <cstring>
#include <random>
#include <fcntl.h> // IO control
#include <time.h>  // sleep for ms
#include <thread>

// #define DEBUG 1

void Base::start_listen()
{
    this->listen_th = std::thread(&Base::listen, this);
}
void Base::stop_listen()
{
    this->listen_stop = true;
    if (this->listen_th.joinable())
    {
        this->listen_th.join();
    }
}

void Base::listen()
{
    double curr_time;
    while (true)
    {
        if ((this->listen_stop) && (this->start_time < 0.0))
        {
            return;
        }
        GET_TIME(curr_time);
        if ((this->start_time > 0.0) &&                     // if empty (seq == nseq), timer stop, cant in
            (curr_time - this->start_time > this->timeout)) // time out resend
        {
#ifdef DEBUG
            std::cout << "** listen timeout, resend" << std::endl;
            std::cout << "** resend: this->base: " << this->base << ", "
                      << "this->nseq: " << (this->nseq) << std::endl;
#endif
            GET_TIME(this->start_time);

            for (int pi = this->base; pi < this->nseq; pi++)
            {
#ifdef DEBUG
                std::cout << "++ RESEND" << std::endl;
#endif
                udt_send(&(this->pkt_buf[INDEX(pi)]));
            }
        }
        if (udt_recv(this->recv_buffer) == false)
        {
            continue;
        }
        Packet *recv_pkt_p = (Packet *)(this->recv_buffer);

        if (!is_corrupt(recv_pkt_p)) // not corrupt
        {
            this->base = NEXT(recv_pkt_p->seq);

#ifdef DEBUG
            std::cout << "!!! base: " << this->base << ", "
                      << "nseq: " << this->nseq << std::endl;
#endif

            if (this->base == this->nseq) // empty stop timer
            {
                this->start_time = -1.0;
            }
            else
            {
                GET_TIME(this->start_time);
            }
        }
        else
        {

            std::cout << "1 corrupt pkt!" << std::endl;

            continue;
        }
    }
}

bool Base::set_sin_addr(
    sockaddr_in *sin_addr_p,
    const Address &addr)
{
    sin_addr_p->sin_family = AF_INET;                        // 设置为IP通信
    sin_addr_p->sin_addr.s_addr = inet_addr(addr.ip.data()); // 设置IP地址
    sin_addr_p->sin_port = htons(addr.port);                 // 设置端口号
    return true;
}

void Base::set_bit_err_rate(const double rate)
{
    this->sim_bit_err_rate = rate;
}
void Base::set_pkt_loss_rate(const double lrate)
{
    this->sim_pkt_loss_rate = lrate;
}
void Base::set_delay_time(const double time)
{
    this->sim_delay_time = time;
}
void Base::set_recv_time(const double time)
{
    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = ceil(time * 1000000);
    setsockopt(this->sockfd, SOL_SOCKET, SO_RCVTIMEO, (void *)&timeout, sizeof(timeout));
}
void Base::set_timeout(const double time)
{
    this->timeout = time;
}
Base::Base(
    const Address addr,
    const Address paddr)
    : addr(addr), paddr(paddr)
{
    this->timeout = 0.3;
    this->sim_bit_err_rate = 0.0;
    this->sim_pkt_loss_rate = 0.0;
    this->sim_delay_time = 0.0; // secs

    // this->send_seq = 0;
    // this->recv_seq = 0;

    memset(&(this->sock_addr), 0, sizeof(this->sock_addr));
    memset(&(this->sock_paddr), 0, sizeof(this->sock_paddr));

    set_sin_addr((sockaddr_in *)&(this->sock_addr), addr);
    set_sin_addr((sockaddr_in *)&(this->sock_paddr), paddr);

    // std::cout << ((sockaddr_in *)&(this->sock_addr))->sin_family << endl;

    this->sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (this->sockfd == -1)
    {
        std::cerr << "Failed to create UDP socket." << std::endl;
        return;
    }
    if (bind(this->sockfd, &(this->sock_addr), sizeof(this->sock_addr)) < 0)
    {
        std::cerr << "Failed to bind UDP socket." << std::endl;
        return;
    }
    this->base = 1;
    this->nseq = 1;

    this->eseq = 1;
    this->start_time = -1.0;

    this->listen_stop = false;

    // this->listen_th = std::thread(&Base::listen, this);
}

Base::~Base()
{
    close(this->sockfd);
}

void Base::udt_send(const Packet *send_pkt_p)
{
    if (sendto(this->sockfd,
               (void *)send_pkt_p,
               sizeof(Packet),
               0,
               &(this->sock_paddr),
               sizeof(this->sock_paddr)) < 0)
    {
        std::cerr << "Error sending packet" << std::endl;
    }
#ifdef DEBUG
    std::cout << "+ SEND" << std::endl;
    print_pkt(send_pkt_p);
#endif
}

bool Base::udt_recv(const uint8_t *recv_buffer)
{
    socklen_t paddr_len = sizeof(this->sock_paddr);

    int len = recvfrom(this->sockfd,
                       &(this->recv_buffer),
                       BUFFER_SIZE,
                       0,
                       &(this->sock_paddr),
                       &paddr_len);
    // std::cout << "udt_recv len: " << len << std::endl;
    if (len == -1)
    {
        if (errno == EAGAIN)
        {
            // std::cout << "? recv time out!" << std::endl;
            return false;
        }
        std::cerr << "Failed to recv msg." << std::endl;
        return false;
    }
#ifdef DEBUG
    Packet *recv_pkt_p = (Packet *)(this->recv_buffer);
    std::cout << "- RECV" << std::endl;
    print_pkt(recv_pkt_p);
#endif
    return true;
}

bool Base::rdt_send(const uint8_t *data, const int dsize)
{
    if (this->base + WINDOW_SIZE == this->nseq)
    {
        std::cout << "full!" << std::endl;
        return false; // buffer is full
    }
    // not full
    // std::cout << "enter rdt_send" << std::endl;

    make_pkt_beta(DATA_FLAG,
                  this->nseq,
                  dsize,
                  data,
                  (uint8_t *)&(this->pkt_buf[INDEX(this->nseq)]),
                  sizeof(Packet));

    Packet *send_pkt_p = &(this->pkt_buf[INDEX(this->nseq)]);
    udt_send(send_pkt_p);
    // std::cout << "udt_send: " << std::endl;
    // print_pkt(send_pkt_p);
    if (this->base == this->nseq) // 未满 and next == base, 空
    {
        GET_TIME(this->start_time);
    }
#ifdef DEBUG
    std::cout << "^^ after send, this->nseq: " << (this->nseq) << ", ";
    std::cout << "NEXT(this->nseq) " << NEXT(this->nseq) << std::endl;
#endif
    this->nseq = NEXT(this->nseq);

    return true; // sended
}

int Base::rdt_recv(uint8_t *data_buffer, const int db_size)
{
    while (true) // always in loop
    {
        if (udt_recv(this->recv_buffer) == false) // listen and recv
        {
            continue;
        }
        Packet *recv_pkt_p = (Packet *)(this->recv_buffer);
        ///////////////////
        if (random_pkt_loss(this->sim_pkt_loss_rate)) // loss
        {
            // std::cout << "loss! pkt seq: " << recv_pkt_p->seq << std::endl;
            continue;
        }
        random_bit_err(recv_pkt_p, this->sim_bit_err_rate);
        ///////////////////
        if (!is_corrupt(recv_pkt_p) &&       // not corrupt and
            (recv_pkt_p->seq == this->eseq)) // right seq
        {
            sim_delay(this->sim_delay_time);
            make_pkt_beta(ACK_FLAG,
                          this->eseq, // not wanted pkt, send last recv seq
                          0,
                          0,
                          this->send_buffer,
                          BUFFER_SIZE);
            Packet *send_pkt_p = (Packet *)(this->send_buffer);
            udt_send(send_pkt_p);
            // print_pkt(send_pkt_p);
            this->eseq = NEXT(this->eseq);
            // copy data to data_buffer
            memcpy(data_buffer, recv_pkt_p->data, recv_pkt_p->size);
            data_buffer[recv_pkt_p->size] = 0; // string end '\0'
            // std::cout << "GOOD" << std::endl;
            return recv_pkt_p->size; // return data size
        }
        // corrupt or disorder
        make_pkt_beta(ACK_FLAG,
                      PRE(this->eseq), // not wanted pkt, send last recv seq
                      0,
                      0,
                      this->send_buffer,
                      BUFFER_SIZE);
        Packet *send_pkt_p = (Packet *)(this->send_buffer);
        udt_send(send_pkt_p);
#ifdef DEBUG

        if (is_corrupt(recv_pkt_p))
        {
            std::cout << "1 corrupt pkt!" << std::endl;
        }
        else if (recv_pkt_p->seq != this->eseq)
        {
            std::cout << "3 wrong ack seq: recv(" << recv_pkt_p->seq << "),"
                      << " await(" << this->eseq << ")" << std::endl;
        }
        else
        {
            std::cout << "4 else" << std::endl;
        }
#endif
        continue;
    }
}