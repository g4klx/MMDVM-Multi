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
#include <string>
#include <cstdint>
#include <SoapySDR/Device.hpp>
#include <zmq.hpp>
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
    Receiver(Device* device, FMMod* fm_mod, Resampler* resampler, Rotator* rotator,
             Channelizer* channelizer, DMRTiming* burst_timer, unsigned int num_active_channels, unsigned int num_pfb_channels);
    ~Receiver();
    void start();
    void run();
    void stop();
    bool stopped() const;
    
private:
    void processSamples(unsigned int channel, std::complex<float>* in_samples, float* output_samples);
    bool m_running;
    bool m_stopped;
    Device* m_device;
    FMMod* m_fmMod;
    Resampler* m_resampler;
    Rotator* m_rotator;
    Channelizer* m_channelizer;
    DMRTiming* m_burstTimer;
    unsigned int m_activeChannels;
    unsigned int m_pfbChannels;
    unsigned int m_fillReal;
    std::thread m_thread;
    zmq::context_t m_zmqCtx[MAX_MMDVM_CHANNELS];
    zmq::socket_t m_zmqSocket[MAX_MMDVM_CHANNELS];
    std::vector<uint8_t> control_buf[MAX_MMDVM_CHANNELS];
    std::vector<int16_t> data_buf[MAX_MMDVM_CHANNELS];

};

#endif // RECEIVER_H
