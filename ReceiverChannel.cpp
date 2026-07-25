/*
 *   Copyright (C) 2023-2026 by Adrian Musceac YO8RZZ
 *   Copyright (C) 2023-2026 by Shawn Chain BG5HHP
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

#include "ReceiverChannel.h"
#include "Log.h"

#include <cassert>

CReceiverChannel::CReceiverChannel(unsigned int activeChannels, unsigned int pfbChannels, float sampleRate, float deviation) : 
m_activeChannels(activeChannels),
m_channels(pfbChannels),
m_sampleRate(sampleRate),
m_ncoChannelRotator(0),
m_firChannelAnalyzer(0),
m_downsampleDecim(RESAMPLER_DECIMATION),
m_downsampleInterp(RESAMPLER_INTERPOLATION)
{
    assert(m_activeChannels > 0);
    assert(m_activeChannels <= MAX_MMDVM_CHANNELS);

    assert(m_channels > 3U);
    assert(m_channels <= MAX_PFB_CHANNELS);
    assert(m_activeChannels < m_channels);

    // channel splitter
    m_firChannelAnalyzer = ::firpfbch_crcf_create_kaiser(LIQUID_ANALYZER, m_channels, PFB_FILTER_DELAY, 70.0F);

    // channel rotator
    this->setRotateParams(DEFAULT_BASEBAND_SHIFT, m_sampleRate);

    // downsampler
    for (unsigned int i = 0U; i < sizeof(m_rreDownsampler) / sizeof(m_rreDownsampler[0]); i++) {
        m_rreDownsampler[i] = 0;
    }
    for (unsigned int i = 0U; i < m_activeChannels; i++) {
        m_rreDownsampler[i] = ::rresamp_crcf_create_kaiser(m_downsampleDecim, m_downsampleInterp, RESAMPLER_FILTER_DELAY, RESAMPLER_FRACTIONAL_BW, 70.0F);
    }

    // demodulator
    for (unsigned int i = 0U; i < sizeof(m_FMdemod) / sizeof(m_FMdemod[0]); i++) {
        m_FMdemod[i] = 0;
    }
    for (unsigned int i = 0U; i < m_activeChannels; i++) {
      m_FMdemod[i] = ::freqdem_create(deviation);
    }
}

CReceiverChannel::~CReceiverChannel()
{
    ::nco_crcf_destroy(m_ncoChannelRotator);

    ::firpfbch_crcf_destroy(m_firChannelAnalyzer);

    for (unsigned int i = 0U; i < m_activeChannels; i++) {
        ::rresamp_crcf_destroy(m_rreDownsampler[i]);
    }

    for (unsigned int i = 0U; i < m_activeChannels; i++) {
        ::freqdem_destroy(m_FMdemod[i]);
    }
}

void CReceiverChannel::setRotateParams(float rotation_hz, float sample_rate) {
    assert(sample_rate > 0.0F);

    nco_crcf nco = ::nco_crcf_create(LIQUID_VCO);
    ::nco_crcf_set_phase(nco, 0.0F);
    ::nco_crcf_set_frequency(nco, (2.0F * M_PI * rotation_hz / sample_rate));

    if (m_ncoChannelRotator)
        ::nco_crcf_destroy(m_ncoChannelRotator);
    
    m_ncoChannelRotator = nco;
}

void CReceiverChannel::setDownsamplerParams(unsigned int interp, unsigned int decim, float bw) {
    assert(decim > 0U);

    m_downsampleInterp = interp;
    m_downsampleDecim = decim;

    for (unsigned int i = 0U; i < sizeof(m_rreDownsampler) / sizeof(m_rreDownsampler[0]); i++) {
        if (m_rreDownsampler[i]) {
            ::rresamp_crcf_destroy(m_rreDownsampler[i]);
            m_rreDownsampler[i] = 0;
        }
    }

    for (unsigned int i = 0U; i < m_activeChannels; i++) {
        m_rreDownsampler[i] = ::rresamp_crcf_create_kaiser(decim, interp, RESAMPLER_FILTER_DELAY, bw, 70.0F);
    }
}

void CReceiverChannel::rotateDown(std::complex<float>* in_samples, unsigned int num_samples, std::complex<float>* out_samples) {
    assert(in_samples != nullptr);
	assert(out_samples != nullptr);

	::nco_crcf_mix_block_down(m_ncoChannelRotator, in_samples, out_samples, num_samples);
}

void CReceiverChannel::channelize(std::complex<float>* in_samples, std::complex<float>* out_samples) {
    ::firpfbch_crcf_analyzer_execute(m_firChannelAnalyzer, in_samples, out_samples);
}

void CReceiverChannel::downsample(unsigned int channel, std::complex<float>* in_samples,
                unsigned int num_samples, std::complex<float>* out_samples) {

    assert(in_samples != nullptr);
    assert(out_samples != nullptr);

    // TODO - Eliminate the heap-allocation 

    unsigned int interp = m_downsampleInterp;
    unsigned int decim  = m_downsampleDecim;

    // Interpolation and decimation are reversed when downsampling
    unsigned int p_in = num_samples / interp;
    std::complex<float>* in_buf  = new std::complex<float>[interp];
    std::complex<float>* out_buf = new std::complex<float>[decim];

    for (unsigned int i = 0U; i < p_in; i++) {
        ::memcpy(in_buf, in_samples + (i * interp), interp * sizeof(std::complex<float>));
        ::rresamp_crcf_execute(m_rreDownsampler[channel], in_buf, out_buf);
        ::memcpy(out_samples + (i * decim), out_buf, decim * sizeof(std::complex<float>));
    }

    delete[] in_buf;
    delete[] out_buf;        
}

void CReceiverChannel::demodulate(unsigned int channel, std::complex<float>* in_samples, const unsigned int num_samples, float* out_samples)
{
    assert(channel < m_activeChannels);
    assert(in_samples != nullptr);
    assert(out_samples != nullptr);

    ::freqdem_demodulate_block(m_FMdemod[channel], in_samples, num_samples, out_samples);
}