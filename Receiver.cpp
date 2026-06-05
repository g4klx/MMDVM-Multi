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

#include "Receiver.h"


Receiver::Receiver(Device* device, FMMod* fm_mod, Resampler* resampler, Rotator* rotator,
                   Channelizer* channelizer, DMRTiming* burst_timer, unsigned int num_active_channels,
                   unsigned int num_pfb_channels, float power_calibration) : 
m_running(true),
m_stopped(false),
m_device(device),
m_fmMod(fm_mod),
m_resampler(resampler),
m_rotator(rotator),
m_channelizer(channelizer),
m_burstTimer(burst_timer),
m_activeChannels(num_active_channels),
m_pfbChannels(num_pfb_channels),
m_powerCalibration(power_calibration)
{
  assert(m_activeChannels <= MAX_MMDVM_CHANNELS);
  assert(m_pfbChannels > 1U);
  assert(m_pfbChannels <= MAX_PFB_CHANNELS);
  for(unsigned i = 0;i < m_activeChannels;i++)
  {
    m_zmqCtx[i] = zmq::context_t(1);
    m_zmqSocket[i] = zmq::socket_t(m_zmqCtx[i], ZMQ_PUSH);
    m_zmqSocket[i].set(zmq::sockopt::sndhwm, 100);
    m_zmqSocket[i].set(zmq::sockopt::linger, 0);
    int socket_no = i + 1;
    m_zmqSocket[i].bind ("ipc:///tmp/mmdvm-rx" + std::to_string(socket_no) + ".ipc");
    data_buf[i].reserve(SAMPLES_PER_SLOT);
    control_buf[i].reserve(SAMPLES_PER_SLOT);
    m_RSSI[i] = 0U;
  }
  unsigned int max_real_chan = m_pfbChannels / 2U - 1U;
  max_real_chan = std::min<unsigned int>(max_real_chan, 4U);
  m_fillReal = std::min<unsigned int>(max_real_chan, m_activeChannels);
}

Receiver::~Receiver()
{
  for(unsigned i = 0;i < m_activeChannels;i++)
  {
    m_zmqSocket[i].close();
    m_zmqCtx[i].shutdown();
    m_zmqCtx[i].close();
  }
}

void Receiver::start()
{
  m_thread = std::thread(&Receiver::run, this);
  m_thread.detach();
}

void Receiver::stop()
{
  m_running = false;
}

bool Receiver::stopped() const
{
  return m_stopped;
}

