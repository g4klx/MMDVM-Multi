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

#include "channelizer.h"
#include <assert.h>

Channelizer::Channelizer(unsigned int num_pfb_channels) :
m_channels(num_pfb_channels)
{
  assert(m_channels > 1);
  m_synthesizer = firpfbch_crcf_create_kaiser(LIQUID_SYNTHESIZER, m_channels, PFB_FILTER_DELAY, 70.0f);
  m_analyzer = firpfbch_crcf_create_kaiser(LIQUID_ANALYZER, m_channels, PFB_FILTER_DELAY, 70.0f);
}

Channelizer::~Channelizer()
{
  firpfbch_crcf_destroy(m_synthesizer);
  firpfbch_crcf_destroy(m_analyzer);
}

void Channelizer::synthesize(std::complex<float>* in_samples, std::complex<float>* out_samples)
{
  firpfbch_crcf_synthesizer_execute(m_synthesizer, in_samples, out_samples);
}

void Channelizer::channelize(std::complex<float>* in_samples, std::complex<float>* out_samples)
{
  firpfbch_crcf_analyzer_execute(m_analyzer, in_samples, out_samples);
}
