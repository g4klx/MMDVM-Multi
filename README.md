MMDVM multi-carrier modem
====

This is the source code of the MMDVM-Multi program that can transmit multiple RF channels at once using the D-Star, DMR, System Fusion, P25, NXDN, POCSAG, and FM modes. This program is also used to transmit multiple DMR Tier 3 trunking channels on the same SDR device.

It can be used with the Lime SDR, the Pluto SDR, Ettus USRP, LibreSDR and clones that advertise themselves as compatible with the USRP, or the SXceiver Pi hat or similar running on a Raspberry Pi. All of the development work is done on Linux with a LimeSDR-mini. It uses the SoapySDR interface drivers that can be installed from the distribution repositories, and for the SXceiver, found at https://sxceiver.com/

The same number of MMDVM-IQ and MMDVM-Host programs as the number of channels configured in the .ini file must be started to serve each RF channel.

It connects to the MMDVM-IQ modems via UDP ports sequentially numbered starting from the base port.
The number of RF channels can be between 1 and 7. Any channel can be allocated to any mode.
The channel numbering in reference to the transmit/receive frequency for 7 channels is:

```
Channel 7    Channel 6      Channel 5     Channel 1        Channel 2     Channel 3     Channel 4
434.7500     433.7750       434.8000      434.8250         434.8500      434.8750      434.9000
   |             |            |             |               |             |              |
   |             |            |             |               |             |              |
   |             |            |             |               |             |              |
---|-------------|------------|-------------|---------------|-------------|--------------|---------
                                       RX/TX frequency

```



Requirements
====

- SoapySDR and needed device drivers: SoapyLMS7, SoapyPlutoSDR, SoapyUHD, SoapySx
- LiquidDSP

Installing build dependencies on Debian Trixie: 

```
$ sudo apt-get install cmake libliquid-dev libliquid1 libsoapysdr-dev libsoapysdr0.8
```

For the LimeSDR:

```
$ sudo apt-get install liblimesuite-dev  soapysdr-module-lms7 
```

For the Ettus USRP:

```
$ sudo apt-get install soapysdr-module-uhd uhd-soapysdr libuhd4.8.0 uhd-host
```

For the PlutoSDR: https://github.com/pothosware/SoapyPlutoSDR

The master branch must be cloned and built for the repo as the current packaged versions are not compatible with MMDVM-Multi.


Running
====


MMDVM-Multi
----


```
$ ./MMDVM-Multi ../MMDVM-Multi.ini
```


MMDVM-IQ
----

```
[MMDVM Multi]
Enabled=1
MultiModemAddress=127.0.0.1
MultiModemPort=48200
LocalAddress=127.0.0.1
LocalPort=48100
```

Create a config file for each MMDVM-IQ program instance, and change the ports to map to each RF channel in MMDVM-Multi, starting with the base port and incrementing the numbers in each config file. Change the HostPort and LocalPort in the section [MMDVM Host] to match each MMDVM-Host program instance.

Start multiple MMDVM-IQ programs with the separate config files.

```
$ ./MMDVM-IQ MMDVM-IQ-1.ini
$ ./MMDVM-IQ MMDVM-IQ-2.ini
$ ./MMDVM-IQ MMDVM-IQ-3.ini
...etc
```

MMDVM-Host
----


Configure modem protocol in MMDVM-Host.ini to use the UDP protocol, and change ModemPort and LocalPort to match each MMDVM-IQ instance.

```
[Modem]
Protocol=udp
ModemAddress=127.0.0.1
ModemPort=3334             # Change for each channel
LocalAddress=127.0.0.1
LocalPort=3335             # Change for each channel

TXInvert=1
RXInvert=1
RXLevel=83
TXLevel=83
```

```
$ cd MMDVM-Host/
$ ./MMDVM-Host MMDVM-Host-1.ini
$ ./MMDVM-Host MMDVM-Host-2.ini
$ ./MMDVM-Host MMDVM-Host-3.ini
...etc
```


License
====

This software is licenced under the GPL v2 and is primarily intended for amateur and educational use.
