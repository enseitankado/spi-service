#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <wiringPi.h>
#include <math.h>
#include <time.h>
#include <syslog.h>

// SHM functions
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

// SIGINT
#include <signal.h>

#include "def.h"
#include "lib.h"

clock_t start, end;

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

int main(int argc, char *argv[])
{
	// Catch termination signal
	signal(SIGINT, program_halt);

	// Process command line arguments
	get_arguments(argc, argv);

	// Load WiringPi
	init_wiring_pi();

	// Create shared memory segment (SHM)
	int shmid = create_shm_segment(SHM_SEGMENT_KEY, SHMSZ);

	unsigned char buff[SHMSZ];
	unsigned char buff_old[SHMSZ];
	unsigned char ro_buff[SHMSZ]; // Readonly buffer. wiringPiSPIDataRW reads MISO into readonly (ro) bufferr.
	char *shm, *s;
	clock_t start, end;
	int i, f_c=0, f_c2=0, data_changed, shm_read_count = PORT_COUNT / 8;

	// Can read SHM segment
	if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
			if (SERVICE_MODE)
				syslog(LOG_ERR, "SHM Memory cannot read. Exiting(-2)...");
			else
				printf("SHM Memory cannot read. Exiting(-2)...");
	        perror("shmat");
        	exit(-2);
    	}

	if (SERVICE_MODE) {
		syslog(LOG_INFO, "SPI Port        : %d\n", SPI_PORT);
		syslog(LOG_INFO, "STCP/LATCH pin  : %d (GPIO/BCM)\n", STCP_PIN);
		syslog(LOG_INFO, "STCP Delay      : %d uS\n", STCP_DELAY);
		syslog(LOG_INFO, "SPI Speed       : %d Hz\n", SPI_SPEED);
		syslog(LOG_INFO, "Loop Delay (uS) : %d uS\n", LOOP_DELAY_US);
		syslog(LOG_INFO, "Port Count      : %d \n", PORT_COUNT);
		syslog(LOG_INFO, "Board Count     : %d\n", PORT_COUNT/96);
		syslog(LOG_INFO, "SHM_SEGMENT_KEY : %d\n", SHM_SEGMENT_KEY);
		syslog(LOG_INFO, "SHM Size        : %d bytes/%d ports\n", SHMSZ, SHMSZ*8);
		syslog(LOG_WARNING, "Listening changes for first %d bytes (%d ports) of SHM...\n", PORT_COUNT/8, PORT_COUNT);
	} else {
		printf("SPI Port        : %d\n", SPI_PORT);
		printf("STCP/LATCH pin  : %d (GPIO/BCM)\n", STCP_PIN);
		printf("STCP Delay      : %d uS\n", STCP_DELAY);
		printf("SPI Speed       : %d Hz\n", SPI_SPEED);
		printf("Loop Delay (uS) : %d uS\n", LOOP_DELAY_US);
		printf("Port Count      : %d \n", PORT_COUNT);
		printf("Board Count     : %d\n", PORT_COUNT/96);
		printf("SHM_SEGMENT_KEY : %d\n", SHM_SEGMENT_KEY);
		printf("SHM Size        : %d bytes/%d ports\n", SHMSZ, SHMSZ*8);
		printf("Listening changes for first %d bytes (%d ports) of SHM...\n", PORT_COUNT/8, PORT_COUNT);
		printf("To break press ^C\n\n");
	}

	// Showing SHM updates in --console-mode
	if (SHOW_UPDATES == 1) {
		start = clock();
		printf("SHM updates listing.\n");
	}

	// Infinitive loop
	while (1==1)
	{
		// Save old buffer
		for (i = 0; i < shm_read_count; i++)
			buff_old[i] = ro_buff[i];

		i = 0;
		data_changed = 0;

		// Compare buffers
		for (s = shm; i < shm_read_count; s++)
		{
			buff[i] = *s;
			ro_buff[i] = buff[i];
			if (ro_buff[i] != buff_old[i])
				data_changed = 1;
			i++;
		}

		// if changed then write to the SPI
		if ( 1 == data_changed)
		{
			if (SHOW_UPDATES == 1) {
				f_c++;
				printf("%4d Hz: ", f_c2);
				for (i = 0; i < PORT_COUNT; i++)
					printf(BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(buff[i]));

				printf("\n");
				if (((double) (clock() - start) * 1000.0 / CLOCKS_PER_SEC) > 1000/13) {
					start = clock();
					f_c2 = f_c;
					f_c = 0;
				}
			}

			// Write to the SPI buff and read-back to the buff
			wiringPiSPIDataRW(SPI_PORT, buff, shm_read_count);
			latch(STCP_PIN, STCP_DELAY);
		}
		else
			usleep(LOOP_DELAY_US);
	}

	exit(EXIT_SUCCESS);
}
