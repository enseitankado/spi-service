clear
# Stop the service
printf "> Stopping ss963.service...\n"
sudo systemctl stop ss963.service
sudo systemctl kill ss963.service

# Remove old binary
sudo rm -rf /usr/sbin/ss963
if [ ! -f /usr/sbin/ss963 ]; then
    printf "> Old binary removed.\n"
else
	printf "\n\n> ERROR: Old binary was not removed.\n\n"
fi


# Compile and copy to target location /usr/sbin/
printf "> Compiling from source code...\n"
sudo gcc -o /usr/sbin/ss963 ss963.c -lwiringPi -Ofast -s -Wno-implicit

if [ -f /usr/sbin/ss963 ]; then
	printf "> New binary created."
fi

# Remove old service file
sudo rm -rf /lib/systemd/system/ss963.service
if [ ! -f /lib/systemd/system/ss963.service ]; then
    printf "\n> Old SystemD unit file removed.\n"
else
	printf "\n\n> ERROR: Old SystemD unit file was not removed.\n\n"
fi

# Remove conf file
sudo rm -rf /etc/ss963.service.conf
if [ ! -f /etc/ss963.service.conf ]; then
    printf "> Old configuration file removed.\n"
else
	printf "\n\n> ERROR: Old configuration file was not removed.\n\n"
fi

# Install SystemD service
printf "> New SystemD unit file installing...\n"

sudo cp ss963.service.conf.template /etc/ss963.service.conf

if [ -f /etc/ss963.service.conf ]; then
	printf "> New configuration file created at /etc/ss963.service.conf with default settings.\n"
else
	printf "\n\n> ERROR: New configuration file was not created at /etc/ss963.service.conf.\n"
fi

sudo cp ss963.service.template /lib/systemd/system/ss963.service
sudo systemctl daemon-reload
sudo systemctl enable ss963.service
sudo systemctl unmask ss963.service

printf "> The service is ss963.service starting...\n\n"
sudo systemctl start ss963.service
sudo systemctl status ss963.service

printf "\nFinished.\n"