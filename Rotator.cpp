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

#include "Rotator.h"

Rotator::Rotator(float rotation_hz, float sample_rate)
{
  m_ncoD = nco_crcf_create(LIQUID_VCO);
  m_ncoU = nco_crcf_create(LIQUID_VCO);
  nco_crcf_set_phase(m_ncoD, 0.0f);
  nco_crcf_set_phase(m_ncoU, 0.0f);
  nco_crcf_set_frequency(m_ncoD, (2.0f*M_PI*rotation_hz/sample_rate));
  nco_crcf_set_frequency(m_ncoU, (2.0f*M_PI*rotation_hz/sample_rate));
}

Rotator::~Rotator()
{
  nco_crcf_destroy(m_ncoD);
  nco_crcf_destroy(m_ncoU);
}

void Rotator::rotate(std::complex<float>* in_samples, unsigned int num_samples, std::complex<float>* out_samples)
{
  nco_crcf_mix_block_up(m_ncoU, in_samples, out_samples, num_samples);
}

void Rotator::derotate(std::complex<float>* in_samples, unsigned int num_samples, std::complex<float>* out_samples)
{
  nco_crcf_mix_block_down(m_ncoD, in_samples, out_samples, num_samples);
}
