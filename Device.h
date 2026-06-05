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

#ifndef DEVICE_H
#define DEVICE_H

#include <string>
#include <SoapySDR/Device.hpp>
#include <SoapySDR/Logger.hpp>
#include "Constants.h"


class Device
{
public:

    Device(std::string deviceType, std::string modemURI, double sampleRate, float rxFreq, float txFreq,
           float rxGain, float txGain, std::string rxAntenna, std::string txAntenna, bool debug);
    ~Device();
    SoapySDR::Stream* getTxStream() { return m_txStream;};
    SoapySDR::Stream* getRxStream() { return m_rxStream;};
    SoapySDR::Device* getDevice();
    bool getSoapyInit() const;
    
private:
    std::string          m_soapyDeviceType;
    std::string          m_soapyDeviceURI;
    SoapySDR::Device*    m_device;
    SoapySDR::Stream*    m_rxStream;
    SoapySDR::Stream*    m_txStream;
    double               m_sampleRate;
    float                m_soapyTXFreq;
    float                m_soapyRXFreq;
    float                m_soapyTXGain;
    float                m_soapyRXGain;
    std::string          m_rxAntenna;
    std::string          m_txAntenna;
    bool                 m_soapyInit;

};

#endif // DEVICE_H
