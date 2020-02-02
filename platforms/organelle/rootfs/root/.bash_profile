#
# ~/.bash_profile
#

[[ -f ~/.bashrc ]] && . ~/.bashrc
# to startx in read only root fs
export XAUTHORITY=/var/tmp/.Xauthority_$USER

export FW_DIR=/root/fw_dir

ps cax | grep mother > /dev/null
if [ $? -eq 0 ]; then
    echo "Welcome to Organelle."
else
    $FW_DIR/scripts/setup.sh > /dev/null 2>&1
    $FW_DIR/scripts/start-mother.sh > /dev/null 2>&1
    $FW_DIR/root/scripts/welcome.sh
fi
