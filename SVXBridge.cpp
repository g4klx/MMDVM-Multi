/*
 *   Copyright (C) 2026 by Adrian Musceac YO8RZZ
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

#include "SVXBridge.h"
#include <assert.h>

const unsigned int BUFFER_LENGTH = 8000U;
const unsigned int NET_TIMEOUT_FRAMES = 25U;
const char* DEFAULT_INI_FILE = "/etc/SVXBridge.ini";
bool running = false;

void signal_handler(int signal)
{
  switch (signal) {
    case 2:
      ::fprintf(stdout, "SVXBridge exited on receipt of SIGINT\n");
      break;
    case 15:
      ::fprintf(stdout, "SVXBridge exited on receipt of SIGTERM\n");
      break;
    case 1:
      ::fprintf(stdout, "SVXBridge exited on receipt of SIGHUP\n");
      break;
    case 10:
      ::fprintf(stdout, "SVXBridge is restarting on receipt of SIGUSR1\n");
      break;
    default:
      ::fprintf(stdout, "SVXBridge exited on receipt of an unknown signal\n");
      break;
  }
  running = false;
}

int main(int argc, char** argv)
{
  const char* iniFile = DEFAULT_INI_FILE;
  if (argc > 1) {
    for (int currentArg = 1; currentArg < argc; ++currentArg) {
      std::string arg = argv[currentArg];
      if ((arg == "-v") || (arg == "--version")) {
        ::fprintf(stdout, "SVXBridge version unknown\n");
        return 0;
      } else if (arg.substr(0, 1) == "-") {
        ::fprintf(stderr, "Usage: SVXBridge [filename]\n");
        return 1;
      } else {
        iniFile = argv[currentArg];
      }
    }
  }
  CConf conf(iniFile);
  bool ret = conf.read();
  if (!ret) {
    ::fprintf(stderr, "SVXBridge: cannot read the .ini file\n");
    return 1;
  }
  
  bool m_daemon = conf.getDaemon();
  if (m_daemon) {
    // Create new process
    pid_t pid = ::fork();
    if (pid == -1) {
      ::fprintf(stderr, "Couldn't fork() , exiting\n");
      return -1;
    }
    else if (pid != 0) {
      exit(EXIT_SUCCESS);
    }
    
    // Create new session and process group
    if (::setsid() == -1) {
      ::fprintf(stderr, "Couldn't setsid(), exiting\n");
      return -1;
    }
    
    // Set the working directory to the root directory
    if (::chdir("/") == -1) {
      ::fprintf(stderr, "Couldn't cd /, exiting\n");
      return -1;
    }
    
    // If we are currently root...
    if (getuid() == 0) {
      struct passwd* user = ::getpwnam("mmdvm");
      if (user == nullptr) {
        ::fprintf(stderr, "Could not get the mmdvm user, exiting\n");
        return -1;
      }
      
      uid_t mmdvm_uid = user->pw_uid;
      gid_t mmdvm_gid = user->pw_gid;
      
      // Set user and group ID's to mmdvm:mmdvm
      if (::setgid(mmdvm_gid) != 0) {
        ::fprintf(stderr, "Could not set mmdvm GID, exiting\n");
        return -1;
      }
      
      if (::setuid(mmdvm_uid) != 0) {
        ::fprintf(stderr, "Could not set mmdvm UID, exiting\n");
        return -1;
      }
      
      // Double check it worked (AKA Paranoia)
      if (::setuid(0) != -1) {
        ::fprintf(stderr, "It's possible to regain root - something is wrong!, exiting\n");
        return -1;
      }
    }
  }
  
  std::string modemAddress = conf.getBridgeModemAddress();
  unsigned int modemPort = conf.getBridgeModemPort();
  std::string localModemAddress = conf.getBridgeLocalAddress();
  unsigned int localModemPort = conf.getBridgeLocalPort();
  
  std::string svxAddress = conf.getBridgeSVXAddress();
  unsigned int svxPort = conf.getBridgeSVXPort();
  std::string localSVXAddress = conf.getBridgeLocalSVXAddress();
  unsigned int localSVXPort = conf.getBridgeLocalSVXPort();
  
  ::fprintf(stdout, "MMDVM-Multi Network Parameters\n");
  ::fprintf(stdout, "    Modem Address: %s\n", modemAddress.c_str());
  ::fprintf(stdout, "    Modem Port: %hu\n", modemPort);
  ::fprintf(stdout, "    Local Address: %s\n", localModemAddress.c_str());
  ::fprintf(stdout, "    Local Port: %hu\n", localModemPort);
  
  ::fprintf(stdout, "SVXLink Network Parameters\n");
  ::fprintf(stdout, "    SVXLink Address: %s\n", svxAddress.c_str());
  ::fprintf(stdout, "    SVXLink Port: %hu\n", svxPort);
  ::fprintf(stdout, "    Local Address: %s\n", localSVXAddress.c_str());
  ::fprintf(stdout, "    Local Port: %hu\n", localSVXPort);
  
  unsigned int interp = 2U; // 48k svxlink
  unsigned int decim = 1U;
  unsigned int rxGain = conf.getBridgeRxGain();
  unsigned int txGain = conf.getBridgeTxGain();
  
  SVXBridge* bridge = new SVXBridge(modemAddress, modemPort, localModemAddress, localModemPort,
                                    svxAddress, svxPort, localSVXAddress, localSVXPort, interp, decim, rxGain, txGain);
  if(!bridge->open())
  {
    ::fprintf(stderr, "SVXBridge: Unable to open network connections\n");
    delete bridge;
    return 1;
  }
  
  std::signal(SIGINT, signal_handler);
  std::signal(SIGTERM, signal_handler);
  std::signal(SIGHUP, signal_handler);
  std::signal(SIGUSR1, signal_handler);

  ::fprintf(stdout, "SVXBridge started\n");
  running = true;
  while(running)
  {
    bridge->runBridge();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  bridge->close();
  delete bridge;
  return 0;
}

SVXBridge::SVXBridge(const std::string& modemAddress, unsigned int modemPort,
                     const std::string& localModemAddress, unsigned int localModemPort,
                     const std::string& svxAddress, unsigned int svxPort,
                     const std::string& localSVXAddress, unsigned int localSVXPort,
                     unsigned int interp, unsigned int decim, unsigned int rxGain, unsigned int txGain) :
m_init(false),
m_addrLenSVX(0U),
m_addrLenModem(0U),
m_decim(decim),
m_interp(interp),
m_rxGain(rxGain),
m_txGain(txGain),
m_netTimeout(NET_TIMEOUT_FRAMES)
{
  m_inputBuffer.reserve(SAMPLES_PER_SLOT * 2U);
  m_upsampler = rresamp_rrrf_create_kaiser(interp, decim, RESAMPLER_FILTER_DELAY, 0.16f, 60.0f);
  m_downsampler = rresamp_rrrf_create_kaiser(decim, interp, RESAMPLER_FILTER_DELAY, 0.16f, 60.0f);

  bool svxInit = false;
  bool modemInit = false;

  if (CUDPSocket::lookup(svxAddress, svxPort, m_addrSVX, m_addrLenSVX) == 0)
    svxInit = true;

  m_sockSVX = CUDPSocket(localSVXAddress, localSVXPort);

  if (CUDPSocket::lookup(modemAddress, modemPort, m_addrModem, m_addrLenModem) == 0)
    modemInit = true;

  m_sockModem = CUDPSocket(localModemAddress, localModemPort);

  m_init = svxInit && modemInit;
}

SVXBridge::~SVXBridge()
{
  rresamp_rrrf_destroy(m_upsampler);
  rresamp_rrrf_destroy(m_downsampler);
}

void SVXBridge::runBridge()
{
  processModem();
  processSVXLink();
}

void SVXBridge::processSVXLink()
{
  unsigned char svxbuf[BUFFER_LENGTH];
  int n = readSVX(svxbuf, BUFFER_LENGTH);
  if(n < 2)
    return;

  unsigned int len = (unsigned int)n / sizeof(int16_t);
  int16_t* svxSamples = (int16_t*)svxbuf;
  for(unsigned int i=0; i < len; i++)
  {
    m_inputBuffer.push_back(svxSamples[i]);
  }
  if(m_inputBuffer.size() > SAMPLES_PER_SLOT * 8U)
  {
    m_inputBuffer.erase(m_inputBuffer.begin(), m_inputBuffer.begin() + SAMPLES_PER_SLOT * 2U);
  }
}

void SVXBridge::processModem()
{
  uint32_t num_items = SAMPLES_PER_SLOT;
  unsigned char recv_message[NETWORK_TX_PACKET_SIZE];
  int n = readModem(recv_message, NETWORK_TX_PACKET_SIZE);

  if((unsigned int)n != NETWORK_TX_PACKET_SIZE)
    return;

  if(m_inputBuffer.size() >= SAMPLES_PER_SLOT * 2U)
  {
    m_netTimeout = NET_TIMEOUT_FRAMES;
    float in_samples[SAMPLES_PER_SLOT * 2U];
    ::memset(in_samples, 0U, SAMPLES_PER_SLOT * 2U * sizeof(float));
    for(unsigned int i=0;i < SAMPLES_PER_SLOT * 2U;i++)
    {
      in_samples[i] = float(m_inputBuffer.at(i)) / 32767.0f;
    }

    float out_samples[SAMPLES_PER_SLOT];
    ::memset(out_samples, 0U, SAMPLES_PER_SLOT * sizeof(float));
    downsample(in_samples, SAMPLES_PER_SLOT * 2U, out_samples);
    m_inputBuffer.erase(m_inputBuffer.begin(), m_inputBuffer.begin() + SAMPLES_PER_SLOT * 2U);

    int16_t modem_samples[SAMPLES_PER_SLOT];
    ::memset(modem_samples, 0U, SAMPLES_PER_SLOT * sizeof(int16_t));
    for(unsigned int i=0; i< SAMPLES_PER_SLOT; i++)
    {
      int32_t s = int32_t(out_samples[i] * 32767.0f * (float(m_txGain) / 100.0f));
      s = (s < -32767) ? -32767 : s;
      s = (s > 32767) ? 32767 : s;
      modem_samples[i] = int16_t(s);
    }

    unsigned char reply[NETWORK_RX_PACKET_SIZE];
    ::memset(reply, 0U, NETWORK_RX_PACKET_SIZE * sizeof(unsigned char));
    ::memcpy(reply, &num_items, sizeof(uint32_t));
    ::memcpy(reply + sizeof(uint32_t) + num_items * sizeof(uint8_t),
              (unsigned char *)modem_samples, num_items*sizeof(int16_t));
    writeModem((unsigned char*)reply, NETWORK_RX_PACKET_SIZE);
  }
  else if(m_netTimeout > 0U)
  {
    unsigned char reply[NETWORK_RX_PACKET_SIZE];
    ::memset(reply, 0U, NETWORK_RX_PACKET_SIZE * sizeof(unsigned char));
    ::memcpy(reply, &num_items, sizeof(uint32_t));
    writeModem((unsigned char*)reply, NETWORK_RX_PACKET_SIZE);
    m_netTimeout--;
  }

  uint32_t data_size = 0;
  ::memcpy(&data_size, recv_message, sizeof(uint32_t));
  if(data_size != SAMPLES_PER_SLOT)
  {
    ::fprintf(stderr, "Received malformed packet size from MMDVM-Multi: %u\n", data_size);
    return;
  }
  else
  {
    int16_t modem_samples[SAMPLES_PER_SLOT];
    ::memset(modem_samples, 0U, SAMPLES_PER_SLOT * sizeof(int16_t));
    ::memcpy(&modem_samples, recv_message + 2U * sizeof(uint32_t) + data_size * sizeof(uint8_t), data_size * sizeof(int16_t));
    float in_samples[SAMPLES_PER_SLOT];
    ::memset(in_samples, 0U, SAMPLES_PER_SLOT * sizeof(float));
    for(unsigned int i=0;i < SAMPLES_PER_SLOT;i++)
    {
      in_samples[i] = float(modem_samples[i]) / 32767.0f;
    }
    float out_samples[SAMPLES_PER_SLOT * 2U];
    ::memset(out_samples, 0U, SAMPLES_PER_SLOT * 2U * sizeof(float));
    upsample(in_samples, SAMPLES_PER_SLOT, out_samples);

    int16_t svx_samples[SAMPLES_PER_SLOT * 2U];
    ::memset(svx_samples, 0U, SAMPLES_PER_SLOT * 2U * sizeof(int16_t));
    for(unsigned int i=0; i< SAMPLES_PER_SLOT * 2U; i++)
    {
      int32_t s = int32_t(out_samples[i] * 32767.0f * (float(m_rxGain) / 100.0f));
      s = (s < -32767) ? -32767 : s;
      s = (s > 32767) ? 32767 : s;
      svx_samples[i] = int16_t(s);
    }
    writeSVX((unsigned char*)&svx_samples, SAMPLES_PER_SLOT * 2U * sizeof(int16_t));
  }
}

bool SVXBridge::open()
{
  if (!m_init) {
    ::fprintf(stderr, "Unable to resolve the address of the modem\n");
    return false;
  }

  if(!m_sockSVX.open(m_addrSVX))
  {
    return false;
  }
  if(!m_sockModem.open(m_addrModem))
  {
    return false;
  }
  return true;
}

int SVXBridge::readSVX(unsigned char* buffer, unsigned int length)
{
  assert(buffer != nullptr);
  assert(length > 0U);
  
  unsigned char data[BUFFER_LENGTH];
  sockaddr_storage addr;
  unsigned int addrLen;
  int ret = m_sockSVX.read(data, length, addr, addrLen);
  
  // An error occurred on the socket
  if (ret < 0)
    return ret;

  if (ret > 0) {
    length = (unsigned int) ret;
    //if (CUDPSocket::match(addr, m_addrSVX)) { // FIXME: svxlink bind addr
      ::memcpy(buffer, data, length * sizeof(unsigned char));
      return ret;
    //}
  }
  return 0;
}

int SVXBridge::writeSVX(const unsigned char* buffer, unsigned int length)
{
  assert(buffer != nullptr);
  assert(length > 0U);
  int ret = m_sockSVX.write(buffer, length, m_addrSVX, m_addrLenSVX) ? int(length) : -1;
  return ret;
}

int SVXBridge::readModem(unsigned char* buffer, unsigned int length)
{
  assert(buffer != nullptr);
  assert(length > 0U);
  
  unsigned char data[BUFFER_LENGTH];
  sockaddr_storage addr;
  unsigned int addrLen;
  int ret = m_sockModem.read(data, length, addr, addrLen);
  
  // An error occurred on the socket
  if (ret < 0)
    return ret;

  if (ret > 0) {
    length = (unsigned int) ret;
    if (CUDPSocket::match(addr, m_addrModem)) {
      ::memcpy(buffer, data, length * sizeof(unsigned char));
      return ret;
    }
  }
  return 0;
}

int SVXBridge::writeModem(const unsigned char* buffer, unsigned int length)
{
  assert(buffer != nullptr);
  assert(length > 0U);
  int ret = m_sockModem.write(buffer, length, m_addrModem, m_addrLenModem) ? int(length) : -1;
  return ret;
}

void SVXBridge::close()
{
  if (!m_init) {
    return;
  }
  m_sockSVX.close();
  m_sockModem.close();
}

void SVXBridge::upsample(float* in_samples, unsigned int num_samples, float* out_samples)
{
  unsigned int p_in = num_samples / m_decim;
  float* in_buf = new float[m_decim];
  float* out_buf = new float[m_interp];
  for(unsigned int i=0;i<p_in;i++)
  {
    ::memcpy(in_buf, in_samples + (i * m_decim), m_decim * sizeof(float));
    rresamp_rrrf_execute(m_upsampler, in_buf, out_buf);
    ::memcpy(out_samples + (i * m_interp), out_buf, m_interp * sizeof(float));
  }
  delete[] in_buf;
  delete[] out_buf;
}

void SVXBridge::downsample(float* in_samples, unsigned int num_samples, float* out_samples)
{
  // interpolation and decimation are reversed when downsampling
  unsigned int p_in = num_samples / m_interp;
  float* in_buf = new float[m_interp];
  float* out_buf = new float[m_decim];
  for(unsigned int i=0;i<p_in;i++)
  {
    ::memcpy(in_buf, in_samples + (i * m_interp), m_interp * sizeof(float));
    rresamp_rrrf_execute(m_downsampler, in_buf, out_buf);
    ::memcpy(out_samples + (i * m_decim), out_buf, m_decim * sizeof(float));
  }
  delete[] in_buf;
  delete[] out_buf;
}
