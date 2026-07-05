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

#ifndef SVXBRIDGE_H
#define SVXBRIDGE_H

#include <string.h>
#include <csignal>
#include <thread>
#include <chrono>
#include <vector>
#include <cstdint>
#include <string>
#include <liquid/liquid.h>
#include "Constants.h"
#include "Conf.h"
#include "UDPSocket.h"


class SVXBridge
{
public:
  
  SVXBridge(const std::string& modemAddress, unsigned int modemPort,
            const std::string& localModemAddress, unsigned int localModemPort,
            const std::string& svxAddress, unsigned int svxPort,
            const std::string& localSVXAddress, unsigned int localSVXPort,
            unsigned int interp, unsigned int decim, unsigned int rxGain, unsigned int txGain);

  ~SVXBridge();

  void runBridge();
  void processSVXLink();
  void processModem();

  bool open();
  int readSVX(unsigned char* buffer, unsigned int length);
  int writeSVX(const unsigned char* buffer, unsigned int length);
  int readModem(unsigned char* buffer, unsigned int length);
  int writeModem(const unsigned char* buffer, unsigned int length);
  void close();

  void upsample(float* in_samples, unsigned int num_samples, float* out_samples);
  void downsample(float* in_samples, unsigned int num_samples, float* out_samples);
    
private:
  bool m_init;
  CUDPSocket       m_sockSVX;
  CUDPSocket       m_sockModem;
  sockaddr_storage m_addrSVX;
  unsigned int     m_addrLenSVX;
  sockaddr_storage m_addrModem;
  unsigned int     m_addrLenModem;
  unsigned int m_decim;
  unsigned int m_interp;
  unsigned int m_rxGain;
  unsigned int m_txGain;
  unsigned int m_netTimeout;
  rresamp_rrrf m_upsampler;
  rresamp_rrrf m_downsampler;
  std::vector<int16_t> m_inputBuffer;

};

#endif // SVXBRIDGE_H
