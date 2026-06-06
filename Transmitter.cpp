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
#include "Transmitter.h"


Transmitter::Transmitter(Device* device, FMMod* fm_mod, Resampler* resampler, Rotator* rotator,
                         Channelizer* channelizer, DMRTiming* burst_timer,
                         unsigned int num_active_channels, unsigned int num_pfb_channels, bool needs_timestamp) : 
m_running(true),
m_stopped(false),
m_timingInit(false),
m_timestamping(needs_timestamp),
m_activeChannels(num_active_channels),
m_pfbChannels(num_pfb_channels),
m_timingCorrection(0LL),
m_device(device),
m_fmMod(fm_mod),
m_resampler(resampler),
m_rotator(rotator),
m_channelizer(channelizer),
m_burstTimer(burst_timer)
{
  assert(m_activeChannels <= MAX_MMDVM_CHANNELS);
  assert(m_pfbChannels > 1U);
  assert(m_pfbChannels <= MAX_PFB_CHANNELS);
  for(unsigned i = 0;i < m_activeChannels;i++)
  {
    m_zmqCtx[i] = zmq::context_t(1);
    m_zmqSocket[i] = zmq::socket_t(m_zmqCtx[i], ZMQ_REQ);
    m_zmqSocket[i].set(zmq::sockopt::sndhwm, 10);
    m_zmqSocket[i].set(zmq::sockopt::linger, 0);
    int socket_no = i + 1;
    m_zmqSocket[i].connect ("ipc:///tmp/mmdvm-tx" + std::to_string(socket_no) + ".ipc");
    m_sn[i] = 1;
  }
  unsigned int max_chan_real = m_pfbChannels / 2U - 1U;
  max_chan_real = std::min<unsigned int>(max_chan_real, 4U);
  m_fillReal = std::min<unsigned int>(max_chan_real, m_activeChannels);
  
}

Transmitter::~Transmitter()
{
  for(unsigned i = 0;i < m_activeChannels;i++)
  {
    m_zmqSocket[i].close();
    m_zmqCtx[i].shutdown();
    m_zmqCtx[i].close();
  }
}

void Transmitter::start()
{
  m_thread = std::thread(&Transmitter::run, this);
  m_thread.detach();
}


void Transmitter::stop()
{
  m_running = false;
}

bool Transmitter::stopped() const
{
  return m_stopped;
}

void Transmitter::nextSlot(unsigned int channel)
{
  // idle channel will keep DMR timing so other modes can run at the same time
  if(m_sn[channel] == 2)
    m_sn[channel] = 1;
  else
    m_sn[channel] = 2;
}

void Transmitter::getZMQMessage()
{
  for(unsigned j=0; j < m_activeChannels; j++)
  {
    zmq::message_t mq_message;
    int size = 0;
    zmq::recv_result_t recv_result;
    zmq::message_t request_msg (1);
    ::memcpy (request_msg.data(), "s", sizeof(char));
    m_zmqSocket[j].send (request_msg, zmq::send_flags::none);
    recv_result = m_zmqSocket[j].recv(mq_message);
    size = mq_message.size();
    if(size < 1)
    {
      continue;
    }
    unsigned int buf_size = 0;
    ::memcpy(&buf_size, (uint8_t*)mq_message.data(), sizeof(uint32_t));
    if(buf_size == SAMPLES_PER_SLOT)
    {
      uint8_t control[SAMPLES_PER_SLOT];
      int16_t data[SAMPLES_PER_SLOT];
      ::memcpy(&control, (uint8_t*)mq_message.data() + sizeof(uint32_t), buf_size * sizeof(uint8_t));
      
      ::memcpy(&data, (uint8_t*)mq_message.data() + sizeof(uint32_t) + buf_size * sizeof(uint8_t),
             buf_size * sizeof(int16_t));
      for(unsigned i=0; i<buf_size; i++)
      {
        m_controlBuf[j].push_back(control[i]);
        m_dataBuf[j].push_back(float(data[i])/32767.0f);
      }
    }
  }
}

