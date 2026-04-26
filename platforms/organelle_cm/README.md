# Organelle CM 3,4 OG S, M, S2

Configuring RPI disk image for CM3 and CM4 based devices.

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

    sudo fw_dir/scripts/remount-rw.sh
    sudo apt-get update
    sudo apt-get install faust chuck lua5.4 liblua5.4-dev libsdl2-dev realvnc-vnc-server
    cd Organelle_OS/
    make clean
    git pull
    cd platforms/organelle_cm/cm3_kernel/
    sudo ./install_from_archive.sh stuff2.tar.gz manifest.txt 
    ./verify_archive.sh stuff2.tar.gz 
    cd ../helpers
    sudo ./disable_services.sh
    cd ~/Organelle_OS
    make clean
    sudo make organelle_cm_deploy 


## ^OGSMS2_v5.0_beta2.img

set date time:

    sudo fw_dir/scripts/remount-rw.sh
    sudo timedatectl set-time "2026-03-17 12:01"

install patches:

    cd /sdcard/ 
    rm -fr Patches
    mkdir -p Patches/EXPLORE
    wget https://github.com/critterandguitari/Organelle_Patches/archive/refs/tags/OSv4.3.zip
    unzip OSv4.3.zip
    mv Organelle_Patches-OSv4.3/* Patches/EXPLORE/
    rm OSv4.3.zip
    rm -r Organelle_Patches-OSv4.3

install PLAY patches using web

pull changes 

    cd ~/Organelle_OS
    git pull
    sudo make clean
    sudo make organelle_cm_deploy 

update Faust:

    cd /tmp
    git clone https://github.com/grame-cncm/faustlibraries.git
    sudo cp faustlibraries/*.lib /usr/share/faust/

Reboot and install PLAY patches

adjust headphone level to 74 db and store:

    sudo alsactl store

turn off SSH:

    sudo systemctl disable ssh.service   

do deploy stuff

## ^OGSMS2_v5.0_beta3.img

install updated PLAY patches
do deploy stuff

## ^OGSMS2_v5.0.img

install PLAY patches using web

pull changes 

    cd ~/Organelle_OS
    git pull
    sudo make clean
    sudo make organelle_cm_deploy 

make startx wrapper exec

    sudo chmod +x /usr/local/bin/startx

do deploy stuff

## ^OGSMS2_v5.1.img

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

run fsck from another machine, make sure unmounted, zero free space and check. 
    
    sudo umount /dev/mmcblk0p1
    sudo umount /dev/mmcblk0p2
    sudo umount /dev/mmcblk0p3       
    sudo zerofree -v /dev/mmcblk0p2
    sudo zerofree -v /dev/mmcblk0p3
    sudo fsck /dev/mmcblk0p1
    sudo fsck /dev/mmcblk0p2
    sudo fsck /dev/mmcblk0p3
    sudo sync
    sudo dd if=/dev/mmcblk0 of=OGS2_v4.3.img bs=1M count=7360
    zip OGS2_v4.3.img.zip OGS2_v4.3.img


