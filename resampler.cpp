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

#include <assert.h>
#include "resampler.h"

Resampler::Resampler(unsigned interp, unsigned decim, float bw, unsigned int num_channels) :
m_decim(decim),
m_interp(interp),
m_activeChannels(num_channels)
{
  assert(m_activeChannels <= MAX_MMDVM_CHANNELS);
  assert(m_decim > 0);
  assert(m_interp > 0);
  for(unsigned i=0;i<m_activeChannels;i++)
  {
    m_upsampler[i] = rresamp_crcf_create_kaiser(interp, decim, RESAMPLER_FILTER_DELAY, bw, 70.0f);
    m_downsampler[i] = rresamp_crcf_create_kaiser(decim, interp, RESAMPLER_FILTER_DELAY, bw, 70.0f);
  }
}

Resampler::~Resampler()
{
  for(unsigned i=0;i<m_activeChannels;i++)
  {
    rresamp_crcf_destroy(m_upsampler[i]);
    rresamp_crcf_destroy(m_downsampler[i]);
  }
}

void Resampler::upsample(unsigned int channel, std::complex<float>* in_samples,
                         unsigned int num_samples, std::complex<float>* out_samples)
{
  unsigned int p_in = num_samples / m_decim;
  std::complex<float>* in_buf = new std::complex<float>[m_decim];
  std::complex<float>* out_buf = new std::complex<float>[m_interp];
  for(unsigned int i=0;i<p_in;i++)
  {
    ::memcpy(in_buf, in_samples + (i * m_decim), m_decim * sizeof(std::complex<float>));
    rresamp_crcf_execute(m_upsampler[channel], in_buf, out_buf);
    ::memcpy(out_samples + (i * m_interp), out_buf, m_interp * sizeof(std::complex<float>));
  }
  delete[] in_buf;
  delete[] out_buf;
}

void Resampler::downsample(unsigned int channel, std::complex<float>* in_samples,
                           unsigned int num_samples, std::complex<float>* out_samples)
{
  unsigned int p_in = num_samples / m_interp;
  std::complex<float>* in_buf = new std::complex<float>[m_interp];
  std::complex<float>* out_buf = new std::complex<float>[m_decim];
  for(unsigned int i=0;i<p_in;i++)
  {
    ::memcpy(in_buf, in_samples + (i * m_interp), m_interp * sizeof(std::complex<float>));
    rresamp_crcf_execute(m_downsampler[channel], in_buf, out_buf);
    ::memcpy(out_samples + (i * m_decim), out_buf, m_decim * sizeof(std::complex<float>));
  }
  delete[] in_buf;
  delete[] out_buf;
}


