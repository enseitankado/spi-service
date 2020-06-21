/*
	Create SHM segmet by id and fill zero in it.
*/
int create_shm_segment(key_t segment_id, int SHMSZ)
{
	char c;
	int shmid;
	char *shm, *s;

	/*
	 * Create the segment.
	 */
	if ((shmid = shmget(segment_id, SHMSZ, IPC_CREAT | 0666)) < 0) {
		if (SERVICE_MODE)
			syslog(LOG_ERR, "Failed to create shm segment!\n");
		else
			printf("Failed to create shm segment!\n");
		perror("shmget");
		exit(-40);
	}

	/*
	 * Now we attach the segment to our data space.
	 */
	if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
		if (SERVICE_MODE)
			syslog(LOG_ERR, "Failed to attach with SHM segment!\n");
		else
			printf("Failed to attach with SHM segment!\n");
		perror("shmat");
		exit(-30);
	}
	return shmid;
}

void program_halt(int sig){
	if (SERVICE_MODE)
		syslog(LOG_INFO, "Termination (Signal %d) detected. Exiting...", sig);
	else
		printf("Termination (Signal %d) detected. Exiting...", sig);
	exit(EXIT_SUCCESS);
}

void init_wiring_pi()
{
	wiringPiSetupGpio();
	if (wiringPiSPISetup(SPI_PORT, SPI_SPEED) < 0)
	{
		if (SERVICE_MODE)
			syslog(LOG_INFO, "SPI cannot be initilized! Exiting(-10))\n");
		else
			printf ("SPI cannot be initilized! Exiting(-10))\n");
		exit(-10);
	}
	pinMode (STCP_PIN, OUTPUT);
}

/*
	74HC595'in latch islevi icin STCP (Store Clock Pulse)
	saat sinyali veriliyor.
	Not: 74HC595 tarafindan yapilan store islemi sinyalin
	yukselen kenarinda gerceklestirilir.
*/
void latch(int latch_pin, int delay)
{
	digitalWrite(latch_pin, HIGH);
    usleep(delay);
    digitalWrite(latch_pin, LOW);
}

void print_current_settings()
{
    if (SERVICE_MODE) {
        syslog(LOG_INFO, "SPI Port        : %d\n", SPI_PORT);

		if (STCP_PIN == 0) {
            syslog(LOG_INFO, "STCP/LATCH pin  : Disabled\n");
            syslog(LOG_INFO, "STCP Delay      : Disabled\n");
		} else {
	        syslog(LOG_INFO, "STCP/LATCH pin  : %d (GPIO/BCM)\n", STCP_PIN);
    	    syslog(LOG_INFO, "STCP Delay      : %d uS\n", STCP_DELAY);
		}

        syslog(LOG_INFO, "SPI Speed       : %d Hz\n", SPI_SPEED);
        syslog(LOG_INFO, "Loop Delay (uS) : %d uS\n", LOOP_DELAY_US);
        syslog(LOG_INFO, "Port Count      : %d \n", PORT_COUNT);
        syslog(LOG_INFO, "Board Count     : %d\n", PORT_COUNT/96);
        syslog(LOG_INFO, "SHM_SEGMENT_KEY : %d\n", SHM_SEGMENT_KEY);
        syslog(LOG_INFO, "SHM Size        : %d bytes/%d ports\n", SHMSZ, SHMSZ*8);
		if (DISABLE_SHM_WRITE_BACK == 1)
			syslog(LOG_INFO, "SHM Writeback   : Disabled\n");
		else
			syslog(LOG_INFO, "SHM Writeback   : Enabled\n");
        syslog(LOG_WARNING, "Listening changes for first %d bytes (%d ports) of SHM...\n", PORT_COUNT/8, PORT_COUNT);
    } else {

        printf("SPI Port        : %d\n", SPI_PORT);
		if (STCP_PIN == 0) {
    	    printf("STCP/LATCH pin  : Disabled\n");
	        printf("STCP Delay      : Disabled\n");
		} else {
	        printf("STCP/LATCH pin  : %d (GPIO/BCM)\n", STCP_PIN);
	        printf("STCP Delay      : %d uS\n", STCP_DELAY);
		}

        printf("SPI Speed       : %d Hz\n", SPI_SPEED);
        printf("Loop Delay (uS) : %d uS\n", LOOP_DELAY_US);
        printf("Port Count      : %d \n", PORT_COUNT);
        printf("Board Count     : %d\n", PORT_COUNT/96);
        printf("SHM_SEGMENT_KEY : %d\n", SHM_SEGMENT_KEY);
        printf("SHM Size        : %d bytes/%d ports\n", SHMSZ, SHMSZ*8);
        if (DISABLE_SHM_WRITE_BACK == 1)
	        printf("SHM Writeback   : Disabled\n");
        else
            printf("SHM Writeback   : Enabled\n");

        printf("Listening changes for first %d bytes (%d ports) of SHM...\n", PORT_COUNT/8, PORT_COUNT);
        printf("To break press ^C\n\n");
    }
}

