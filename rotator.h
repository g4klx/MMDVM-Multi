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

#ifndef ROTATOR_H
#define ROTATOR_H

#include <complex>
#include <string.h>
#include <liquid/liquid.h>
#include "constants.h"

class Rotator
{
public:
    Rotator(float rotation_hz=12000.0f, float sample_rate=250000.0f);
    ~Rotator();
    void rotate(std::complex<float>* in_samples, unsigned int num_samples, std::complex<float>* out_samples);
    void derotate(std::complex<float>* in_samples, unsigned int num_samples, std::complex<float>* out_samples);
    
private:
    nco_crcf m_ncoD;
    nco_crcf m_ncoU;

};

#endif // ROTATOR_H
