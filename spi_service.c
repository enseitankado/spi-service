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

	// Create shared memory segment (SHM)
	int shmid = create_shm_segment(SHM_SEGMENT_KEY, SHMSZ);

	unsigned char buff[SHMSZ];
	unsigned char buff_old[SHMSZ];
	unsigned char ro_buff[SHMSZ]; 	// Readonly buffer. wiringPiSPIDataRW reads MISO into readonly (ro) bufferr.
	char *shm, *s; 					// shm pointers
	clock_t start, end;				// reporting of updates freq.
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

	// if show-updates-enabled print settings header
	print_current_settings();

	// Showing SHM updates in --console-mode
	if (SHOW_UPDATES == 1) {
		start = clock();
		printf("SHM (SHM_SEGMENT_KEY=%d) updates monitoring.\n", SHM_SEGMENT_KEY);
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

		// if changed then write to the SPI
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

			// Write to the SPI buff and read-back to the buff
			wiringPiSPIDataRW(SPI_PORT, buff, shm_read_count);

			// write back to shm
			if (DISABLE_SHM_WRITE_BACK == 0)
	        for (s = shm; i < shm_read_count; s++) {

printf("shm will be written\n");

            	*s = buff[i];
            	i++;
	        }

			// Latch functionaliry doesnt used if STCP_PIN is 0
			if (STCP_PIN != 0)
				latch(STCP_PIN, STCP_DELAY);

			// show miso input/rx updates
            if (SHOW_UPDATES == 1) {
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
