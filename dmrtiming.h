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

#ifndef DMRTIMING_H
#define DMRTIMING_H

#include <mutex>
#include <chrono>
#include <cstdint>
#include <vector>
#include <assert.h>
#include "constants.h"

class TimeSlot {
public:
    TimeSlot() {}; 
    ~TimeSlot() {};
    uint8_t slotNo;
    long long slotTime;
    int16_t slotSampleCounter;
};

class DMRTiming
{
public:
    DMRTiming(int sample_delay);
    ~DMRTiming();
    void setTimer(long long value, unsigned int cn=0);
    uint8_t checkTime(unsigned int cn=0, bool time_base_received=false);
    long long allocateSlot(uint8_t slot_no, int64_t &timing, unsigned int cn=0);
    bool getInit(unsigned int cn=0);
    void lock();
    void unlock();

private:
    long long getTimeDelta(unsigned int cn=0);
    long long m_sampleDelay;
    bool m_timingInitialized[MAX_MMDVM_CHANNELS];
    std::mutex m_timingMutex;
    std::mutex m_slotMutex[MAX_MMDVM_CHANNELS];
    long long m_sampleCounter[MAX_MMDVM_CHANNELS];
    long long m_lastSlot[MAX_MMDVM_CHANNELS];
    long long m_timeBase[MAX_MMDVM_CHANNELS];
    std::vector<TimeSlot*> m_timeSlots[MAX_MMDVM_CHANNELS];

};


#endif // DMRTIMING_H
