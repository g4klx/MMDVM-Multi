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

#ifndef RESAMPLER_H
#define RESAMPLER_H

#include <complex>
#include <string.h>
#include <liquid/liquid.h>
#include "constants.h"

class Resampler
{
public:
    Resampler(unsigned int interp, unsigned int decim, float bw, unsigned int num_channels);
    ~Resampler();
    void upsample(unsigned int channel, std::complex<float>* in_samples,
                  unsigned int num_samples, std::complex<float>* out_samples);
    void downsample(unsigned int channel, std::complex<float>* in_samples,
                    unsigned int num_samples, std::complex<float>* out_samples);
    
private:
    unsigned int m_decim;
    unsigned int m_interp;
    unsigned int m_activeChannels;
    rresamp_crcf m_upsampler[MAX_MMDVM_CHANNELS];
    rresamp_crcf m_downsampler[MAX_MMDVM_CHANNELS];
};

#endif // RESAMPLER_H
