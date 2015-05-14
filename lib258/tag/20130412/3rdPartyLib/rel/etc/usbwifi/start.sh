echo 1 > /sys/class/leds/cpldwifi/brightness
sleep 1
wpa_supplicant -B -Dwext -iwlan0 -c /etc/usbwifi/wpa_supplicant.conf
udhcpc -S -i wlan0 -s /etc/usbwifi/dhcpc.script &
