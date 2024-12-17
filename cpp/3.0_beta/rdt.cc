
#include "rdt.hh"
#include "utils.hh"

#include "timer.h" // timer in secs

#include <iostream>
#include <unistd.h>
#include <cstring>
#include <random>
#include <fcntl.h> // IO control
#include <time.h>  // sleep for ms

#define DEBUG 1
#define SERVER_DELAY 1
#define SERVER_
using namespace std;
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

    this->send_seq = 0;
    this->recv_seq = 0;

    memset(&(this->sock_addr), 0, sizeof(this->sock_addr));
    memset(&(this->sock_paddr), 0, sizeof(this->sock_paddr));

    set_sin_addr((sockaddr_in *)&(this->sock_addr), addr);
    set_sin_addr((sockaddr_in *)&(this->sock_paddr), paddr);

    // cout << ((sockaddr_in *)&(this->sock_addr))->sin_family << endl;

    this->sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (this->sockfd == -1)
    {
        cerr << "Failed to create UDP socket." << std::endl;
        return;
    }
    if (bind(this->sockfd, &(this->sock_addr), sizeof(this->sock_addr)) < 0)
    {
        cerr << "Failed to bind UDP socket." << std::endl;
        return;
    }
}

Base::~Base()
{
    close(this->sockfd);
}

void Base::udt_send(const Pocket *send_pkt_p)
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

bool Base::udt_recv(const uint8_t *recv_buffer)
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
            cout << "? recv time out!" << endl;
            return false;
        }
        cerr << "Failed to recv msg." << std::endl;
        return false;
    }
    return true;
}

bool Base::rdt_send(const uint8_t *data, const int dsize)
{
    make_pkt_beta(DATA_FLAG,
                  this->send_seq,
                  dsize,
                  data,
                  this->send_buffer,
                  BUFFER_SIZE);

    Pocket *send_pkt_p = (Pocket *)(this->send_buffer);

#ifdef DEBUG
    cout << "+ SEND" << endl;
    print_pkt(send_pkt_p);
#endif

    udt_send(send_pkt_p);

    /////// start timer //////////
    double start_time, curr_time;
    GET_TIME(start_time);
    /////// start timer //////////

    while (true) // state loop
    {
        GET_TIME(curr_time);
        if (curr_time - start_time > this->timeout)
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

        if (is_corrupt(recv_pkt_p) ||            // corrupt
            (recv_pkt_p->flag != ACK_FLAG) ||    // not ACK pkt
            (recv_pkt_p->seq != this->send_seq)) // wrong seq
        {
#ifdef DEBUG
            if (is_corrupt(recv_pkt_p))
            {
                cout << "1 corrupt pkt!" << endl;
            }
            else if (recv_pkt_p->flag != ACK_FLAG)
            {
                cout << "2 recv pkt is not ACK!" << endl;
            }
            else if (recv_pkt_p->seq != this->send_seq)
            {
                cout << "3 wrong ack seq: recv(" << recv_pkt_p->seq << "),"
                     << " await(" << this->send_seq << ")" << endl;
            }
#endif
            udt_send(send_pkt_p); // resend
            continue;
        }
        // right ack pkt
#ifdef DEBUG
        cout << "!! right ack seq: recv(" << recv_pkt_p->seq << "),"
             << " await(" << this->send_seq << ")" << endl;

        cout << "this->send_seq: " << this->send_seq << endl;
        cout << "NEXT(this->send_seq): " << NEXT(this->send_seq) << endl;
#endif
        this->send_seq = NEXT(this->send_seq);
        break;
    }

    return true;
}

int Base::rdt_recv(uint8_t *data_buffer, const int db_size)
{
    while (true) // always in loop
    {
        if (udt_recv(this->recv_buffer) == false) // listen and recv
        {
            continue;
        }
        Pocket *recv_pkt_p = (Pocket *)(this->recv_buffer);
        cout << "- RECV" << endl;
        print_pkt(recv_pkt_p);
        ///////////////////
        if (random_pkt_loss(this->sim_pkt_loss_rate)) // loss
        {
            continue;
        }
        random_bit_err(recv_pkt_p, this->sim_bit_err_rate);
        ///////////////////
        if (is_corrupt(recv_pkt_p) ||           // corrupt
            (recv_pkt_p->flag != DATA_FLAG) ||  // not data pkt
            (recv_pkt_p->seq != this->recv_seq) // wrong seq
        )
        {
#ifdef DEBUG
            if (is_corrupt(recv_pkt_p)) // corrupt
            {
                cout << "1 corrupt pkt! send: ACK_" << NEXT(this->recv_seq) << ", "
                     << "need pkt_" << this->recv_seq << endl;
            }
            else if (recv_pkt_p->flag != DATA_FLAG) // not data pkt
            {
                cout << "2 recv pkt is not data pkt!" << endl;
            }
            else if (recv_pkt_p->seq != this->recv_seq) // wrong seq
            {
                cout << "3 wrong data seq: "
                     << "recv(" << recv_pkt_p->seq << "), "
                     << "await(" << this->recv_seq << "), "
                     << "send(" << NEXT(this->recv_seq) << ")" << endl;
            }
#endif
            make_pkt_beta(ACK_FLAG,
                          NEXT(this->recv_seq), // not wanted pkt, send last recv seq
                          0,
                          0,
                          this->send_buffer,
                          BUFFER_SIZE);

            Pocket *send_pkt_p = (Pocket *)(this->send_buffer);
            udt_send(send_pkt_p);

#ifdef DEBUG
            cout << "+ SEND" << endl;
            print_pkt(send_pkt_p);
#endif

            continue;
        }
        // right seq pkt
        if (recv_pkt_p->size > db_size)
        {
            cerr << "data size > db_size!" << endl;
            break;
        }

        /// sleep to trigger timeout ///
        sim_delay(this->sim_delay_time);
        /// sleep to trigger timeout ///
#ifdef DEBUG
        cout << "!! right data pkt: "
             << "recv(" << recv_pkt_p->seq << "), "
             << "await(" << this->recv_seq << ")" << endl;
#endif
        ///////////////////////////////////////////////////
        make_pkt_beta(ACK_FLAG,
                      this->recv_seq, // get wanted pkt, ack recv_seq
                      0,
                      0,
                      this->send_buffer,
                      BUFFER_SIZE);
        Pocket *send_pkt_p = (Pocket *)(this->send_buffer);

        udt_send(send_pkt_p);
        //////////////////////////////////////////////
#ifdef DEBUG
        cout << "+ SEND" << endl;
        print_pkt(send_pkt_p);

        cout << "this->recv_seq: " << this->recv_seq << endl;
        cout << "NEXT(this->recv_seq): " << NEXT(this->recv_seq) << endl;
#endif

        this->recv_seq = NEXT(this->recv_seq);

        // copy data to data_buffer
        memcpy(data_buffer, recv_pkt_p->data, recv_pkt_p->size);
        data_buffer[recv_pkt_p->size] = 0; // string end '\0'

        return recv_pkt_p->size; // return data size
    }
    return 0;
}