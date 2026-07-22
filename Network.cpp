/*
 *   Copyright (C) 2024,2025,2026 by Jonathan Naylor G4KLX
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

#include "Network.h"
#include "Log.h"

#include <cassert>

const unsigned int BUFFER_LENGTH = 3000U;

Network::Network(const std::string& modemAddress, unsigned int modemStartPort,
                 const std::string& localAddress, unsigned int localStartPort,
                 unsigned int numChannels) :
m_numChannels(numChannels),
m_init(true)
{
    assert(m_numChannels <= MAX_MMDVM_CHANNELS);

    for (unsigned int i = 0U; i < m_numChannels; i++) {
        m_addrLen[i] = 0U;

        if (CUDPSocket::lookup(modemAddress, modemStartPort + i, m_addr[i], m_addrLen[i]) != 0) {
            m_init = false;
            break;
        }

        m_sockets[i] = CUDPSocket(localAddress, localStartPort + i);
    }
}

Network::~Network()
{
}

bool Network::open()
{
    if (!m_init) {
        ::LogError("Unable to resolve the address of the modem");
        return false;
    }

    bool success = true;
    for (unsigned int i = 0U; i < m_numChannels; i++) {
        if (!m_sockets[i].open(m_addr[i])) {
            success = false;
            break;
        }
    }

    return success;
}

int Network::read(unsigned char* buffer, unsigned int length, unsigned int channel)
{
    assert(buffer != nullptr);
    assert(length > 0U);
    assert(channel < m_numChannels);

    unsigned char data[BUFFER_LENGTH];
    sockaddr_storage addr;
    unsigned int addrLen;

    int ret = m_sockets[channel].read(data, length, addr, addrLen);
    if (ret < 0)
        return ret;

    if (ret > 0) {
        length = (unsigned int)ret;
        if (CUDPSocket::match(addr, m_addr[channel])) {
            ::memcpy(buffer, data, length * sizeof(unsigned char));
            return ret;
        }
    }

    return 0;
}

int Network::write(const unsigned char* buffer, unsigned int length, unsigned int channel)
{
    assert(buffer != nullptr);
    assert(length > 0U);
    assert(channel < m_numChannels);

    return m_sockets[channel].write(buffer, length, m_addr[channel], m_addrLen[channel]) ? int(length) : -1;
}

void Network::close()
{
    if (!m_init)
        return;

    for (unsigned int i = 0U; i < m_numChannels; i++)
        m_sockets[i].close();
}
