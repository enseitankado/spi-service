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

int main(int argc, char *argv[])
{
	// Catch termination signal
	signal(SIGINT, program_halt);

	// Process command line arguments
	get_arguments(argc, argv);

	// Load WiringPi
	init_wiring_pi();

	unsigned char LATCH_ENABLE;
	unsigned char buff[SHMSZ];
	unsigned char buffc[SHMCSZ];
	unsigned char buff_old[SHMSZ];
	unsigned char ro_buff[SHMSZ]; 	// Readonly buffer. wiringPiSPIDataRW reads MISO into readonly (ro) bufferr.
	char *shm, *s, *shmc, *sc;		// shm pointers
	clock_t start, end;				// reporting of updates freq.
	int i, f_c=0, f_c2=0, data_changed;
	int shm_read_count = PORT_COUNT / 8;

	// Create data SHM (Shared Memory Segmeng)
	int shmid = create_shm_segment(SHM_DATA_SEGMENT_KEY, SHMSZ);

	// Can read data SHM segment
	if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
		if (SERVICE_MODE)
			syslog(LOG_ERR, "SHM Data segment cannot be read. Exiting(-2)...");
		else
			printf("SHM Data segment cannot read. Exiting(-2)...");
        perror("shmat");
       	exit(-2);
   	}


    // Create control SHM (Shared Memory Segmeng)
    int shmcid = create_shm_segment(SHM_CONTROL_SEGMENT_KEY, SHMCSZ);

    // Can read control SHM segment
    if ((shmc = shmat(shmcid, NULL, 0)) == (char *) -1) {
        if (SERVICE_MODE)
            syslog(LOG_ERR, "SHM control segment cannot be read. Exiting(-2)...");
        else
            printf("SHM control segment cannot read. Exiting(-2)...");
        perror("shmat");
        exit(-2);
    } else {
		// Set dafault control values
		sc = shmc;
		// latch enabled/disabled
		*sc = STCP_PIN & 1; sc++;
		// latch delay
		*sc = STCP_DELAY; sc++;
		// loop delay
		*sc = LOOP_DELAY_US; sc++;
		// disable shm writeback
		*sc = DISABLE_SHM_WRITE_BACK;
	}

	// if show-updates-enabled print settings header
	print_current_settings();

	// Showing SHM updates in --console-mode
	if (SHOW_UPDATES == 1) {
		start = clock();
		printf("SHM (SHM_DATA_SEGMENT_KEY=%d) updates monitoring.\n", SHM_DATA_SEGMENT_KEY);
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
		for (s = shm; i < shm_read_count; s++) {
			buff[i] = *s;
			ro_buff[i] = buff[i];
			if (ro_buff[i] != buff_old[i])
				data_changed = 1;
			i++;
		}

		// -------------------------------------
		// if data changed then write to the SPI
		// -------------------------------------
		if ( 1 == data_changed)
		{
			// show Tx updates
			if (SHOW_UPDATES == 1) {
				f_c++;
				printf("Tx (%4d Hz): ", f_c2);
				for (i = 0; i < PORT_COUNT/8; i++) {
					printf(BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(buff[i]));
					printf("-");
				}

				printf("\n");
				if (((double) (clock() - start) * 1000.0 / CLOCKS_PER_SEC) > 1000/13) {
					start = clock();
					f_c2 = f_c;
					f_c = 0;
				}
			}
			// Read control SHM
			sc = shmc;
			LATCH_ENABLE = *sc; sc++;
			STCP_DELAY = *sc; sc++;
			LOOP_DELAY_US = *sc; sc++;
			DISABLE_SHM_WRITE_BACK = *sc;

			// Write to the SPI buff and read-back to the buff
			wiringPiSPIDataRW(SPI_PORT, buff, shm_read_count);

			// Write back to shm
			if (DISABLE_SHM_WRITE_BACK == 0) {
				i = 0;
		        for (s = shm; i < shm_read_count; s++) {
	            	*s = buff[i];
	            	i++;
		        }
			}

			// Latch functionaliry doesnt used if STCP_PIN is 0
			if (STCP_PIN != 0)
				latch(STCP_PIN, STCP_DELAY);

			// show miso input/rx updates
            if (SHOW_UPDATES == 1 && DISABLE_SHM_WRITE_BACK == 0) {
                f_c++;
                printf("Rx (%4d Hz): ", f_c2);
                for (i = 0; i < PORT_COUNT/8; i++) {
                    printf(BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(buff[i]));
					printf("-");
				}

                printf("\n");
                if (((double) (clock() - start) * 1000.0 / CLOCKS_PER_SEC) > 1000/13) {
                    start = clock();
                    f_c2 = f_c;
                    f_c = 0;
                }
            }
		}
		else
			usleep(LOOP_DELAY_US);
	}

	exit(EXIT_SUCCESS);
}
