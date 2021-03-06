# Service
Listens for SHM (Shared Memory) changes to send SHM data to the SPI port and 
from there to the your SPI devices's MISO. This is full configurable SPI listener. 
If want to write bytes to SPI port of RPi at high speed, write it to SHM (shared memory) 
with any programming language. The SPI service will detect changes and will read the 
modified memory (SHM) and write it to SPI.0 port. It will then write back the bytes 
returned from the device to SHM due to the nature of the SPI.

![Animated SPI communitaciton ](diagrams/spi_animated.gif)

# Configuration
Configuration options stored in /etc/spi.service.conf as described below:

- **PORT_COUNT** = This is bit count to write SPI port at the same time. Default 96.
- **LATCH_PIN** = Optional. If your SPI hardware has latch pin (eq: 75HC595) you can define a GPIO pin. Default GPIO.22.
- **LATCH_DELAY** = Optional. You can set LATCH signal period as microseconds. Default 0.
- **SPEED** = SPI port speed as Hz. Default 40000. 
- **LOOP_DELAY_US** = This is main loop delay. If you high detection speed on SHM you can decrease. Small values increase CPU loar. Default 100uS.

# Installation
```
pi@raspberrypi:~ $ git clone https://github.com/enseitankado/spi-service.git
pi@raspberrypi:~ $ cd spi-service/
pi@raspberrypi:~/spi-service $./install.sh

Stopping spi.service...
Failed to stop spi.service: Unit spi.service not loaded.
Failed to kill unit spi.service: Unit spi.service not loaded.
Old binary removed.
Compiling from source code...
New binary created.
Old SystemD unit file removed.
Old configuration file removed.
New SystemD unit file installing...
New configuration file created at /etc/spi.service.conf with default settings.
Created symlink /etc/systemd/system/multi-user.target.wants/spi.service → /lib/systemd/system/spi.service.
The service is spi.service starting...

● spi.service - SHM (Shared Memory) listener for ss963 serial driver board and SPI driver
   Loaded: loaded (/lib/systemd/system/spi.service; enabled; vendor preset: enabled)
   Active: active (running) since Sun 2020-06-14 03:59:29 +03; 108ms ago
 Main PID: 16818 (spiservice)
    Tasks: 1 (limit: 2200)
   Memory: 248.0K
   CGroup: /system.slice/spi.service
           └─16818 /usr/sbin/spiservice --port-count=96 --latch-pin=2 --latch-delay=0 --speed=4000000 --loop-delay-us=100

Haz 14 03:59:29 raspberrypi spiservice[16818]: SPI Port        : 0
Haz 14 03:59:29 raspberrypi spiservice[16818]: STCP/LATCH pin  : 2 (GPIO/BCM)
Haz 14 03:59:29 raspberrypi spiservice[16818]: STCP Delay      : 0 uS
Haz 14 03:59:29 raspberrypi spiservice[16818]: SPI Speed       : 4000000 Hz
Haz 14 03:59:29 raspberrypi spiservice[16818]: Loop Delay (uS) : 100 uS
Haz 14 03:59:29 raspberrypi spiservice[16818]: Port Count      : 96
Haz 14 03:59:29 raspberrypi spiservice[16818]: Board Count     : 1
Haz 14 03:59:29 raspberrypi spiservice[16818]: SHM_SEGMENT_ID  : 1000146
Haz 14 03:59:29 raspberrypi spiservice[16818]: SHM Size        : 1024 bytes/8192 ports
Haz 14 03:59:29 raspberrypi spiservice[16818]: Listening changes for first 12 bytes (96 ports) of SHM...

Finished.
```

# Uninstall 
Simply run unistall.sh 

```
pi@raspberrypi:~ $ ./sp-service/uninstall.sh
```

# Notes
SHM memory key (SHM_SEGMENT_ID) is 1000146 (0x000f42d2). You can read/write SHM with the key in any programming language. Remember: The service listen only (PORT_COUNT/8) byte for changes.
- To list SHM areas: ipcs -lm
- If you want delete SHM key: sudo ipcrm -M <KEY>
Maz SHM size is as default 1024 (You can drive 8194 port with it). 
Otherwise you can change this from def.c and run install.sh

Somethings about SPI writaback and --disable-write-back switch:
Data that was in your buffer is overwritten by data returned from the SPI (MISO) bus.

The driver supports the following speeds:
```
  cdiv    speed
     2    125.0 MHz
     4     62.5 MHz
     8     31.2 MHz
    16     15.6 MHz
    32      7.8 MHz
    64      3.9 MHz
   128     1953 kHz
   256      976 kHz
   512      488 kHz
  1024      244 kHz
  2048      122 kHz
  4096       61 kHz
  8192     30.5 kHz
 16384     15.2 kHz
 32768     7629 Hz
```
When asking for say 24 MHz, the actual speed will be 15.6 MHz.

