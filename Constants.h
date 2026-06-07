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
static const long long SLOT_TIME = 30000000LL;     // nanosec
static const long long TIME_PER_SAMPLE = 41667LL;  // nanosec
static const long long SAMPLES_PER_SLOT = 720LL;   // at 24k

static const unsigned int MAX_MMDVM_CHANNELS = 7U;
static const unsigned int MAX_PFB_CHANNELS = 40U;  // max sample rate 1M
static const unsigned int MAX_SAMPLE_RATE = 1000000U;
static const unsigned int PFB_FILTER_DELAY = 48U;  // must reduce on RPi platforms if bursts are consistently late due to CPU load
static const unsigned int RESAMPLER_FILTER_DELAY = 24U; // ntaps = 2 * delay + 1
static const long long TOTAL_FILTER_DELAY = ((2LL * RESAMPLER_FILTER_DELAY) + (2LL * PFB_FILTER_DELAY)) * TIME_PER_SAMPLE;

static const float MAX_TX_DAC_SCALE = 0.98f;

static const unsigned int TX_INTERP_OUT_SIZE = 750U;
static const unsigned int RX_INTERP_IN_SIZE = 75U;
static const unsigned int RX_SAMP_OUT_SIZE = 72U;
static const unsigned int TX_SAMP_OUT_SIZE = TX_INTERP_OUT_SIZE * MAX_PFB_CHANNELS;
static const unsigned int RX_SAMP_IN_SIZE = RX_INTERP_IN_SIZE * MAX_PFB_CHANNELS;






#endif // DEFINES_H
