clear
# Stop the service
printf "> Stopping spi.service...\n"
sudo systemctl stop spi.service
sudo systemctl kill spi.service

# Kill instances
printf "> Killing spiservice instances...\n"
sudo pkill spiservice

# Remove old binary
sudo rm -rf /usr/sbin/spiservice
if [ ! -f /usr/sbin/spiservice ]; then
    printf "> Old binary removed.\n"
else
	printf "\n\n> ERROR: Old binary was not removed.\n\n"
fi


# Compile and copy to target location /usr/sbin/
printf "> Compiling from source code...\n"
sudo gcc -o /usr/sbin/spiservice spi_service.c -lwiringPi -Ofast -s -Wno-implicit

if [ -f /usr/sbin/spiservice ]; then
	printf "> New binary created."
fi

# Remove old service file
sudo rm -rf /lib/systemd/system/spi.service
if [ ! -f /lib/systemd/system/spi.service ]; then
    printf "\n> Old SystemD unit file removed.\n"
else
	printf "\n\n> ERROR: Old SystemD unit file was not removed.\n\n"
fi

# Remove conf file
sudo rm -rf /etc/spi.service.conf
if [ ! -f /etc/spi.service.conf ]; then
    printf "> Old configuration file removed.\n"
else
	printf "\n\n> ERROR: Old configuration file was not removed.\n\n"
fi

# Install SystemD service
printf "> New SystemD unit file installing...\n"

sudo cp spi.service.conf.template /etc/spi.service.conf

if [ -f /etc/spi.service.conf ]; then
	printf "> New configuration file created at /etc/spi.service.conf with default settings.\n"
else
	printf "\n\n> ERROR: New configuration file was not created at /etc/spi.service.conf.\n"
fi

sudo cp spi.service.template /lib/systemd/system/spi.service
sudo systemctl daemon-reload
sudo systemctl enable spi.service
sudo systemctl unmask spi.service

printf "> The service is spi.service starting...\n\n"
sudo systemctl start spi.service
sudo systemctl status spi.service

printf "\nFinished.\n"
