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

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <thread>
#include <chrono>
#include <string>
#include <csignal>
#include <cmath>
#include "Constants.h"
#include "Network.h"
#include "Device.h"
#include "DMRTiming.h"
#include "Receiver.h"
#include "Transmitter.h"
#include "FMMod.h"
#include "Resampler.h"
#include "Rotator.h"
#include "Channelizer.h"
#include "Conf.h"

const char* DEFAULT_INI_FILE = "/etc/MMDVM-Multi.ini";
bool running = false;

void signal_handler(int signal)
{
    switch (signal) {
        case 2:
            ::fprintf(stdout, "MMDVM-Multi exited on receipt of SIGINT\n");
            break;
        case 15:
            ::fprintf(stdout, "MMDVM-Multi exited on receipt of SIGTERM\n");
            break;
        case 1:
            ::fprintf(stdout, "MMDVM-Multi exited on receipt of SIGHUP\n");
            break;
        case 10:
            ::fprintf(stdout, "MMDVM-Multi is restarting on receipt of SIGUSR1\n");
            break;
        default:
            ::fprintf(stdout, "MMDVM-Multi exited on receipt of an unknown signal\n");
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
                ::fprintf(stdout, "MMDVM-Multi version unknown\n");
                return 0;
            } else if (arg.substr(0, 1) == "-") {
                ::fprintf(stderr, "Usage: MMDVM-Multi [filename]\n");
                return 1;
            } else {
                iniFile = argv[currentArg];
            }
        }
    }
    CConf conf(iniFile);
    bool ret = conf.read();
    if (!ret) {
        ::fprintf(stderr, "MMDVM-Multi: cannot read the .ini file\n");
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

    bool debug = conf.getModemTrace();
    unsigned int active_channels = std::min<unsigned int>(conf.getNumChannels(), MAX_MMDVM_CHANNELS);
    active_channels = std::max<unsigned int>(active_channels, 1U);
    int sample_delay = conf.getSampleDelay(); // can be negative
    unsigned int rf_delay = conf.getRFDelay();
    unsigned int sample_rate = std::min<unsigned int>(conf.getSampleRate(), MAX_SAMPLE_RATE);
    unsigned int digital_gain = std::max<unsigned int>(conf.getDigitalGain(), 1U);
    float dac_scaling = std::min<float>(float(digital_gain) / 100.0f, MAX_TX_DAC_SCALE);
    float symbol_deviation = float(std::max<unsigned int>(conf.getSymbolDeviation(), 1U));
    unsigned int interpolation = 25U;
    unsigned int decimation = 24U;
    unsigned int channel_spacing = 25000U;
    float baseband_shift = 12500.0f;
    float fractional_bandwidth = 0.4f;
    unsigned int power_calibration = conf.getRSSICalibration();
    if((sample_rate % channel_spacing) != 0U)
    {
        ::fprintf(stderr, "MMDVM-Multi: Sample Rate must be a multiple of channel space\n");
        return 1;
    }
    unsigned int num_pfb_channels = sample_rate / channel_spacing;
    if(num_pfb_channels > MAX_PFB_CHANNELS)
    {
        ::fprintf(stderr, "MMDVM-Multi: The sample rate %d is not supported\n", sample_rate);
        return 1;
    }
    if(active_channels >= num_pfb_channels)
    {
        ::fprintf(stderr, "MMDVM-Multi: The number of channels must be lower than %d for this sample rate\n", num_pfb_channels);
        return 1;
    }

    std::string modemAddress = conf.getNetworkModemAddress();
    unsigned short modemPort = conf.getNetworkModemPort();
    std::string localAddress = conf.getNetworkLocalAddress();
    unsigned short localPort = conf.getNetworkLocalPort();
    Network* network = new Network(modemAddress, modemPort, localAddress, localPort, active_channels);
    if(!network->open())
    {
        ::fprintf(stderr, "MMDVM-Multi: Unable to open network connections\n");
        return 1;
    }

    float rx_gain = float(conf.getRxGain());
    float tx_gain = float(conf.getTxGain());
    float rx_freq = float(conf.getRxFreq()) - baseband_shift;
    float tx_freq = float(conf.getTxFreq()) - baseband_shift;
    std::string rx_antenna = conf.getRxAntenna();
    std::string tx_antenna = conf.getTxAntenna();
    std::string deviceType = conf.getModemType();
    std::string modemURI = conf.getModemURI();
    bool needs_timestamp = true;
    if (deviceType.compare("plutosdr") == 0 || deviceType.compare("pluto") == 0) {
        needs_timestamp = false;
    }

    Device* device = new Device(deviceType, modemURI, double(sample_rate), rx_freq, tx_freq,
                                rx_gain, tx_gain, rx_antenna, tx_antenna, num_pfb_channels, debug);
    if(!device->getSoapyInit() || (device->getRxStream() == nullptr) || (device->getTxStream() == nullptr))
    {
        network->close();
        return 1;
    }
    Rotator* rotator = new Rotator(baseband_shift, float(sample_rate));
    Channelizer* channelizer = new Channelizer(num_pfb_channels);
    Resampler* resampler = new Resampler(interpolation, decimation, fractional_bandwidth, active_channels);
    FMMod* fm_mod = new FMMod(0.5, active_channels);
    DMRTiming* timing = new DMRTiming(rf_delay, sample_delay);
    Receiver* rx = new Receiver(network, device, fm_mod, resampler, rotator, channelizer,
                                timing, active_channels, num_pfb_channels, power_calibration, symbol_deviation, needs_timestamp);
    Transmitter* tx = new Transmitter(network, device, fm_mod, resampler, rotator, channelizer,
                                      timing, active_channels, num_pfb_channels, needs_timestamp, dac_scaling, symbol_deviation);

    rx->start();
    tx->start();
    running = true;

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    std::signal(SIGHUP, signal_handler);
    std::signal(SIGUSR1, signal_handler);

    while(running)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    rx->stop();
    tx->stop();

    while(!tx->stopped() || !rx->stopped())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    network->close();

    delete tx;
    delete rx;
    delete timing;
    delete fm_mod;
    delete resampler;
    delete channelizer;
    delete rotator;
    delete device;
    delete network;

    return 0;
}
