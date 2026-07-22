/*
 *   Copyright (C) 2015-2023,2025,2026 by Jonathan Naylor G4KLX
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

#if !defined(CONF_H)
#define	CONF_H

#include <string>
#include <cstdint>

class CConf
{
public:
	CConf(const std::string& file);
	~CConf();

	bool read();

	// The General section
	bool         getDaemon() const;
	unsigned int getNumChannels() const;
	int          getSampleDelay() const;
	unsigned int getRFDelay() const;
	unsigned int getRSSICalibration() const;
	unsigned int getSampleRate() const;
	unsigned int getDigitalGain() const;
	unsigned int getSymbolDeviation() const;

	// The Log section
	unsigned int getLogDisplayLevel() const;
	unsigned int getLogMQTTLevel() const;

	// The MQTT section
	std::string    getMQTTHost() const;
	unsigned short getMQTTPort() const;
	unsigned int   getMQTTKeepalive() const;
	std::string    getMQTTName() const;
	bool           getMQTTAuthEnabled() const;
	std::string    getMQTTUsername() const;
	std::string    getMQTTPassword() const;

	// The Modem section
	std::string  getModemType() const;
	std::string  getModemURI() const;
	unsigned int getRxGain() const;
	unsigned int getTxGain() const;
	unsigned int getRxFreq() const;
	unsigned int getTxFreq() const;
	std::string  getRxAntenna() const;
	std::string  getTxAntenna() const;
	bool         getModemTrace() const;

	// The MMDVM section
	std::string  getNetworkModemAddress() const;
	unsigned short getNetworkModemPort() const;
	std::string  getNetworkLocalAddress() const;
	unsigned short getNetworkLocalPort() const;
	bool         getNetworkTrace() const;

	// The SVXLink section
	std::string  getBridgeModemAddress() const;
	unsigned short getBridgeModemPort() const;
	std::string  getBridgeLocalAddress() const;
	unsigned short getBridgeLocalPort() const;
	std::string  getBridgeSVXAddress() const;
	unsigned short getBridgeSVXPort() const;
	std::string  getBridgeLocalSVXAddress() const;
	unsigned short getBridgeLocalSVXPort() const;

	unsigned int getBridgeRxGain() const;
	unsigned int getBridgeTxGain() const;

	int getBridgeSquelch() const;

private:
	std::string m_file;

	bool         m_daemon;
	unsigned int m_numChannels;
	int          m_sampleDelay;
	unsigned int m_RFDelay;
	unsigned int m_RSSICalibration;
	unsigned int m_sampleRate;
	unsigned int m_digitalGain;
	unsigned int m_symbolDeviation;

	unsigned int m_logDisplayLevel;
	unsigned int m_logMQTTLevel;

	std::string  m_mqttHost;
	unsigned short m_mqttPort;
	unsigned int m_mqttKeepalive;
	std::string  m_mqttName;
	bool         m_mqttAuthEnabled;
	std::string  m_mqttUsername;
	std::string  m_mqttPassword;

	std::string  m_modemType;
	std::string  m_modemURI;
	unsigned int m_rxFreq;
	unsigned int m_txFreq;
	unsigned int m_rxGain;
	unsigned int m_txGain;
	std::string  m_rxAntenna;
	std::string  m_txAntenna;
	bool         m_modemTrace;

	std::string  m_networkModemAddress;
	unsigned short m_networkModemPort;
	std::string  m_networkLocalAddress;
	unsigned short m_networkLocalPort;
	bool         m_networkTrace;

	std::string  m_svxBridgeModemAddress;
	unsigned short m_svxBridgeModemPort;
	std::string  m_svxBridgeLocalAddress;
	unsigned short m_svxBridgeLocalPort;

	std::string  m_svxBridgeSVXAddress;
	unsigned short m_svxBridgeSVXPort;
	std::string  m_svxBridgeLocalSVXAddress;
	unsigned short m_svxBridgeLocalSVXPort;

	unsigned int m_svxBridgeRxGain;
	unsigned int m_svxBridgeTxGain;

	int m_svxBridgeSquelch;
};

#endif