void print_usage()
{
	printf("\n OPTIONS\n");

	printf("\t -c, --console-mode\n");
	printf("\t\t Enable console mode.\n");
	printf("\t\t In console mode the outputs redirected to console instead of syslog.\n");
	printf("\t\t \n");

	printf("\t -u, --show-updates\n");
	printf("\t\t Used in console mode to print out SHM updates.\n");
	printf("\t\t Printing updates to the console dramatically reduces SPI update speed.\n");
	printf("\t\t Because of terminal screen refreshing very slow according to SPI peripheral.\n");
	printf("\t\t \n");

	printf("\t -s, --speed <value>\n");
	printf("\t\t SPI communication speed as Hertz (Hz).\n");
	printf("\t\t Available rates are (as MHz): 0.5,1,2,4,8,16,32.\n");

	printf("\t\t Default value is: %d Hz\n", SPI_SPEED);
	printf("\t\t \n");

	printf("\t -g, --latch-pin <value>\n");
	printf("\t\t Latch pin number as GPIO/BCM numbering.\n");
	printf("\t\t Default value is: %d and disabled.\n", STCP_PIN);
	printf("\t\t \n");

    printf("\t -l, --latch-delay <value>\n");
    printf("\t\t Latch signal width as microsecond.\n");
    printf("\t\t Default value is: %d uS\n", STCP_DELAY);
    printf("\t\t \n");

	printf("\t -p, --port-count <value>\n");
	printf("\t\t Port count to drive. Port count must be a multiple of 96.\n");
	printf("\t\t Default value is: %d \n", PORT_COUNT);
	printf("\t\t \n");

	printf("\t -d, --loop-delay-us <value>\n");
	printf("\t\t SHM scanning delay as micro seconds (us).\n");
	printf("\t\t With small values, SHM is read more often, whereas high CPU usage occurs.\n");
	printf("\t\t Default value is: %d \n", LOOP_DELAY_US);
	printf("\t\t \n");

    printf("\t -k, --shm-segment-key <value>\n");
    printf("\t\t Key value of shm memory to monitor.\n");
    printf("\t\t Key value must be decimal. \n");
	printf("\t\t The data read back to the spi buffer is also written back to the SHM memory.");
    printf("\t\t Default value is: %d \n", SHM_SEGMENT_KEY);
    printf("\t\t \n");

    printf("\t -w, --disable-shm-writeback <value>\n");
    printf("\t\t As a default the SPI readback data written back to the SHM.\n");
    printf("\t\t Use this key to disable write back if you want shm data to remain unchanged.\n");
    printf("\t\t \n");

	exit(EXIT_SUCCESS);
}

static void get_arguments(int argc, char *argv[])
{
	int opt = 0;

	/* Available arguments and parameters */
	static struct option long_options[] =
	{
		{"help", 					no_argument, 		NULL, 	'h'}, // Argumansiz secenek
		{"console-mode", 			no_argument, 		NULL, 	'c'}, // Argumansiz secenek
		{"show-updates",  			no_argument, 		NULL, 	'u'}, // Argumansiz secenek
		{"speed",  					required_argument, 	NULL, 	's'}, // Argumanli secenek
		{"latch-delay",  			required_argument, 	NULL, 	'l'}, // Argumanli secenek
		{"latch-pin",  				required_argument, 	NULL, 	'g'}, // Argumanli secenek
		{"port-count",  			required_argument, 	NULL, 	'p'}, // Argumanli secenek
		{"loop-delay-us",  			required_argument, 	NULL, 	'd'}, // Argumanli secenek
		{"shm-segment-key", 		required_argument,  NULL,   'k'}, // Argumanli secenek
        {"disable-shm-writeback", 	no_argument,  		NULL,   'w'}, // Argumanli secenek
		{0, 						0, 					0, 		0}
	};

	/* getopt_long arguman indeksi. */
	int option_index = 0;

	while ((opt = getopt_long (argc, argv, "hwcus:l:g:p:d:k:", long_options, &option_index)) != -1)
	{
		/* Secenekleri parselle */
		switch (opt)
		{
			case 's':
				SPI_SPEED = atoi(optarg);
				break;
			case 'l':
				STCP_DELAY = atoi(optarg);
				break;
			case 'p':
				PORT_COUNT = atoi(optarg);
				break;
			case 'g':
				STCP_PIN = atoi(optarg);
				break;
			case 'd':
				LOOP_DELAY_US = atoi(optarg);
				break;
			case 'c':
				SERVICE_MODE = 0;
				break;
			case 'u':
				SHOW_UPDATES = 1;
				break;
			case 'k':
				SHM_SEGMENT_KEY = atoi(optarg);
				break;
			case 'w':
				DISABLE_SHM_WRITE_BACK = 1;
				break;
			case 'h':
				print_usage();
				break;
		}
	}
}
