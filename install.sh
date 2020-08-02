#!/bin/bash

# Compile and copy to target location /usr/sbin/
printf "> Compiling from source codes...\n"
sudo rm -rf ./spiservice
sudo gcc -o ./spiservice spi_service.c -lwiringPi -Ofast -s -Wno-implicit

if [ -f ./spiservice ]; then
	printf "> New binary created at ./spiservice \n"

	# Stop and remove the already installed service
	if [ -f /usr/sbin/spiservice ]; then
		printf "> Service already installed.\n"

		# Remove SystemD service
		printf "> Removing spi.service...\n"
		sudo systemctl stop spi.service > /dev/null 2>&1
		sudo systemctl kill spi.service > /dev/null 2>&1
		sudo pkill spiservice > /dev/null 2>&1
		sudo systemctl disable spi.service > /dev/null 2>&1
		sudo systemctl mask spi.service > /dev/null 2>&1

		# Remove old binary
		sudo rm -rf /usr/sbin/spiservice
		if [ ! -f /usr/sbin/spiservice ]; then
    		printf "> Old binary removed.\n"
		else
	        printf "\n\n> ERROR: Old binary was not removed.\n\n"
	        exit 1
		fi

		# Remove old service file
		sudo rm -rf /lib/systemd/system/spi.service
		if [ ! -f /lib/systemd/system/spi.service ]; then
		    printf "> Old service unit file /lib/systemd/system/spi.service removed.\n"
		else
		    printf "\n\n> ERROR: Old SystemD unit file was not removed.\n\n"
		    exit 2
		fi
	fi
fi


# Install SystemD service
printf "\n> Installing service (spi.service)...\n"


# Create new SystemD service unit file
sudo mv ./spiservice /usr/sbin/spiservice
if [ -f /usr/sbin/spiservice ]; then
    printf "> Service binary copied to /usr/sbin/spiservice.\n"
else
    printf "\n\n> ERROR: Service binary file (/usr/sbin/spiservice) was not copied.\n\n"
    exit 3
fi

# Create new SystemD service unit file
sudo cp spi.service.template /lib/systemd/system/spi.service
if [ -f /lib/systemd/system/spi.service ]; then
	printf "> New service unit file created at /lib/systemd/system/spi.service.\n"
else
    printf "\n\n> ERROR: New service unit file was not created.\n\n"
    exit 3
fi

# Keep or overwrite conf file
if [ -f /etc/spi.service.conf ]; then
    printf "> Service configuration file detected at: /etc/spi.service.conf \n\n"
	cat /etc/spi.service.conf
	printf "\n"
    read -p "  Do you want to keep current configuration file? [Y/n]: " -n 1 -r
    if [[  $REPLY =~ ^[Nn]$ ]]
    then
        sudo rm -rf /etc/spi.service.conf
        if [ ! -f /etc/spi.service.conf ]; then
            printf "\n> Old configuration file removed.\n"
            sudo cp spi.service.conf.template /etc/spi.service.conf
            if [ -f /etc/spi.service.conf ]; then
            printf "> New configuration file created at /etc/spi.service.conf with default settings.\n"
            else
                printf "\n\n> ERROR: New configuration file was not created at /etc/spi.service.conf.\n"
				exit 3
            fi
        else
            printf "\n\n> ERROR: Old configuration file was not removed.\n\n"
			exit 4
        fi
	else
		printf "\n> Old configuration file /etc/spi.service.conf keeped.\n"
    fi
fi

sudo systemctl unmask spi.service >/dev/null 2>&1
sudo systemctl enable spi.service >/dev/null 2>&1
sudo systemctl daemon-reload >/dev/null 2>&1

printf "> The service is starting...\n\n"
sudo systemctl start spi.service
sudo systemctl status spi.service --no-pager

printf "\n> All done. Thank you.\n\n"
exit 0
