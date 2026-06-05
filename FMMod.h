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

#ifndef FMMOD_H
#define FMMOD_H

#include <complex>
#include <cmath>
#include <liquid/liquid.h>
#include "Constants.h"


class FMMod
{
public:
    FMMod(float deviation, unsigned int num_channels);
    ~FMMod();
    void modulate(unsigned int channel, float* in_samples, const unsigned int num_samples, std::complex<float>* out_samples);
    void demodulate(unsigned int channel, std::complex<float>* in_samples, const unsigned int num_samples,float* out_samples);
    
private:
    unsigned int m_activeChannels;
    freqmod m_FMmod[MAX_MMDVM_CHANNELS];
    freqdem m_FMdemod[MAX_MMDVM_CHANNELS];
};

#endif // FMMOD_H
