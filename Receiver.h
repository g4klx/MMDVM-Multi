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

#ifndef RECEIVER_H
#define RECEIVER_H

#include <thread>
#include <chrono>
#include <cmath>
#include <string>
#include <cstdint>
#include <SoapySDR/Device.hpp>
#include "Network.h"
#include "Device.h"
#include "Constants.h"
#include "DMRTiming.h"
#include "FMMod.h"
#include "Resampler.h"
#include "Rotator.h"
#include "Channelizer.h"


class Receiver
{
public:
    Receiver(Network* network, Device* device, FMMod* fm_mod, Resampler* resampler, Rotator* rotator,
             Channelizer* channelizer, DMRTiming* burst_timer, unsigned int num_active_channels,
             unsigned int num_pfb_channels, float power_calibration, float symbol_deviation, bool needs_timestamp);
    ~Receiver();
    void start();
    void run();
    void stop();
    bool stopped() const;
    
private:
    void processSamples(unsigned int channel, std::complex<float>* in_samples, float* output_samples);
    bool m_running;
    bool m_stopped;
    bool m_timestamping;
    Network* m_network;
    Device* m_device;
    FMMod* m_fmMod;
    Resampler* m_resampler;
    Rotator* m_rotator;
    Channelizer* m_channelizer;
    DMRTiming* m_burstTimer;
    unsigned int m_activeChannels;
    unsigned int m_pfbChannels;
    unsigned int m_fillReal;
    unsigned int m_powerCalibration;
    float m_symbolDeviation;
    long long m_readTime;
    std::thread m_thread;
    std::vector<uint8_t> m_controlBuf[MAX_MMDVM_CHANNELS];
    std::vector<int16_t> m_sampleBuf[MAX_MMDVM_CHANNELS];
    unsigned int m_RSSI[MAX_MMDVM_CHANNELS];

};

#endif // RECEIVER_H
