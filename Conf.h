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
	unsigned int getDelay() const;
	unsigned int getSampleRate() const;

	// The Log section
	unsigned int getLogDisplayLevel() const;

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

	// The MMDVMHost section
	std::string  getNetworkHostAddress() const;
	unsigned short getNetworkHostPort() const;
	std::string  getNetworkLocalAddress() const;
	unsigned short getNetworkLocalPort() const;
	bool         getNetworkTrace() const;

private:
	std::string m_file;

	bool         m_daemon;
	unsigned int m_numChannels;
	unsigned int m_delay;
	unsigned int m_sampleRate;

	unsigned int m_logDisplayLevel;

	std::string  m_modemType;
	std::string  m_modemURI;
	unsigned int m_rxFreq;
	unsigned int m_txFreq;
	unsigned int m_rxGain;
	unsigned int m_txGain;
	std::string  m_rxAntenna;
	std::string  m_txAntenna;
	bool         m_modemTrace;

	std::string  m_networkHostAddress;
	unsigned short m_networkHostPort;
	std::string  m_networkLocalAddress;
	unsigned short m_networkLocalPort;
	bool         m_networkTrace;
};

#endif
