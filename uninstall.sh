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

printf "> The service removing...\n\n"
sudo systemctl disable spi.service
sudo systemctl mask spi.service

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

sudo systemctl daemon-reload

printf "\nFinished.\n"
