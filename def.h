// Small values create high CPU usage
static int LOOP_DELAY_US = 100;

// Show SHM updates in console mode
static int SHOW_UPDATES = 0;

// Program runs a SystemD service as default
static int SERVICE_MODE = 1;

// Which gpio output is connected to the Latch pin of ss963 board. GPIO/BCM numbering is used.
static int STCP_PIN = 2;

// Default Latch signal width (uS).
static int STCP_DELAY = 0;

// Default port count can be changed by -p option
static int PORT_COUNT = 96;
/*
	74HC595'in frequency characteristic: Maximum clock pulse frequency for SH_CP or ST_CP:
	Vcc		Min		Typ		Max		Unit
	2.0		9		30		-		MHz		Typ. T:0,033 uSec = 33nS
	4.5		30		91		-		MHz
	6.0		35		108		-		MHz
	Propagation delay Max. 220nS
	
	At VDD=15V, ID=4.4A, VGS=10V and RG=6Ω PJA3406's switching limit: 10,75MHz
		Turn-On Delay Time	3ns
		Turn-On Rise Time	39ns
		Turn-Off Delay Time	23ns
		Turn-Off Fall Time	28ns	
	
	=========================================================================
	RPi Max. SPI Speed 32MHZ
	Transmitted decimals: 1 2 4 8 16 32 64 128
	My Clock Test (25.2.17)

	 CONFIG				MEAUSURED CLOCK(MHz)			EXEC TIME(uS)
	~~~~~~~~    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~    ~~~~~~~~~~~~
	SPISetup	MOSTLY		Min			Max			SPIDataRW		Data Lost
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	0.1			0.05		0.05		0.05		~65				0/8
	0.5*		0.25		0.25		0.25		~70				0/8
	1*			0.5			0.5			0.5			~70				0/8
	2*			1			1			1			~70				0/8
	3			1.455		1.455		1.455		~100			0/8
	4*			2			2			2			~80				0/8
	5			2.66		2.28		2.66		~75				0/8
	6			3.2			2.66		3.2			~68				0/8
	7			4			3.2			4			~65				0/8
	8*			4			3.2			4			~62				0/8
	9			4			4			5.3			~58				0/8
	10			4.8			4.8			6			~60				0/8
	11			5.33		5.33		8			~63				0/8
	12			5.33		5.33		5.33		~55				0/8
	13			5.33		5.33		5.33		~55				0/8
	14			8			5.33		8			~52				0/8
	15			8			5.33		8			~52				0/8
	16*			8			6			8			~55				1/8
	20			8			5.33		8			~50				7/8
	24			4			5.33		4			~49				7/8
	32			8			8			12			~50				0/8
	40			4			2.6			5.3			~45				7/8
	48			3			3			8			~47				7/8

	Gordon's clock dividers for declared frequencies
	ref: projects.drogon.net/understanding-spi-on-the-raspberry-pi
	0.5 MHz
	1 MHz
	2 MHz
	4 MHz
	8 MHz
	16 MHz and
	32 MHz.

*/
static int SPI_PORT = 0;
static int SPI_SPEED = 8 * 1000 * 1000;

// SHM segment için ayrılacak bellek miktarı (byte)
// Bu alan SHM_SEGMENT_ID ile iliskilendirilir ve 
// her seferinde sadece shm_read_count kadar uzunlukta bilgi okunur.
// WiringPi SPI tek sefer 4K lık bloklar okuyabiliyor.
// Gordon amca daha uzunsa 4K lık parçalara böl demiş.
/*
	$ ipcs -lm
	------ Shared Memory Limits --------
	max number of segments = 4096
	max seg size (kbytes) = 4177919
	max total shared memory (kbytes) = 17112760316
	min seg size (bytes) = 1
*/

// SHM must be destroyed manually. Even if the process is closed, it remains in memory until restart.
// The amount of memory to be allocated for the SHM segment (bytes)
// *Maximum value 4KiBiByte
static int SHMSZ = 1024;

/*
SHM segment KEY

Some SHM inspecting tricks:
- To list SHM limits: ipcs -l
- To list SHM segments: ipcs -m
- Detailed list of segments: ipcs -apct
- Remove shared memory segment by key: sudo ipcrm -M <KEY>
- Remove all SHM segments (Warning): sudo ipcrm --all=shm
*/
key_t SHM_SEGMENT_KEY = 1000146; //0x000f42d2 (Mustafa Kemal Ataturk's National ID Number)


// for decimal to binary conversion
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

/*
Resources:
https://projects.drogon.net/raspberry-pi/wiringpi/spi-library/
*/
