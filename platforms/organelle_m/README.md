# Organelle M and S (on pi CM3)

Start with 2025 EYOG base bookworm image

## configure stuff

make stuff in /root readable

    sudo chmod +xr /root

make sdcard and usb directories

    sudo mkdir /sdcard
    sudo chown music:music /sdcard
    sudo mkdir /usbdrive
    sudo chown music:music /usbdrive

add this to /etc/fstab to mount the patches partition:

    /dev/mmcblk0p3 /sdcard  ext4 defaults,noatime 0 0

reboot and change owner

    sudo chown music:music /sdcard 

remove this if it got added along the way

    sudo rm -fr /sdcard/lost+found/

enable rt. in /etc/security/limits.conf add to end:

    @music - rtprio 99
    @music - memlock unlimited
    @music - nice -10

enable i2c from raspi-config

install software

    sudo apt-get install zip jwm xinit x11-utils x11-xserver-utils lxterminal pcmanfm libasound2-dev liblo-dev liblo-tools mpg123 puredata conky python3-liblo

    git clone https://github.com/WiringPi/WiringPi.git
    cd WiringPi
    ./build debian
    mv debian-template/wiringpi_3.16_armhf.deb .
    sudo chmod o+r ./wiringpi_3.16_armhf.deb
    sudo apt install ./wiringpi_3.16_armhf.deb

Don't persist logs. add Storage=volatile to /etc/systemd/journald.conf then remove old sudo rm -rf /var/log/journal

Don't log nmcli commands Open the sudoers file for editing using visudo:

    sudo visudo

Add a rule to disable logging for music user:

    Defaults:music !syslog

disable swap

    sudo dphys-swapfile swapoff
    sudo dphys-swapfile uninstall
    sudo update-rc.d dphys-swapfile remove

run helpers/disable_services.sh

copy config.txt and cmdline.txt


    sudo apt-get install iptables python3-psutil python3-pip
    pip3 install flask flask_sock --break-system-packages

clone Organelle_OS and make organelle_m_deploy, restart

