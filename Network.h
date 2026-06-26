/*
 *   Copyright (C) 2026 by Adrian Musceac YO8RZZ
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef NETWORK_H
#define NETWORK_H

#include <string>
#include <mutex>
#include <string.h>
#include "UDPSocket.h"
#include "Constants.h"


class Network
{
public:

    Network(const std::string& modemAddress, unsigned int modemStartPort,
            const std::string& localAddress, unsigned int localStartPort,
            unsigned int numChannels);
    ~Network();

    bool open();

    int read(unsigned char* buffer, unsigned int length, unsigned int channel);

    int write(const unsigned char* buffer, unsigned int length, unsigned int channel);

    void close();

private:
    unsigned int     m_numChannels;
    CUDPSocket       m_sockets[MAX_MMDVM_CHANNELS];
    sockaddr_storage m_addr[MAX_MMDVM_CHANNELS];
    unsigned int     m_addrLen[MAX_MMDVM_CHANNELS];
    bool             m_init;
    std::mutex       m_mutex;

};

#endif // NETWORK_H