- [ref: projects.drogon.net/understanding-spi-on-the-raspberry-pi)](http://projects.drogon.net/understanding-spi-on-the-raspberry-pi)
- [ref: www.raspberrypi.org/documentation/hardware/raspberrypi/spi/README.md](https://www.raspberrypi.org/documentation/hardware/raspberrypi/spi/README.md)

# Command Line

You can also use spiservice as a command line tool while running service. 
However, I recommend that you stop the service with the 
`sudo systemctl stop spi.service` command to avoid logical confusion. 
When using the spiservice tool from the command line, configuration in 
`/etc/spi.service.conf` will be ommited.
You must use its own option for each setting as parameter.
You can give the parameters `-h` or `--help` to see the options.

```
pi@raspberrypi:~/spi-service $ spiservice -h
 OPTIONS
         -c, --console-mode
                 Enable console mode.
                 In console mode the outputs redirected to console instead of syslog.

         -u, --show-updates
                 Used in console mode to print out SHM updates.
                 Printing updates to the console dramatically reduces SPI update speed.
                 Because of terminal screen refreshing very slow according to SPI peripheral.

         -s, --speed <value>
                 SPI communication speed as Hertz (Hz).
                 Available rates are (as MHz): 0.5,1,2,4,8,16,32.
                 Default value is: 8000000 Hz

         -g, --latch-pin <value>
                 Latch pin number as GPIO/BCM numbering.
                 Default value is: 0 and disabled.

         -l, --latch-delay <value>
                 Latch signal width as microsecond.
                 Default value is: 0 uS

         -p, --port-count <value>
                 Port count to drive. Port count must be a multiple of 96.
                 Default value is: 96

         -d, --loop-delay-us <value>
                 SHM scanning delay as micro seconds (us).
                 With small values, SHM is read more often, whereas high CPU usage occurs.
                 Default value is: 100

         -k, --shm-segment-key <value>
                 Key value of shm memory to monitor.
                 Key value must be decimal.
                 The data read back to the spi buffer is also written back to the SHM memory.
                 Default value is: 1000146

         -w, --disable-shm-writeback <value>
                 As a default the SPI readback data written back to the SHM.
                 Use this key to disable write back if you want shm data to remain unchanged.
                 You can give a value other than 0 to enable this key.
                 Default value is: 1000146

```
# Testing
Below command runs with service at the same time. I suggest firstly stop the service (sudo systemctl stop spi.service)
```
$ spiservice --console-mode --show-updates 

SPI Port        : 0
STCP/LATCH pin  : 2 (GPIO/BCM)
STCP Delay      : 0 uS
SPI Speed       : 8000000 Hz
Loop Delay (uS) : 100 uS
Port Count      : 96
Board Count     : 1
SHM_SEGMENT_KEY : 1000146
SHM Size        : 1024 bytes/8192 ports
SHM Writeback   : Enabled
Listening changes for first 12 bytes (96 ports) of SHM...
To break press ^C

SHM (SHM_SEGMENT_KEY=1000146) updates monitoring.
Tx (   0 Hz): 000000000000000000000000000000000000000000000010000000000000000000000000000000000000000000000000
Rx (   0 Hz): 000000000000000000000000000000000000000000000010000000000000000000000000000000000000000000000000

```


# Some datasheet

    74HC595'in frequency characteristic: Maximum clock pulse frequency for SH_CP or ST_CP:
    Vcc     Min     Typ     Max     Unit
    2.0     9       30      -       MHz     Typ. T:0,033 uSec = 33nS
    4.5     30      91      -       MHz
    6.0     35      108     -       MHz
    Propagation delay Max. 220nS

    At VDD=15V, ID=4.4A, VGS=10V and RG=6Ω PJA3406's switching limit: 10,75MHz
        Turn-On Delay Time  3ns
        Turn-On Rise Time   39ns
        Turn-Off Delay Time 23ns
        Turn-Off Fall Time  28ns

---
    RPi Max. SPI Speed 32MHZ
    Transmitted decimals: 1 2 4 8 16 32 64 128
    My Clock Test (25.2.17)

     CONFIG             MEAUSURED CLOCK(MHz)            EXEC TIME(uS)
    ~~~~~~~~    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~    ~~~~~~~~~~~~
    SPISetup    MOSTLY      Min         Max         SPIDataRW       Data Lost
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    0.1         0.05        0.05        0.05        ~65             0/8
    0.5*        0.25        0.25        0.25        ~70             0/8
    1*          0.5         0.5         0.5         ~70             0/8
    2*          1           1           1           ~70             0/8
    3           1.455       1.455       1.455       ~100            0/8
    4*          2           2           2           ~80             0/8
    5           2.66        2.28        2.66        ~75             0/8
    6           3.2         2.66        3.2         ~68             0/8
    7           4           3.2         4           ~65             0/8
    8*          4           3.2         4           ~62             0/8
    9           4           4           5.3         ~58             0/8
    10          4.8         4.8         6           ~60             0/8
    11          5.33        5.33        8           ~63             0/8
    12          5.33        5.33        5.33        ~55             0/8
    13          5.33        5.33        5.33        ~55             0/8
    14          8           5.33        8           ~52             0/8
    15          8           5.33        8           ~52             0/8
    16*         8           6           8           ~55             1/8
    20          8           5.33        8           ~50             7/8
    24          4           5.33        4           ~49             7/8
    32          8           8           12          ~50             0/8
    40          4           2.6         5.3         ~45             7/8
    48          3           3           8           ~47             7/8

# About Latch Function

If your device at the end of the SPI port has a Latch control, you can configure 
the spi service accordingly. The service sends the data of each SHM update to 
the device and can activate the latch line for the desired time. 
The pin and latch time to provide the latch function can be set via 
the configuration file. The connection diagram of a driver card with the 
visual latch function below appears. SPI service activates the latch pin 
after writing the bits to the card, and the bits are reflected in the outputs of 
the card. Latch function is normally not enabled. However, if a GPIO pin is 
assigned for the latch function, the service applies the latch signal for 
the STCP delay time.

![SS963 Serial Driver Card](diagrams/ss963_diagram.png)
[More info about SS963](http://www.izlencebilisim.com/urun/ss963-seri-surucu-karti/2/)
