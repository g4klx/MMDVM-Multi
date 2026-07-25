/*
 *   Copyright (C) 2023-2026 by Adrian Musceac YO8RZZ
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

#ifndef RECEIVER_H
#define RECEIVER_H

#include <SoapySDR/Device.hpp>
#include "Network.h"
#include "Device.h"
#include "Constants.h"
#include "ReceiverChannel.h"
#include "DMRTiming.h"
#include "Thread.h"

#include <cmath>
#include <string>
#include <cstdint>

class Receiver : public CThread
{
public:
    Receiver(Network* network, Device* device, DMRTiming* burst_timer, 
             unsigned int num_active_channels, unsigned int num_pfb_channels, float sampleRate,
             bool needs_timestamp, float symbol_deviation, float power_calibration);
    virtual ~Receiver();

    virtual void entry();

    void stop();
    bool stopped() const;
    
private:
    bool m_running;
    bool m_stopped;
    bool m_timestamping;
    Network* m_network;
    Device* m_device;
    CReceiverChannel* m_rxch;
    DMRTiming* m_burstTimer;
    unsigned int m_activeChannels;
    unsigned int m_pfbChannels;
    unsigned int m_fillReal;
    unsigned int m_powerCalibration;
    float m_symbolDeviation;
    long long m_readTime;
    std::vector<uint8_t> m_controlBuf[MAX_MMDVM_CHANNELS];
    std::vector<int16_t> m_sampleBuf[MAX_MMDVM_CHANNELS];
    unsigned int m_RSSI[MAX_MMDVM_CHANNELS];

    void processSamples(unsigned int channel, std::complex<float>* in_samples, float* output_samples);
};

#endif // RECEIVER_H
