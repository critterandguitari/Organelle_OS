# Organelle CM4 aka S2 

Configuring RPI CM4 disk image.

Start with 2025-05-13-raspios-bookworm-armhf-lite

Boot up, do initial config, make music user. Make second ext4 primary partition for "/sdcard" user storage.

Install basics:

    sudo apt-get update
    sudo apt-get install vim git

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

## ^OG4_20250618.img

sudo apt-get install iptables python3-psutil python3-pip

pip3 install flask flask_sock --break-system-packages

clean up

    sudo apt-get autoremove --purge

configure network manager for read only

update /etc/NetworkManager/NetworkManager.conf and add rc-manager=file under the [main] section

move files to tmp locations

    sudo mv /etc/resolv.conf /var/run/resolv.conf && sudo ln -s /var/run/resolv.conf /etc/resolv.conf
    sudo rm -rf /var/lib/dhcp && sudo ln -s /var/run /var/lib/dhcp
    sudo rm -rf /var/lib/NetworkManager && sudo ln -s /var/run /var/lib/NetworkManager

other changes for read only

    sudo systemctl mask man-db.timer
    sudo systemctl mask apt-daily.timer
    sudo systemctl mask apt-daily-upgrade.timer

add to etc/fstab

    tmpfs  /tmp      tmpfs  defaults,noatime,nosuid,nodev   0  0
    tmpfs  /var/tmp  tmpfs  defaults,noatime,nosuid,nodev   0  0
    tmpfs  /var/log  tmpfs  defaults,noatime,nosuid,nodev,noexec  0  0
    tmpfs  /var/spool/mail  tmpfs  defaults,noatime,nosuid,nodev,noexec,size=25m  0  0
    tmpfs  /var/lib/logrotate  tmpfs  defaults,noatime,nosuid,nodev,noexec,size=1m,mode=0755  0  0
    tmpfs  /var/lib/sudo  tmpfs  defaults,noatime,nosuid,nodev,noexec,size=1m,mode=0700  0  0

add ro to / and /boot entries

move NetworkManager connection files to /sdcard:

    sudo mv /etc/NetworkManager/system-connections /sdcard/system-connections
    sudo ln -s /sdcard/system-connections /etc/NetworkManager/system-connections
    sudo chown root:root /sdcard/system-connections

## ^OG4-base-ro-20250622.img

run resize.sh on the OG4-base-ro-20250622.img file to ensure partition doesn't exceed 7360 * 1024 * 1024 bytes. (that is bs=1m count=7360 for dd, so it fits on any "8" GB card)

pull and make deploy Organelle_OS

do deploy stuff

## ^OG4_v4.3_beta1.img

pull and make deploy Organelle_OS
install new patchesi

    sudo fw_dir/scripts/remount-rw.sh 
    cd Organelle_OS/
    git pull
    make clean
    sudo make organelle_4_deploy 
    rm /sdcard/pedal_cfg.sh 
    rm -r /sdcard/Patches
    cd /sdcard/
    wget https://github.com/critterandguitari/Organelle_Patches/archive/refs/tags/OSv4.3.zip
    unzip OSv4.3.zip 
    mv Organelle_Patches-OSv4.3 Patches
    rm OSv4.3.zip 

or :

sudo fw_dir/scripts/remount-rw.sh && cd Organelle_OS/ && git pull && make clean && sudo make organelle_4_deploy && rm /sdcard/pedal_cfg.sh && rm -r /sdcard/Patches && cd /sdcard/ && wget https://github.com/critterandguitari/Organelle_Patches/archive/refs/tags/OSv4.3.zip && unzip OSv4.3.zip && mv Organelle_Patches-OSv4.3 Patches && rm OSv4.3.zip

do deploy stuff

## ^OGS2_v4.3.img

update software, disable ssh

sudo fw_dir/scripts/remount-rw.sh && cd Organelle_OS/ && git pull && make clean && sudo make organelle_4_deploy && sudo systemctl disable ssh.service

do deploy stuff

## ^OGS2_v4.4.img

## deploy stuff

common stuff before making disk image

clean up

    sudo mount -o remount,rw /
    git config --global user.email "..."
    git config --global user.name "..."
    cd ~
    rm .viminfo
    cat /dev/null > ~/.bash_history && history -c

remove all /sdcard/system-connections except music network

run fsck from another machine. 
    
    sudo fsck /dev/mmcblk0p1
    sudo fsck /dev/mmcblk0p2
    sudo fsck /dev/mmcblk0p3
    sudo dd if=/dev/mmcblk0 of=OGS2_v4.3.img bs=1M count=7360
    zip OGS2_v4.3.img.zip OGS2_v4.3.img


