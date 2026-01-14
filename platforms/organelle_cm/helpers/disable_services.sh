#!/bin/bash
# disable-unwanted-services.sh
# This script disables and masks services that are not needed for the EYESY device.
# Masking prevents any accidental or dependency-based activation.

# List of unwanted services:
SERVICES=(
    "apparmor.service"
    "rpi-eeprom-update.service"
    "ModemManager.service"
    "console-setup.service"
    "keyboard-setup.service"
    "rpi-display-backlight.service"
    "glamor-test.service"
    "cron.service"
    "dphys-swapfile.service"
    "rp1-test.service"
    "NetworkManager-wait-online.service"
    "e2scrub_reap.service"
    "raspi-config.service"
    "apt-daily-upgrade.service"
    "systemd-random-seed.service"
    "systemd-journal-flush"
    "avahi-daemon"
    "apt-daily.timer"
    "fbrestore"
    "bluetooth"
    "hciuart"
    #"ssh.service"
)

# Loop through each service to disable and mask it.
for SERVICE in "${SERVICES[@]}"; do
    echo "Disabling ${SERVICE}..."
    sudo systemctl disable "${SERVICE}" || echo "Failed to disable ${SERVICE}"
    
    echo "Stopping ${SERVICE} if running..."
    sudo systemctl stop "${SERVICE}" || echo "${SERVICE} was not running"
    
    echo "Masking ${SERVICE}..."
    sudo systemctl mask "${SERVICE}" || echo "Failed to mask ${SERVICE}"
    
    echo "--------------------------------------"
done

echo "All unwanted services have been disabled and masked."

# Disable additional unnecessary systemd services and timers for Organelle audio performance

echo "Disabling filesystem maintenance services..."
sudo systemctl disable e2scrub_all.timer
sudo systemctl disable e2scrub_all.service
sudo systemctl disable fstrim.timer
sudo systemctl disable fstrim.service

echo "Disabling and masking fsck daemon..."
sudo systemctl disable --now systemd-fsckd.service
sudo systemctl mask systemd-fsckd.service

echo "Disabling and masking hostname daemon..."
sudo systemctl disable systemd-hostnamed.service
sudo systemctl mask systemd-hostnamed.service

echo "Disabling log and maintenance timers..."
sudo systemctl disable logrotate.timer
sudo systemctl disable systemd-tmpfiles-clean.timer
sudo systemctl disable dpkg-db-backup.timer

echo "Disabling CUPS printing services..."
sudo systemctl stop cups
sudo systemctl disable cups
sudo systemctl stop cups-browsed
sudo systemctl disable cups-browsed

echo "Disabling triggerhappy..."
sudo systemctl stop triggerhappy
sudo systemctl disable triggerhappy

echo "Disabling time synchronization (optional)..."
sudo systemctl disable systemd-timesyncd

echo "Done! Reboot for changes to take full effect."
echo "To verify, run: systemctl list-timers"
