#pragma once

#include "rdt.hh"

#include <string>
#include <unistd.h>
#include <sys/socket.h>
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

void random_bit_err(
    Pocket *&p,
    const double rate);

void sim_delay(const double delay);

bool random_pkt_loss(const double lrate);