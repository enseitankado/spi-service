/*
	Bu fonksiyon segment_id adýnda, SHMSZ boyutlu bir SHM segmenti 
	olusturur ve icini 0 larla doldurur.
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

void print_usage() 
{
	printf("\n OPTIONS\n");
	
	printf("\t -c, --console-mode\n");
	printf("\t\t Used for console mode.\n");
	printf("\t\t In console mode outputs redirect to console instead of syslog.\n");
	printf("\t\t \n");
	
	printf("\t -u, --show-updates\n");
	printf("\t\t Used in console mode to print out SHM updates.\n");
	printf("\t\t Printing updates to the console dramatically reduces SPI update speed.\n");
	printf("\t\t \n");
	
	printf("\t -s, --speed <value>\n");
	printf("\t\t SPI communication speed as Hertz (Hz).\n");
	printf("\t\t Available rates are (as MHz): 0.5,1,2,4,8,16,32.\n");
	
	printf("\t\t Default value is %d Hz dir\n", SPI_SPEED);
	printf("\t\t \n");
	
	printf("\t -l, --latch-delay <value>\n");
	printf("\t\t Latch signal width as microsecond.\n");
	printf("\t\t Default value is %d uS dir\n", STCP_DELAY);
	printf("\t\t \n");	
	
	printf("\t -g, --latch-pin <value>\n");
	printf("\t\t Latch pin number as GPIO/BCM numbering.\n");
	printf("\t\t Default value is %d \n", STCP_PIN);
	printf("\t\t \n");	
	
	printf("\t -p, --port-count <value>\n");
	printf("\t\t Port count to drive. Port count must be a multiple of 96.\n");
	printf("\t\t Default value %d \n", PORT_COUNT);
	printf("\t\t \n");
	
	printf("\t -d, --loop-delay-us <value>\n");
	printf("\t\t SHM scanning delay as micro seconds (us).\n");
	printf("\t\t With small values, SHM is read more often, whereas high CPU usage occurs.\n");
	printf("\t\t Default value %d \n", LOOP_DELAY_US);
	printf("\t\t \n");
	exit(EXIT_SUCCESS);
}
    
static void get_arguments(int argc, char *argv[]) 
{
	int opt = 0;

	/* Available arguments and parameters */
	static struct option long_options[] =
	{
		{"help", 			no_argument, 		NULL, 	'h'}, // Argumansiz secenek			
		{"console-mode", 	no_argument, 		NULL, 	'c'}, // Argumansiz secenek
		{"show-updates",  	no_argument, 		NULL, 	'u'}, // Argumansiz secenek		
		{"speed",  			required_argument, 	NULL, 	's'}, // Argumanli secenek		
		{"latch-delay",  	required_argument, 	NULL, 	'l'}, // Argumanli secenek
		{"latch-pin",  		required_argument, 	NULL, 	'g'}, // Argumanli secenek
		{"port-count",  	required_argument, 	NULL, 	'p'}, // Argumanli secenek
		{"loop-delay-us",  	required_argument, 	NULL, 	'd'}, // Argumanli secenek
		{0, 				0, 					0, 		0}
	};
	
	/* getopt_long arguman indeksi. */
	int option_index = 0;	
		
	//while ((opt = getopt_long (argc, argv, "hixas:d:l:p:", long_options, &option_index)) != -1)
	while ((opt = getopt_long (argc, argv, "hixas:l:p:", long_options, &option_index)) != -1)
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
			case 'h':
				print_usage();
				break;
		}
	}
}
