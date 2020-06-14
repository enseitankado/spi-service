# spi-service
Listens for SHM (Shared Memory) changes to send SHM data to the SPI port and from there to the ss963 driver card.
This is full configurable SPI listener. If you write to bytes SPI port of RPi at high speed, write it to SHM (shared memory) with any programming language. The SPI service will detect changes and will read modified memory and write it to SPI.0 port.

# Configuration
Configuration options stored in /etc/spi.service.conf as described below:

PORT_COUNT = This is bit count to write SPI port at the same time. Default 96.
LATCH_PIN = Optional. If your SPI hardware has latch pin (eq: 75HC595) you can define a GPIO pin. Default GPIO.22.
LATCH_DELAY = Optional. You can set LATCH signal period as microseconds. Default 0.
SPEED = SPI port speed as Hz. Default 40000. 
LOOP_DELAY_US = This is main loop delay. If you high detection speed on SHM you can decrease. Small values increase CPU loar. Default 100uS.

# Installation
Just run install script ./install

# Notes
SHM memory key (SHM_SEGMENT_ID) is 1000146 (0x000f42d2). You can read/write SHM with the key in any programming language. Remember: The service listen only (PORT_COUNT/8) byte for changes.
To list SHM areas: ipcs -lm
If you want delete SHM key: sudo ipcrm -M <KEY>
Maz SHM size is as default 1024 (You can drive 8194 port). More than this increase it from def.c
Program uses Gordon's SPI library some parameters as below:

    0.5 MHz
    1 MHz
    2 MHz
    4 MHz
    8 MHz
    16 MHz and
    32 MHz.

(ref: projects.drogon.net/understanding-spi-on-the-raspberry-pi)


# Testing
spiservice --console-mode --show-updates 
