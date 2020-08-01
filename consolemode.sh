#!/bin/bash

if [ -f /usr/sbin/spiservice ]; then
	printf "> Service is stopping...\n"
	sudo systemctl stop spi.service
	sudo pkill spiservice

	PORT_COUNT=$(awk -F "=" '/PORT_COUNT/ {print $2}' /etc/spi.service.conf)

	printf "> The binarry running in debug mode.\n\n"
	/usr/sbin/spiservice --console-mode --show-updates --latch-pin=22 --disable-shm-writeback=1 --port-count=$PORT_COUNT

	printf "> Service is starting...\n"
    sudo systemctl start spi.service
else
	printf "> Soryy, binary not exists at /usr/sbin/spiservice"
fi
