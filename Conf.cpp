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
#include "Conf.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>

const int BUFFER_SIZE = 500;

enum class SECTION {
	NONE,
	GENERAL,
	LOG,
	MQTT,
	MODEM,
	MMDVM,
	SVXLink
};

CConf::CConf(const std::string& file) :
m_file(file),
m_daemon(false),
m_numChannels(1U),
m_sampleDelay(0),
m_RFDelay(0U),
m_RSSICalibration(0U),
m_sampleRate(250000),
m_digitalGain(35),
m_symbolDeviation(10),
m_logDisplayLevel(0U),
m_modemType("sx"),
m_modemURI(),
m_rxFreq(431800000U),
m_txFreq(433800000U),
m_rxGain(30U),
m_txGain(60U),
m_rxAntenna("Auto"),
m_txAntenna("Auto"),
m_modemTrace(false),
m_networkModemAddress("127.0.0.1"),
m_networkModemPort(48100U),
m_networkLocalAddress("127.0.0.1"),
m_networkLocalPort(48200U),
m_networkTrace(false),
m_svxBridgeModemAddress("127.0.0.1"),
m_svxBridgeModemPort(48104U),
m_svxBridgeLocalAddress("127.0.0.1"),
m_svxBridgeLocalPort(48204U),
m_svxBridgeSVXAddress("127.0.0.1"),
m_svxBridgeSVXPort(4937U),
m_svxBridgeLocalSVXAddress("127.0.0.1"),
m_svxBridgeLocalSVXPort(4938U),
m_svxBridgeRxGain(750),
m_svxBridgeTxGain(14)
{
}

CConf::~CConf()
{
}

bool CConf::read()
{
	FILE* fp = ::fopen(m_file.c_str(), "rt");
	if (fp == nullptr) {
		::fprintf(stderr, "Couldn't open the .ini file - %s\n", m_file.c_str());
		return false;
	}

	SECTION section = SECTION::NONE;

	char buffer[BUFFER_SIZE];
	while (::fgets(buffer, BUFFER_SIZE, fp) != nullptr) {
		if (buffer[0U] == '#')
			continue;

		if (buffer[0U] == '[') {
			if (::strncmp(buffer, "[General]", 9U) == 0)
				section = SECTION::GENERAL;
			else if (::strncmp(buffer, "[Log]", 5U) == 0)
				section = SECTION::LOG;
			else if (::strncmp(buffer, "[Modem]", 7U) == 0)
				section = SECTION::MODEM;
			else if (::strncmp(buffer, "[MMDVM]", 7U) == 0)
				section = SECTION::MMDVM;
			else if (::strncmp(buffer, "[SVXLink]", 9U) == 0)
				section = SECTION::SVXLink;
			else
				section = SECTION::NONE;

			continue;
		}

		char* key = ::strtok(buffer, " \t=\r\n");
		if (key == nullptr)
			continue;

		char* value = ::strtok(nullptr, "\r\n");
		if (value == nullptr)
			continue;

		// Remove quotes from the value
		size_t len = ::strlen(value);
		if (len > 1U && *value == '"' && value[len - 1U] == '"') {
			value[len - 1U] = '\0';
			value++;
		} else {
			char *p;

			// if value is not quoted, remove after # (to make comment)
			if ((p = strchr(value, '#')) != nullptr)
				*p = '\0';

			// remove trailing tab/space
			for (p = value + strlen(value) - 1U; p >= value && (*p == '\t' || *p == ' '); p--)
				*p = '\0';
		}

		if (section == SECTION::GENERAL) {
			if (::strcmp(key, "Daemon") == 0)
				m_daemon = ::atoi(value) == 1;
			else if (::strcmp(key, "Channels") == 0)
				m_numChannels = (unsigned int)::atoi(value);
			else if (::strcmp(key, "DMRDelay") == 0)
				m_sampleDelay = ::atoi(value);
			else if (::strcmp(key, "RFDelay") == 0)
				m_RFDelay = (unsigned int)::atoi(value);
			else if (::strcmp(key, "RSSICalibration") == 0)
				m_RSSICalibration = (unsigned int)::atoi(value);
		} else if (section == SECTION::LOG) {
			if (::strcmp(key, "DisplayLevel") == 0)
				m_logDisplayLevel = (unsigned int)::atoi(value);
		} else if (section == SECTION::MODEM) {
			if (::strcmp(key, "Trace") == 0)
				m_modemTrace = ::atoi(value) == 1;
			else if (::strcmp(key, "SampleRate") == 0)
				m_sampleRate = (unsigned int)::atoi(value);
			else if (::strcmp(key, "DigitalGain") == 0)
				m_digitalGain = (unsigned int)::atoi(value);
			else if (::strcmp(key, "SymbolDeviation") == 0)
				m_symbolDeviation = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Type") == 0)
				m_modemType = value;
			else if (::strcmp(key, "URI") == 0)
				m_modemURI = value;
			else if (::strcmp(key, "RxGain") == 0)
				m_rxGain = (unsigned int)::atoi(value);
			else if (::strcmp(key, "TxGain") == 0)
				m_txGain = (unsigned int)::atoi(value);
			else if (::strcmp(key, "RxFrequency") == 0)
				m_rxFreq = (unsigned int)::atoi(value);
			else if (::strcmp(key, "TxFrequency") == 0)
				m_txFreq = (unsigned int)::atoi(value);
			else if (::strcmp(key, "RxAntenna") == 0)
				m_rxAntenna = value;
			else if (::strcmp(key, "TxAntenna") == 0)
				m_txAntenna = value;
		} else if (section == SECTION::MMDVM) {
			if (::strcmp(key, "ModemAddress") == 0)
				m_networkModemAddress = value;
			else if (::strcmp(key, "ModemPort") == 0)
				m_networkModemPort = (unsigned short)::atoi(value);
			else if (::strcmp(key, "LocalAddress") == 0)
				m_networkLocalAddress = value;
			else if (::strcmp(key, "LocalPort") == 0)
				m_networkLocalPort = (unsigned short)::atoi(value);
			else if (::strcmp(key, "Trace") == 0)
				m_networkTrace = ::atoi(value) == 1;
		} else if (section == SECTION::SVXLink) {
			if (::strcmp(key, "ModemAddress") == 0)
				m_svxBridgeModemAddress = value;
			else if (::strcmp(key, "ModemPort") == 0)
				m_svxBridgeModemPort = (unsigned short)::atoi(value);
			else if (::strcmp(key, "LocalAddress") == 0)
				m_svxBridgeLocalAddress = value;
			else if (::strcmp(key, "LocalPort") == 0)
				m_svxBridgeLocalPort = (unsigned short)::atoi(value);
			else if (::strcmp(key, "SVXAddress") == 0)
				m_svxBridgeSVXAddress = value;
			else if (::strcmp(key, "SVXPort") == 0)
				m_svxBridgeSVXPort = (unsigned short)::atoi(value);
			else if (::strcmp(key, "LocalSVXAddress") == 0)
				m_svxBridgeLocalSVXAddress = value;
			else if (::strcmp(key, "LocalSVXPort") == 0)
				m_svxBridgeLocalSVXPort = (unsigned short)::atoi(value);
			else if (::strcmp(key, "RxGain") == 0)
				m_svxBridgeRxGain = (unsigned int)::atoi(value);
			else if (::strcmp(key, "TxGain") == 0)
				m_svxBridgeTxGain = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Trace") == 0)
				m_networkTrace = ::atoi(value) == 1;
		}
	}

	::fclose(fp);

	return true;
}

