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

#include "TransmitterChannel.h"
#include "Log.h"

#include <cassert>

CTransmitterChannel::CTransmitterChannel(unsigned int activeChannels, unsigned int pfbChannels, float sampleRate, float deviation) : 
m_activeChannels(activeChannels),
m_channels(pfbChannels),
m_sampleRate(sampleRate),
m_ncoChannelRotator(0),
m_firChannelSynthesizer(0),
m_upsampleDecim(RESAMPLER_DECIMATION),
m_upsampleInterp(RESAMPLER_INTERPOLATION)
{
    assert(m_activeChannels > 0);
    assert(m_activeChannels <= MAX_MMDVM_CHANNELS);

    assert(m_channels > 3U);
    assert(m_channels <= MAX_PFB_CHANNELS);
    assert(m_activeChannels < m_channels);

    // channel merger
    m_firChannelSynthesizer = ::firpfbch_crcf_create_kaiser(LIQUID_SYNTHESIZER, m_channels, PFB_FILTER_DELAY, 70.0F);

    // channel rotator
    this->setRotateParams(DEFAULT_BASEBAND_SHIFT, m_sampleRate);

    // upsampler
    for (unsigned int i = 0U; i < sizeof(m_rreUpsampler) / sizeof(m_rreUpsampler[0]); i++) {
        m_rreUpsampler[i] = 0;
    }
    for (unsigned int i = 0U; i < m_activeChannels; i++) {
        m_rreUpsampler[i] = ::rresamp_crcf_create_kaiser(m_upsampleInterp, m_upsampleDecim, RESAMPLER_FILTER_DELAY, RESAMPLER_FRACTIONAL_BW, 70.0F);
    }

    // modulator
    for (unsigned int i = 0U; i < sizeof(m_FMmod) / sizeof(m_FMmod[0]); i++) {
        m_FMmod[i] = 0;
    }
    for (unsigned int i = 0U; i < m_activeChannels; i++) {
      m_FMmod[i] = ::freqmod_create(deviation);
    }
}

CTransmitterChannel::~CTransmitterChannel()
{
    ::nco_crcf_destroy(m_ncoChannelRotator);

    ::firpfbch_crcf_destroy(m_firChannelSynthesizer);

    for (unsigned int i = 0U; i < m_activeChannels; i++) {
        ::rresamp_crcf_destroy(m_rreUpsampler[i]);
    }

    for (unsigned int i = 0U; i < m_activeChannels; i++) {
        ::freqmod_destroy(m_FMmod[i]);
    }
}

void CTransmitterChannel::setRotateParams(float rotation_hz, float sample_rate) {
    assert(sample_rate > 0.0F);

    nco_crcf nco = ::nco_crcf_create(LIQUID_VCO);
    ::nco_crcf_set_phase(nco, 0.0F);
    ::nco_crcf_set_frequency(nco, (2.0F * M_PI * rotation_hz / sample_rate));

    if (m_ncoChannelRotator)
        ::nco_crcf_destroy(m_ncoChannelRotator);
    
    m_ncoChannelRotator = nco;
}

void CTransmitterChannel::setUpsamplerParams(unsigned int interp, unsigned int decim, float bw) {
    assert(decim > 0U);

    m_upsampleInterp = interp;
    m_upsampleDecim = decim;

    for (unsigned int i = 0U; i < sizeof(m_rreUpsampler) / sizeof(m_rreUpsampler[0]); i++) {
        if (m_rreUpsampler[i]) {
            ::rresamp_crcf_destroy(m_rreUpsampler[i]);
        }
    }

    for (unsigned int i = 0U; i < m_activeChannels; i++) {
        m_rreUpsampler[i] = ::rresamp_crcf_create_kaiser(interp, decim, RESAMPLER_FILTER_DELAY, bw, 70.0F);
    }
}

void CTransmitterChannel::rotateUp(std::complex<float>* in_samples, unsigned int num_samples, std::complex<float>* out_samples) {
    assert(in_samples != nullptr);
	assert(out_samples != nullptr);

	::nco_crcf_mix_block_up(m_ncoChannelRotator, in_samples, out_samples, num_samples);
}

void CTransmitterChannel::synthesize(std::complex<float>* in_samples, std::complex<float>* out_samples) {
    ::firpfbch_crcf_synthesizer_execute(m_firChannelSynthesizer, in_samples, out_samples);
}

void CTransmitterChannel::upsample(unsigned int channel, std::complex<float>* in_samples,
                                         unsigned int num_samples, std::complex<float>* out_samples)
{
    assert(in_samples != nullptr);
    assert(out_samples != nullptr);

    // TODO - Eliminate the heap-allocation

    unsigned int decim = m_upsampleDecim;
    unsigned int interp = m_upsampleInterp;

    unsigned int p_in = num_samples / decim;
    std::complex<float>* in_buf  = new std::complex<float>[decim];
    std::complex<float>* out_buf = new std::complex<float>[interp];

    for (unsigned int i = 0U; i < p_in; i++) {
        ::memcpy(in_buf, in_samples + (i * decim), decim * sizeof(std::complex<float>));
        ::rresamp_crcf_execute(m_rreUpsampler[channel], in_buf, out_buf);
        ::memcpy(out_samples + (i * interp), out_buf, interp * sizeof(std::complex<float>));
    }

    delete[] in_buf;
    delete[] out_buf;
}

void CTransmitterChannel::modulate(unsigned int channel, float* in_samples, 
                                         const unsigned int num_samples, std::complex<float>* out_samples)
{
    assert(channel < m_activeChannels);
    assert(in_samples != nullptr);
    assert(out_samples != nullptr);

    ::freqmod_modulate_block(m_FMmod[channel], in_samples, num_samples, out_samples);
}