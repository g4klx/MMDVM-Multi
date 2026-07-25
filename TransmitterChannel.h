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

#ifndef TRANSMITTER_CHANNEL_H
#define TRANSMITTER_CHANNEL_H

#include <complex>
#include <cmath>
#include <liquid/liquid.h>

#include "Constants.h"

#include <cstdint>

class CTransmitterChannel
{
public:
    CTransmitterChannel(unsigned int activeChannels, unsigned int channels, float sampleRate, float deviation);
    virtual ~CTransmitterChannel();

    void setRotateParams(float rotation_hz, float sample_rate);
    void setUpsamplerParams(unsigned int interp, unsigned int decim, float bw);

    void rotateUp(std::complex<float>* in_samples, unsigned int num_samples, std::complex<float>* out_samples);

    void synthesize(std::complex<float>* in_samples, std::complex<float>* out_samples);

    void upsample(unsigned int channel, std::complex<float>* in_samples,
                    unsigned int num_samples, std::complex<float>* out_samples);

    void modulate(unsigned int channel, float* in_samples, const unsigned int num_samples, std::complex<float>* out_samples);

    unsigned int getDecim() const
    {
        return m_upsampleDecim;
    }

    unsigned int getInterp() const
    {
        return m_upsampleInterp;
    }

private:
    unsigned int    m_activeChannels;
    unsigned int    m_channels;
    float           m_sampleRate;

    nco_crcf        m_ncoChannelRotator;

    firpfbch_crcf   m_firChannelSynthesizer;

    unsigned int    m_upsampleDecim;
    unsigned int    m_upsampleInterp;
    rresamp_crcf    m_rreUpsampler[MAX_MMDVM_CHANNELS];

    freqmod         m_FMmod[MAX_MMDVM_CHANNELS];
};

#endif // TRANSMITTER_CHANNEL_H
