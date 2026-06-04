MMDVM multi-carrier modem
====

This is the source code of the MMDVM-Multi program that can transmit multiple RF channels at once using the D-Star, DMR, System Fusion, P25, NXDN, POCSAG, and FM modes. This program is also used to transmit multiple DMR Tier 3 trunking channels on the same SDR device.

It can be used with the Lime SDR, the Pluto SDR, or the SXceiver Pi hat or similar running on a Raspberry Pi. All of the development work is done on Linux with a LimeSDR-mini. It uses the SoapySDR interface drivers that can be found at https://sxceiver.com/

It connects to the MMDVM modems via local ZeroMQ pipes. The corresponding entries at the host end are:


Requirements
====

- cmake
- ZeroMQ
- SoapySDR and needed device drivers: SoapyLMS7, SoapyPlutoSDR, SoapySx
- LiquidDSP

Installing build dependencies on Debian Trixie: 

<pre>
$ sudo apt-get install cmake cppzmq-dev libzmq3-dev libzmq5 libliquid-dev libliquid1 liblimesuite-dev libsoapysdr-dev libsoapysdr0.8 soapysdr-module-lms7
</pre>


Building the software
-----

<pre>
$ git clone https://codeberg.org/qradiolink/MMDVM-Multi
$ cd MMDVM-Multi/
$ git checkout master
$ mkdir -p build
$ cd build/
$ cmake ..
$ make -j4
</pre>

This software is licenced under the GPL v2 and is primarily intended for amateur and educational use.
