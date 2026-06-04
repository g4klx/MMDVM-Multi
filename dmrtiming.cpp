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

#include "dmrtiming.h"

DMRTiming::DMRTiming(unsigned int burst_delay)
{
    m_burstDelay = (long long)burst_delay * 1000000LL;
    for(unsigned i = 0;i < MAX_MMDVM_CHANNELS;i++)
        m_sampleCounter[i] = 0;
    for(unsigned i = 0;i < MAX_MMDVM_CHANNELS;i++)
        m_lastSlot[i] = 0;
    for(unsigned i = 0;i < MAX_MMDVM_CHANNELS;i++)
        m_timeBase[i] = 0;
    for(unsigned i = 0;i < MAX_MMDVM_CHANNELS;i++)
        m_timingInitialized[i] = false;
}

DMRTiming::~DMRTiming()
{
    for(unsigned k = 0;k < MAX_MMDVM_CHANNELS;k++)
    {
        for(unsigned i=0;i<m_timeSlots[k].size();i++)
        {
            delete m_timeSlots[k].at(i);
        }
        m_timeSlots[k].clear();
    }
}

void DMRTiming::lock()
{
    m_timingMutex.lock();
}

void DMRTiming::unlock()
{
    m_timingMutex.unlock();
}


long long DMRTiming::getTimeDelta(unsigned int cn)
{
    assert(cn < MAX_MMDVM_CHANNELS);
    return m_timeBase[cn] + m_sampleCounter[cn] * TIME_PER_SAMPLE;
}

void DMRTiming::setTimer(long long value, unsigned int cn)
{
    assert(cn < MAX_MMDVM_CHANNELS);
    m_sampleCounter[cn] = 0;
    m_timeBase[cn] = value;
    if(m_timeBase[cn] > 4LL * SLOT_TIME)
        m_timingInitialized[cn] = true;
}

bool DMRTiming::getInit(unsigned int cn)
{
    assert(cn < MAX_MMDVM_CHANNELS);
    return m_timingInitialized[cn];
}

uint8_t DMRTiming::checkTime(unsigned int cn, bool time_base_received)
{
    assert(cn < MAX_MMDVM_CHANNELS);
    if(!time_base_received)
    {
        m_sampleCounter[cn]++;
    }
    std::scoped_lock<std::mutex> guard(m_slotMutex[cn]);
    if(m_timeSlots[cn].size() < 1)
    {
        return 0;
    }
    TimeSlot* s = m_timeSlots[cn].at(0);
    
    long long sample_time = m_timeBase[cn] + m_sampleCounter[cn] * TIME_PER_SAMPLE - (2LL * 2LL * FILTER_DELAY * TIME_PER_SAMPLE);

    if(sample_time >= s->slotTime && s->slotSampleCounter == 0)
    {
        s->slotSampleCounter++;
        return s->slotNo;
    }
    else if(sample_time >= s->slotTime)
    {
        if(s->slotSampleCounter >= (SAMPLES_PER_SLOT - 1))
        {
            delete m_timeSlots[cn][0];
            m_timeSlots[cn].erase(m_timeSlots[cn].begin());
            return 0;
        }
        s->slotSampleCounter++;
    }
    return 0;
}

long long DMRTiming::allocateSlot(uint8_t slot_no, int64_t &timing, unsigned int cn)
{
    assert(cn < MAX_MMDVM_CHANNELS);
    long long elapsed = getTimeDelta(cn);
    if(elapsed <= m_lastSlot[cn])
    {
        if(cn == 0)
        {
            timing = m_lastSlot[cn] - elapsed;
        }
        m_lastSlot[cn] = m_lastSlot[cn] + SLOT_TIME;
    }
    else if(m_lastSlot[cn] == 0)
    {
        m_lastSlot[cn] = elapsed;
    }
    else if((elapsed - m_lastSlot[cn]) >= (1LL * SLOT_TIME))
    {
        m_lastSlot[cn] = elapsed;
    }
    else
    {
        m_lastSlot[cn] = m_lastSlot[cn] + SLOT_TIME;
    }
    long long nsec = m_lastSlot[cn] + m_burstDelay;
    TimeSlot *s = new TimeSlot;
    s->slotNo = slot_no;
    s->slotTime = nsec;
    s->slotSampleCounter = 0;
    std::unique_lock<std::mutex> guard(m_slotMutex[cn]);
    m_timeSlots[cn].push_back(s);
    return nsec;
}



