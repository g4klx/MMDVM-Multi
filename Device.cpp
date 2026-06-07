/*
 *   Copyright (C) 2025, 2026 by Jonathan Naylor G4KLX
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
#include "Device.h"

Device::Device(std::string deviceType, std::string modemURI, double sampleRate, float rxFreq, float txFreq,
               float rxGain, float txGain, std::string rxAntenna, std::string txAntenna, bool debug) :
m_soapyDeviceType(deviceType),
m_soapyDeviceURI(modemURI),
m_device(nullptr),
m_rxStream(nullptr),
m_txStream(nullptr),
m_sampleRate(sampleRate),
m_soapyTXFreq(txFreq),
m_soapyRXFreq(rxFreq),
m_soapyTXGain(txGain),
m_soapyRXGain(rxGain),
m_rxAntenna(rxAntenna),
m_txAntenna(txAntenna),
m_soapyInit(false)
{
  SoapySDR::Kwargs devArgs;
  SoapySDR::Kwargs rxArgs;
  SoapySDR::Kwargs txArgs;
  if(debug)
    SoapySDR::setLogLevel(SOAPY_SDR_DEBUG);
  else
    SoapySDR::setLogLevel(SOAPY_SDR_INFO);

  const char* LIME_DEFAULT_URI  = "index=0";         // eg: addr=1111:2222 or serial=xxxxxxxx
  const char* PLUTO_DEFAULT_URI = "ip:pluto.local";

  if (m_soapyDeviceType.compare("plutosdr") == 0 || m_soapyDeviceType.compare("pluto") == 0) {
    const char* uri = m_soapyDeviceURI.empty() ? PLUTO_DEFAULT_URI : m_soapyDeviceURI.c_str();
    std::string rxBufLen = std::to_string(RX_INTERP_IN_SIZE * MAX_PFB_CHANNELS);
    std::string txBufLen = std::to_string(TX_INTERP_OUT_SIZE * MAX_PFB_CHANNELS);
    devArgs["driver"] = "plutosdr";
    rxArgs["uri"]     = uri;
    txArgs["uri"]     = uri;
    rxArgs["bufflen"] = rxBufLen.c_str();
    txArgs["bufflen"] = txBufLen.c_str();
    ::fprintf(stdout, "Using Pluto SDR driver uri %s\n", uri);
  } else if (m_soapyDeviceType.compare("limesdr") == 0 || m_soapyDeviceType.compare("lime") == 0) {
    const char* uri = m_soapyDeviceURI.empty() ? LIME_DEFAULT_URI : m_soapyDeviceURI.c_str();
    devArgs["driver"] = "lime";
    rxArgs["uri"]     = uri;
    txArgs["uri"]     = uri;
    rxArgs["latency"] = "0";
    txArgs["latency"] = "0";
    ::fprintf(stdout, "Using Lime SDR driver uri %s\n", uri);
  } else if (m_soapyDeviceType.compare("usrp") == 0) {
    const char* uri = m_soapyDeviceURI.c_str();
    devArgs["driver"] = "uhd";
    rxArgs["uri"]     = uri;
    txArgs["uri"]     = uri;
    rxArgs["recv_frame_size"] = "1024";
    ::fprintf(stdout, "Using Ettus USRP driver uri %s\n", uri);
  } else if (m_soapyDeviceType.compare("mucell") == 0) {
    devArgs["driver"] = "mucell";
  } else {
    devArgs["driver"] = "sx";
    ::fprintf(stdout, "Using SX1255 driver \n");
  }

  try {
    m_device = SoapySDR::Device::make(devArgs);
    assert(m_device != nullptr);
    
    m_device->setSampleRate(SOAPY_SDR_RX, 0, m_sampleRate);
    m_device->setSampleRate(SOAPY_SDR_TX, 0, m_sampleRate);
    
    m_device->setFrequency(SOAPY_SDR_RX, 0, m_soapyRXFreq);
    m_device->setFrequency(SOAPY_SDR_TX, 0, m_soapyTXFreq);
    
    m_device->setAntenna(SOAPY_SDR_RX, 0, m_rxAntenna);
    m_device->setAntenna(SOAPY_SDR_TX, 0, m_txAntenna);

    m_device->setGain(SOAPY_SDR_RX, 0, m_soapyRXGain);
    m_device->setGain(SOAPY_SDR_TX, 0, m_soapyTXGain);

    m_device->setBandwidth(SOAPY_SDR_RX, 0, m_sampleRate);
    m_device->setBandwidth(SOAPY_SDR_TX, 0, m_sampleRate);


    m_rxStream = m_device->setupStream(SOAPY_SDR_RX, "CF32", {0}, rxArgs);
    m_txStream = m_device->setupStream(SOAPY_SDR_TX, "CF32", {0}, txArgs);
    m_device->activateStream(m_txStream);
    m_device->activateStream(m_rxStream);
    
    assert(m_rxStream != nullptr);
    assert(m_txStream != nullptr);

    m_rxMTU = m_device->getStreamMTU(m_rxStream);
    m_txMTU = m_device->getStreamMTU(m_txStream);

    ::fprintf(stdout, "Soapy device initialized\n");
    ::fprintf(stdout, "RX stream MTU is %d\n", m_rxMTU);
    ::fprintf(stdout, "TX stream MTU is %d\n", m_txMTU);
    m_soapyInit = true;
  } catch (std::runtime_error &e) {
    ::fprintf(stderr, "Soapy device failed to initialize\n");
  }
  
}

Device::~Device()
{
  if (m_device != nullptr) {
    assert(m_rxStream != nullptr);
    assert(m_txStream != nullptr);
    
    if (m_soapyInit) {
      m_device->deactivateStream(m_rxStream, 0, 0);
      m_device->deactivateStream(m_txStream, 0, 0);
    }
    
    m_device->closeStream(m_rxStream);
    m_device->closeStream(m_txStream);
    
    SoapySDR::Device::unmake(m_device);
  }
  ::fprintf(stderr, "Soapy device closed\n");
  m_rxStream = nullptr;
  m_txStream = nullptr;
  m_device   = nullptr;
}

bool Device::getSoapyInit() const
{
  return m_soapyInit;
}

SoapySDR::Device* Device::getDevice()
{
  return m_device;
}

unsigned int Device::getRxMTU() const
{
  return m_rxMTU;
}

unsigned int Device::getTxMTU() const
{
  return m_txMTU;
}
