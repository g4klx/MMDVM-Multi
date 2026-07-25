/*
 *   Copyright (C) 2023-2026 by Adrian Musceac YO8RZZ
 *   Copyright (C) 2026 by Shawn Chain BG5HHP
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

#ifndef RECEIVER_CHANNEL_H
#define RECEIVER_CHANNEL_H

#include <complex>
#include <cmath>
#include <liquid/liquid.h>

#include "Constants.h"

#include <cstdint>

class CReceiverChannel
{
public:
    CReceiverChannel(unsigned int activeChannels, unsigned int channels, float sampleRate, float deviation);
    virtual ~CReceiverChannel();

    void setRotateParams(float rotation_hz, float sample_rate);
    void setDownsamplerParams(unsigned int interp, unsigned int decim, float bw);

    void rotateDown(std::complex<float>* in_samples, unsigned int num_samples, std::complex<float>* out_samples);

    void channelize(std::complex<float>* in_samples, std::complex<float>* out_samples);

    void downsample(unsigned int channel, std::complex<float>* in_samples,
                    unsigned int num_samples, std::complex<float>* out_samples);

    void demodulate(unsigned int channel, std::complex<float>* in_samples, const unsigned int num_samples,float* out_samples);

    unsigned int getDecim() const
    {
        return m_downsampleDecim;
    }

    unsigned int getInterp() const
    {
        return m_downsampleInterp;
    }

private:
    unsigned int    m_activeChannels;
    unsigned int    m_channels;
    float           m_sampleRate;

    nco_crcf        m_ncoChannelRotator;

    firpfbch_crcf   m_firChannelAnalyzer;

    unsigned int    m_downsampleDecim;
    unsigned int    m_downsampleInterp;
    rresamp_crcf    m_rreDownsampler[MAX_MMDVM_CHANNELS];

    freqdem         m_FMdemod[MAX_MMDVM_CHANNELS];
};

#endif // RECEIVER_CHANNEL_H
