MMDVM multi-carrier modem
====

This is the source code of the MMDVM-Multi program that can transmit multiple RF channels at once using the D-Star, DMR, System Fusion, P25, NXDN, POCSAG, and FM modes. This program is also used to transmit multiple DMR Tier 3 trunking channels on the same SDR device.

It can be used with the Lime SDR, the Pluto SDR, or the SXceiver Pi hat or similar running on a Raspberry Pi. All of the development work is done on Linux with a LimeSDR-mini. It uses the SoapySDR interface drivers that can be found at https://sxceiver.com/

For the program to run, the same number of MMDVM programs as configured in the .ini file must have been started already.

It connects to the MMDVM modems via local ZeroMQ pipes. The channel numbering for 7 channels is:

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

- cmake
- ZeroMQ
- SoapySDR and needed device drivers: SoapyLMS7, SoapyPlutoSDR, SoapySx
- LiquidDSP

Installing build dependencies on Debian Trixie: 

```
$ sudo apt-get install cmake cppzmq-dev libzmq3-dev libzmq5 libliquid-dev libliquid1 liblimesuite-dev libsoapysdr-dev libsoapysdr0.8 soapysdr-module-lms7
```


Building the software
====

```
$ git clone https://codeberg.org/qradiolink/MMDVM-Multi
$ cd MMDVM-Multi/
$ git checkout master
$ mkdir -p build
$ cd build/
$ cmake ..
$ make -j4
```

```
$ git clone https://codeberg.org/qradiolink/MMDVM-SDR
$ cd MMDVM-SDR/
$ git checkout merge_trunking_master
$ mkdir -p build
$ cd build/
$ cmake ..
$ make -j4
```

```
$ git clone https://codeberg.org/qradiolink/MMDVMHost-SDR
$ cd MMDVM-SDR/
$ git checkout merge_trunking_master2
$ mkdir -p build
$ cd build/
$ cmake ..
$ make -j4
```


Running
====


MMDVM-Multi
----

```
$ ./MMDVM-Multi ../MMDVM-Multi.ini
```

MMDVM
----


Configure modem serial port to use virtual PTY:

```
[Modem]
UARTPort=/home/pi/MMDVM-SDR/build/ttyMMDVM1
TXInvert=1
RXInvert=1
RXLevel=83
TXLevel=83
```

Use the shell script to start all MMDVM programs at the same time

```
$ cd MMDVM-SDR/
$ ./start.sh 7

M: 2025-02-18 09:09:52.591 virtual pts: /dev/pts/24 <> /home/pi/MMDVM-SDR/build/ttyMMDVM1
M: 2025-02-18 09:09:52.591 virtual pts: /dev/pts/25 <> /home/pi/MMDVM-SDR/build/ttyMMDVM4
M: 2025-02-18 09:09:52.592 virtual pts: /dev/pts/26 <> /home/pi/MMDVM-SDR/build/ttyMMDVM2
M: 2025-02-18 09:09:52.592 virtual pts: /dev/pts/27 <> /home/pi/MMDVM-SDR/build/ttyMMDVM3
M: 2025-02-18 09:09:52.592 virtual pts: /dev/pts/27 <> /home/pi/MMDVM-SDR/build/ttyMMDVM5
M: 2025-02-18 09:09:52.593 virtual pts: /dev/pts/28 <> /home/pi/MMDVM-SDR/build/ttyMMDVM6
M: 2025-02-18 09:09:52.593 virtual pts: /dev/pts/28 <> /home/pi/MMDVM-SDR/build/ttyMMDVM7
```

or

```
$ ./mmdvm -c 1
$ ./mmdvm -c 2
$ ./mmdvm -c 3
...
```

MMDVMHost
----


```
$ cd MMDVMHost-SDR/
$ ./MMDVMHost MMDVM1.ini
$ ./MMDVMHost MMDVM2.ini
$ ./MMDVMHost MMDVM3.ini
$ ./MMDVMHost MMDVM4.ini
$ ./MMDVMHost MMDVM5.ini
$ ./MMDVMHost MMDVM6.ini
```

License
====

This software is licenced under the GPL v2 and is primarily intended for amateur and educational use.
