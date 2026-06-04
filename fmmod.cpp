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

#include "fmmod.h"
#include <assert.h>

FMMod::FMMod(float deviation, unsigned int num_channels) :
m_activeChannels(num_channels)
{
  assert(m_activeChannels <= MAX_MMDVM_CHANNELS);
  for(unsigned i=0;i<m_activeChannels;i++)
  {
    m_FMmod[i] = freqmod_create(deviation);
    m_FMdemod[i] = freqdem_create(deviation);
  }
}

FMMod::~FMMod()
{
  for(unsigned i=0;i<m_activeChannels;i++)
  {
    freqmod_destroy(m_FMmod[i]);
    freqdem_destroy(m_FMdemod[i]);
  }
}

void FMMod::modulate(unsigned int channel, float* in_samples, const unsigned int num_samples, std::complex<float>* out_samples)
{
  freqmod_modulate_block(m_FMmod[channel], in_samples, num_samples, out_samples);
}

void FMMod::demodulate(unsigned int channel, std::complex<float>* in_samples, const unsigned int num_samples, float* out_samples)
{
  freqdem_demodulate_block(m_FMdemod[channel], in_samples, num_samples, out_samples);
}