bool CConf::getDaemon() const
{
	return m_daemon;
}

unsigned int CConf::getNumChannels() const
{
	return m_numChannels;
}

int CConf::getSampleDelay() const
{
	return m_sampleDelay;
}

unsigned int CConf::getRFDelay() const
{
	return m_RFDelay;
}

unsigned int CConf::getRSSICalibration() const
{
	return m_RSSICalibration;
}


unsigned int CConf::getSampleRate() const
{
	return m_sampleRate;
}

unsigned int CConf::getDigitalGain() const
{
	return m_digitalGain;
}

unsigned int CConf::getSymbolDeviation() const
{
	return m_symbolDeviation;
}

unsigned int CConf::getLogDisplayLevel() const
{
	return m_logDisplayLevel;
}

std::string CConf::getModemType() const
{
	return m_modemType;
}

std::string CConf::getModemURI() const
{
	return m_modemURI;
}

unsigned int CConf::getRxGain() const
{
	return m_rxGain;
}

unsigned int CConf::getTxGain() const
{
	return m_txGain;
}

unsigned int CConf::getRxFreq() const
{
	return m_rxFreq;
}

unsigned int CConf::getTxFreq() const
{
	return m_txFreq;
}

std::string CConf::getRxAntenna() const
{
	return m_rxAntenna;
}

std::string CConf::getTxAntenna() const
{
	return m_txAntenna;
}

bool CConf::getModemTrace() const
{
	return m_modemTrace;
}

std::string CConf::getNetworkModemAddress() const
{
	return m_networkModemAddress;
}

unsigned short CConf::getNetworkModemPort() const
{
	return m_networkModemPort;
}

std::string CConf::getNetworkLocalAddress() const
{
	return m_networkLocalAddress;
}

unsigned short CConf::getNetworkLocalPort() const
{
	return m_networkLocalPort;
}

bool CConf::getNetworkTrace() const
{
	return m_networkTrace;
}

std::string CConf::getBridgeModemAddress() const
{
	return m_svxBridgeModemAddress;
}

unsigned short CConf::getBridgeModemPort() const
{
	return m_svxBridgeModemPort;
}

std::string CConf::getBridgeLocalAddress() const
{
	return m_svxBridgeLocalAddress;
}

unsigned short CConf::getBridgeLocalPort() const
{
	return m_svxBridgeLocalPort;
}

std::string CConf::getBridgeSVXAddress() const
{
	return m_svxBridgeSVXAddress;
}

unsigned short CConf::getBridgeSVXPort() const
{
	return m_svxBridgeSVXPort;
}

std::string CConf::getBridgeLocalSVXAddress() const
{
	return m_svxBridgeLocalSVXAddress;
}

unsigned short CConf::getBridgeLocalSVXPort() const
{
	return m_svxBridgeLocalSVXPort;
}

unsigned int CConf::getBridgeRxGain() const
{
	return m_svxBridgeRxGain;
}

unsigned int CConf::getBridgeTxGain() const
{
	return m_svxBridgeTxGain;
}

