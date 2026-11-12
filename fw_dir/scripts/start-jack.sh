# Use the first ALSA card that has a playback PCM rather than assuming hw:0.
# USB MIDI controllers (e.g. Arturia KeyStep) register as ALSA cards with no PCM,
# and if connected at boot they can take card 0, which makes jackd fail to start.
CARD=$(awk -F'[-:]' '/playback/ {print $1+0; exit}' /proc/asound/pcm 2>/dev/null)
CARD=${CARD:-0}
jackd -R -P90 -p16 -t200 -d alsa -dhw:$CARD -p 128 -r 44100 -s -i2 -o2 & echo $! > /tmp/pids/jack.pid
