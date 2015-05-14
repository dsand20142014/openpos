ifconfig wlan0 down
pkill wpa_supplicant
pkill udhcpc
echo 0 > /sys/class/leds/cpldwifi/brightness