void Transmitter::run()
{
  while(m_running)
  {
    long long timeNs = 0LL;

    if(!m_timingInit)
    {
      m_burstTimer->lock();
      bool has_time = true;
      for(unsigned i=0;i<m_activeChannels;i++)
      {
        if(!m_burstTimer->getInit(i))
        {
          has_time = false;
          break;
        }
      }
      m_burstTimer->unlock();
      if(has_time)
        m_timingInit = true;
    }

    if(!m_timingInit)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(2));
      continue;
    }

    getZMQMessage();

    if(m_timingCorrection > 0)
    {
      // don't attempt to get next batch of samples too fast, DMRTX generates them on demand
      std::this_thread::sleep_for(std::chrono::nanoseconds(m_timingCorrection));
      m_timingCorrection = 0;
    }

    bool channel_idle[MAX_MMDVM_CHANNELS];
    for(unsigned i=0;i<MAX_MMDVM_CHANNELS;i++)
      channel_idle[i] = false;

    m_burstTimer->lock(); // need to wait for all RX channels to have up-to-date time reference
    for(unsigned i=0;i<m_activeChannels;i++)
    {
      long long time = 0LL;
      if(m_dataBuf[i].size() < 1)
      {
        channel_idle[i] = true;
        m_dataBuf[i].insert(m_dataBuf[i].begin(), SAMPLES_PER_SLOT, 1.0e-9f);
        m_controlBuf[i].insert(m_controlBuf[i].begin(), SAMPLES_PER_SLOT, MARK_NONE);
        time = m_burstTimer->allocateSlot(m_sn[i], m_timingCorrection, i) - (710LL * TIME_PER_SAMPLE);
        if(timeNs == 0LL)
          timeNs = time;
        nextSlot(i);
      }
      else
      {
        for(unsigned j=0;j<m_controlBuf[i].size();j++)
        {
          uint8_t control = m_controlBuf[i].at(j);
          if(control == MARK_SLOT1)
          {
            m_sn[i] = 1;
            time = m_burstTimer->allocateSlot(1U, m_timingCorrection, i) - (j * TIME_PER_SAMPLE);
            if(timeNs == 0LL)
              timeNs = time;
          }
          if(control == MARK_SLOT2)
          {
            m_sn[i] = 2;
            time = m_burstTimer->allocateSlot(2U, m_timingCorrection, i) - (j * TIME_PER_SAMPLE);
            if(timeNs == 0LL)
              timeNs = time;
          }
        }
      }
    }
    m_burstTimer->unlock();

    std::complex<float> output_samples[TX_SAMP_OUT_SIZE] = {0.0f, 0.0f};
    processSamples(output_samples, channel_idle);
    m_writeBuffer.insert(m_writeBuffer.end(), output_samples, output_samples + TX_INTERP_OUT_SIZE * m_pfbChannels);
    void *buffs[1] = {(void*)m_writeBuffer.data()};
    int flags = 0;
    if(m_timestamping && (timeNs > 0LL))  // only needed if there is at least one DMR channel or one idle channel with time info
      flags |= SOAPY_SDR_HAS_TIME;


    // FIXME: Pluto stream MTU !!
    int ret = m_device->getDevice()->writeStream(m_device->getTxStream(), buffs, m_writeBuffer.size(), flags, timeNs);
    if (ret <= 0)
    {
      ::fprintf(stderr,"Error writing samples to device: %s\n", SoapySDR_errToStr(ret));
    }
    else if ((unsigned int)ret != (TX_INTERP_OUT_SIZE * m_pfbChannels))
    {
      ::fprintf(stderr,"TX overrun occured, only wrote %d samples!\n", ret);
    }
    m_writeBuffer.erase(m_writeBuffer.begin(), m_writeBuffer.begin() + TX_INTERP_OUT_SIZE * m_pfbChannels);
  }
  m_stopped = true;
}

void Transmitter::processSamples(std::complex<float>* output_samples, bool* channel_idle)
{
  std::complex<float> channelizer_samples[MAX_MMDVM_CHANNELS][TX_INTERP_OUT_SIZE] = {{0.0f, 0.0f}};
  for(unsigned i=0;i<m_activeChannels;i++)
  {
    if(m_dataBuf[i].size() >= SAMPLES_PER_SLOT)
    {
      std::complex<float> freq_modulated[SAMPLES_PER_SLOT] = {0.0f, 0.0f};
      m_fmMod->modulate(i, m_dataBuf[i].data(), SAMPLES_PER_SLOT, freq_modulated);
      if(channel_idle[i])
      {
        for(unsigned j=0;j<SAMPLES_PER_SLOT;j++)
        {
          freq_modulated[j] = {1.0e-9f, 1.0e-9f}; // zero it but keep next phase step in freqmod
        }
      }
      std::complex<float> resampled[TX_INTERP_OUT_SIZE] = {0.0f, 0.0f};
      m_resampler->upsample(i, freq_modulated, SAMPLES_PER_SLOT, resampled);
      ::memcpy(&channelizer_samples[i][0], resampled, TX_INTERP_OUT_SIZE * sizeof(std::complex<float>));
      m_dataBuf[i].erase(m_dataBuf[i].begin(), m_dataBuf[i].begin() + SAMPLES_PER_SLOT);
      m_controlBuf[i].erase(m_controlBuf[i].begin(), m_controlBuf[i].begin() + SAMPLES_PER_SLOT);
    }
  }
  
  std::complex<float> channelized[TX_SAMP_OUT_SIZE] = {0.0f, 0.0f};
  std::complex<float> channels[MAX_PFB_CHANNELS] = {1.0e-9f, 1.0e-9f};;

  for(unsigned i=0; i<TX_INTERP_OUT_SIZE; i++)
  {
    for (unsigned k=0; k<m_fillReal; k++)
    {
      channels[k] = channelizer_samples[k][i] * TX_DAC_SCALING / float(m_activeChannels);
    }
    for (unsigned m=m_pfbChannels-1U, p=m_fillReal; p<m_activeChannels; m--,p++)
    {
      channels[m] = channelizer_samples[p][i] * TX_DAC_SCALING / float(m_activeChannels);
    }
    m_channelizer->synthesize(channels, &channelized[i*m_pfbChannels]);
  }
  m_rotator->rotate(channelized, TX_INTERP_OUT_SIZE * m_pfbChannels, output_samples);
}