void Receiver::run()
{
  while(m_running)
  {
    std::complex<float> read_buffer[RX_SAMP_IN_SIZE];
    void *buffs[1] = {(void*)read_buffer};
    long long timeNs = 0LL;
    
    int flags = 0;
    int ret = m_device->getDevice()->readStream(m_device->getRxStream(), buffs, RX_SAMP_IN_SIZE, flags, timeNs);
    if (ret <= 0)
    {
      ::fprintf(stderr, "Error reading samples from device: %s\n", SoapySDR_errToStr(ret));
      std::this_thread::sleep_for(std::chrono::milliseconds(3));
      continue;
    }
    else if(ret != RX_SAMP_IN_SIZE)
    {
      ::fprintf(stderr, "Underrun occurred while reading samples from device, only read %d samples!\n", ret);
      std::this_thread::sleep_for(std::chrono::milliseconds(3));
      continue;
    }

    std::complex<float> rotated[RX_SAMP_IN_SIZE] = {0.0f, 0.0f};
    m_rotator->derotate(read_buffer, RX_SAMP_IN_SIZE, rotated);

    std::complex<float> channelized1[RX_INTERP_IN_SIZE][MAX_PFB_CHANNELS] = {{0.0f, 0.0f}};
    std::complex<float> channelized2[MAX_PFB_CHANNELS][RX_INTERP_IN_SIZE] = {{0.0f, 0.0f}};
    std::complex<float> rearranged[MAX_PFB_CHANNELS][RX_INTERP_IN_SIZE] = {{0.0f, 0.0f}};
    for(unsigned i=0;i<RX_INTERP_IN_SIZE;i++)
    {
      m_channelizer->channelize(&rotated[i*m_pfbChannels], &channelized1[i][0]);
    }

    for(unsigned i=0;i<m_pfbChannels;i++)
    {
      for(unsigned j=0;j<RX_INTERP_IN_SIZE;j++)
      {
        channelized2[i][j] = channelized1[j][i];
      }
    }

//    First four usable channels are on the real side of the FFT, the rest in imag,
//    reversed order to minimize occupied BW
//    Channel 5 of the PFB (how many total??) wraps around to the imag side and is not usable
//
//    Channel 7    Channel 6     Channel 5     Channel 1       Channel 2     Channel 3     Channel 4
//    434.7500     433.7750      434.8000      434.8250        434.8500      434.8750      434.9000
//    |             |            |             |               |             |              |
//    |             |            |             |               |             |              |
//    |             |            |             |               |             |              |
// ---|-------------|------------|-------------|---------------|-------------|--------------|---------
//                                         RX/TX frequency
//
    for (unsigned k=0; k<m_fillReal; k++)
    {
      for(unsigned j=0;j<RX_INTERP_IN_SIZE;j++)
      {
        rearranged[k][j] = channelized2[k][j];
      }
    }
    for (unsigned m=m_pfbChannels-1, p=m_fillReal; p<m_activeChannels; m--,p++)
    {
      for(unsigned j=0;j<RX_INTERP_IN_SIZE;j++)
      {
        rearranged[p][j] = channelized2[m][j];
      }
    }

    m_burstTimer->lock();
    for(unsigned int j=0;j<m_activeChannels;j++)
    {
      m_burstTimer->setTimer(timeNs, j);
      float output_samples[RX_SAMP_OUT_SIZE] = { 0.0f };
      processSamples(j, rearranged[j], output_samples);
      for(unsigned int i=0;i<RX_SAMP_OUT_SIZE;i++)
      {
        uint8_t control = MARK_NONE;
        uint8_t slot_no = m_burstTimer->checkTime(j, i==0);
        if(slot_no == 1)
          control = MARK_SLOT1;
        if(slot_no == 2)
          control = MARK_SLOT2;
        int32_t s = int32_t(32767.0f * output_samples[i]);
        s = (s > 32767) ? 32767 : s;
        s = (s < -32767) ? -32767 : s;
        int16_t sample = int16_t(s);
        data_buf[j].push_back(sample);
        control_buf[j].push_back(control);
      }
    }
    m_burstTimer->unlock();

    for(unsigned int j=0;j<m_activeChannels;j++)
    {
      if(data_buf[j].size() >= SAMPLES_PER_SLOT)
      {
        unsigned int rssi = m_RSSI[j];
        uint32_t num_items = SAMPLES_PER_SLOT;
        int buf_size = 2 * sizeof(uint32_t) + num_items * sizeof(uint8_t) + num_items * sizeof(int16_t);
        zmq::message_t reply (buf_size);
        ::memcpy (reply.data (), &num_items, sizeof(uint32_t));
        ::memcpy ((unsigned char *)reply.data () + sizeof(uint32_t), &rssi, sizeof(uint32_t));
        ::memcpy ((unsigned char *)reply.data () + 2 * sizeof(uint32_t),
                  (unsigned char *)control_buf[j].data(), num_items * sizeof(uint8_t));
        ::memcpy ((unsigned char *)reply.data () + 2 * sizeof(uint32_t) + num_items * sizeof(uint8_t),
                (unsigned char *)data_buf[j].data(), num_items*sizeof(int16_t));
        m_zmqSocket[j].send (reply, zmq::send_flags::dontwait);
        data_buf[j].erase(data_buf[j].begin(), data_buf[j].begin() + num_items);
        control_buf[j].erase(control_buf[j].begin(), control_buf[j].begin() + num_items);
        data_buf[j].reserve(SAMPLES_PER_SLOT);
        control_buf[j].reserve(SAMPLES_PER_SLOT);
        m_RSSI[j] = 0U;
      }
    }
  }
  m_stopped = true;
}

void Receiver::processSamples(unsigned int channel, std::complex<float>* in_samples, float* output_samples)
{
  std::complex<float> resampled[RX_SAMP_OUT_SIZE] = {0.0f, 0.0f};
  m_resampler->downsample(channel, in_samples, RX_INTERP_IN_SIZE, resampled);
  float sum = 0.0f;
  for(unsigned i=0;i<RX_SAMP_OUT_SIZE;i++)
  {
    sum += (resampled[i].real() * resampled[i].real()) + (resampled[i].imag() * resampled[i].imag());
  }
  float rms = std::sqrtf(sum / float(RX_SAMP_OUT_SIZE));
  float db = 10.0f * std::log10f(rms + 1.0e-20f) - m_powerCalibration;
  unsigned int rssi = (unsigned int)std::fabs(db); // inverted to positive values since RSSI > 0 dBm is unlikely
  if(rssi > m_RSSI[channel]) // keep max value since we may span two timeslots, one active one inactive
    m_RSSI[channel] = rssi;
  m_fmMod->demodulate(channel, resampled, RX_SAMP_OUT_SIZE, output_samples);
}


