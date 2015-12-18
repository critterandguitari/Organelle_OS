#
# ~/.bash_profile
#

[[ -f ~/.bashrc ]] && . ~/.bashrc
# to startx in read only root fs
export XAUTHORITY=/var/tmp/.Xauthority_$USER
/root/scripts/setup.sh
/root/scripts/start-mother.sh
