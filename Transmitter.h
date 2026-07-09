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

#ifndef TRANSMITTER_H
#define TRANSMITTER_H

#include <complex>
#include <cmath>
#include <chrono>
#include <string>
#include <thread>
#include <vector>
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


class Transmitter
{
public:
    Transmitter(Network* network, Device* device, FMMod* fm_mod, Resampler* resampler, Rotator* rotator,
                Channelizer* channelizer, DMRTiming* burst_timer,
                unsigned int num_active_channels, unsigned int num_pfb_channels, bool needs_timestamp,
                float dac_scaling, float symbol_deviation);
    ~Transmitter();
    void start();
    void run();
    void stop();
    bool stopped() const;

private:
    void readNetwork();
    void processSamples(std::complex<float>* output_samples, bool* channel_idle);
    void setTx(bool* channelIdle);
    void nextSlot(unsigned int channel);

    bool m_running;
    bool m_stopped;
    bool m_timingInit;
    bool m_tx;
    bool m_timestamping;
    float m_DACScaling;
    float m_symbolDeviation;
    unsigned int  m_activeChannels;
    unsigned int  m_pfbChannels;
    unsigned int m_fillReal;
    int64_t m_timingCorrection;
    Network* m_network;
    Device* m_device;
    FMMod* m_fmMod;
    Resampler* m_resampler;
    Rotator* m_rotator;
    Channelizer* m_channelizer;
    DMRTiming* m_burstTimer;
    std::thread m_thread;
    std::vector<uint8_t> m_controlBuf[MAX_MMDVM_CHANNELS];
    std::vector<float> m_sampleBuf[MAX_MMDVM_CHANNELS];
    uint8_t m_sn[MAX_MMDVM_CHANNELS];

};

#endif // TRANSMITTER_H
