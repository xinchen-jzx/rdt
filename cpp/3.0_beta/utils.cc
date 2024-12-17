#include "utils.hh"

#include <iostream>
#include <unistd.h>
#include <cstring>
#include <random>
#include <time.h> // sleep for ms

using namespace std;

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

void random_bit_err(Pocket *&p, const double rate)
{
    random_device r;
    default_random_engine gen(r());
    uniform_real_distribution<double> udist(0.0, 1.0);
    double rand = udist(gen);
    // int randint = rand() % 10 + 1;
    // cout << "randint: " << randint << endl;
    if (rand < rate)
    {
        uint8_t *pp = (uint8_t *)p;
        pp[1] = 7 - pp[1];
    }
}

void sim_delay(const double delay)
{
    timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = ceil(delay * 1000000000);
    nanosleep(&ts, NULL);
}

bool random_pkt_loss(const double lrate)
{
    random_device r;
    default_random_engine gen(r());
    uniform_real_distribution<double> udist(0.0, 1.0);
    double rand = udist(gen);
    // int randint = rand() % 10 + 1;
    // cout << "randint: " << randint << endl;
    if (rand < lrate)
    {
        return true;
    }
    return false;
}