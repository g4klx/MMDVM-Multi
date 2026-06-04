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

#ifndef DEFINES_H
#define DEFINES_H

#include <cstdint>

static const uint8_t MARK_SLOT1 = 0x08U;
static const uint8_t MARK_SLOT2 = 0x04U;
static const uint8_t MARK_NONE  = 0x00U;
static const uint8_t NUMBER_OF_SLOTS = 2U;
static const int64_t BURST_DELAY = 30LL;           // default delay, msec
static const unsigned int MAX_MMDVM_CHANNELS = 7U;
static const unsigned int MAX_PFB_CHANNELS = 12U;  // max sample rate 300k
static const unsigned int FILTER_DELAY = 48U;      // must reduce on RPI platforms if bursts are consistently late due to CPU load
static const long long SLOT_TIME = 30000000LL;     // nanosec
static const long long TIME_PER_SAMPLE = 41667LL;  // nanosec
static const long long SAMPLES_PER_SLOT = 720LL;
static const float TX_DAC_SCALING = 0.35f;

// the following entries are specific for a sample rate of 250k (Lime, Pluto)
// TODO: handle SXCeiver sample rates
static const unsigned int TX_SAMP_OUT_SIZE = 7500U;
static const unsigned int TX_INTERP_OUT_SIZE = 750U;
static const unsigned int RX_SAMP_IN_SIZE = 750U;
static const unsigned int RX_INTERP_IN_SIZE = 75U;
static const unsigned int RX_SAMP_OUT_SIZE = 72U;



#endif // DEFINES_H
