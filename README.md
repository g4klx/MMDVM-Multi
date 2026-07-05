MMDVM multi-carrier modem
====

This is the source code of the MMDVM-Multi program that can transmit multiple RF channels at once using the D-Star, DMR, System Fusion, P25, NXDN, POCSAG, and FM modes. This program is also used to transmit multiple DMR Tier 3 trunking channels on the same SDR device.

It can be used with the Lime SDR, the Pluto SDR, Ettus USRP, LibreSDR and clones that advertise themselves as compatible with the USRP, or the SXceiver Pi hat or similar running on a Raspberry Pi. All of the development work is done on Linux with a LimeSDR-mini. It uses the SoapySDR interface drivers that can be installed from the distribution repositories, and for the SXceiver, found at https://sxceiver.com/

For the program to run, the same number of MMDVM-IQ programs as configured in the .ini file must have been started already.

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


The SVXBridge program can be used to bridge SVXLink and MMDVM-Multi directly. SVXLink must be configured with a card sample rate of 48000 and the [Rx1] and [Tx1] must use UDP as audio device.



Requirements
====

- cmake
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
$ git clone https://codeberg.org/qradiolink/MMDVM-IQ
$ cd MMDVM-IQ/
$ git checkout multi-mmdvm
$ make -j4
```

```
$ git clone https://github.com/g4klx/MMDVM-Host
$ cd MMDVM-Host/
$ git checkout dmr_trunking
$ make -j4
```


Running
====


MMDVM-Multi
----


```
$ ./MMDVM-Multi ../MMDVM-Multi.ini
```


SVXBridge
----

Adjust RxGain between 500 and 2000 and TxGain between 1 and 20 in SVXBridge.ini for normal audio levels; these values also depend on the SymbolDeviation setting in MMDVM-Multi
Select which channel of MMDVM-Multi is used by setting ModemPort to the corresponding value
Adjust SVX ports as needed


```
$ ./SVXBridge ../SVXBridge.ini
```

SVXLink config values for [Rx1] and [Tx1]:

```
[Rx1]
TYPE=Local
RX_ID=
AUDIO_DEV=udp:127.0.0.1:4937
AUDIO_CHANNEL=0
AUDIO_DEV_KEEP_OPEN=0
SQL_DET=CTCSS
SQL_START_DELAY=20
SQL_DELAY=500
SQL_HANGTIME=20
SQL_TIMEOUT=600
VOX_FILTER_DEPTH=10
VOX_THRESH=10000
CTCSS_MODE=2
CTCSS_FQ=103.5
SIGLEV_DET=NOISE
SIGLEV_SLOPE=1
SIGLEV_OFFSET=0
SIGLEV_OPEN_THRESH=30
SIGLEV_CLOSE_THRESH=10
DEEMPHASIS=1

```

```
[Tx1]
TYPE=Local
AUDIO_DEV=udp:127.0.0.1:4938
AUDIO_CHANNEL=0
AUDIO_DEV_KEEP_OPEN=0
PTT_TYPE=NONE
TIMEOUT=1800
TX_DELAY=0
CTCSS_FQ=103.5
CTCSS_LEVEL=9
PREEMPHASIS=1
```


MMDVM
----

```
[MMDVM Multi]
Enabled=1
MultiModemAddress=127.0.0.1
MultiModemPort=48200
LocalAddress=127.0.0.1
LocalPort=48100
```

Use the shell script to start all MMDVM programs at the same time

```
$ cd MMDVM-IQ/
$ ./start.sh 7
```

or

```
$ ./MMDVM-IQ MMDVM-IQ-1.ini
$ ./MMDVM-IQ MMDVM-IQ-2.ini
$ ./MMDVM-IQ MMDVM-IQ-3.ini
...etc
```

MMDVMHost
----


Configure modem serial port in MMDVMHost.ini to use the UDP protocol:

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
$ ./MMDVM-Host MMDVMHost1.ini
$ ./MMDVM-Host MMDVMHost2.ini
$ ./MMDVM-Host MMDVMHost3.ini
...etc
```


License
====

This software is licenced under the GPL v2 and is primarily intended for amateur and educational use.
