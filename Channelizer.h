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

#ifndef SYNTHANALYSIS_H
#define SYNTHANALYSIS_H

#include <complex>
#include <cmath>
#include <liquid/liquid.h>
#include "Constants.h"


class Channelizer
{
public:
    Channelizer(unsigned int num_pfb_channels);
    ~Channelizer();
    void synthesize(std::complex<float>* in_samples, std::complex<float>* out_samples);
    void channelize(std::complex<float>* in_samples, std::complex<float>* out_samples);

private:
    unsigned int m_channels;
    firpfbch_crcf m_synthesizer;
    firpfbch_crcf m_analyzer;
};

#endif // SYNTHANALYSIS_H
