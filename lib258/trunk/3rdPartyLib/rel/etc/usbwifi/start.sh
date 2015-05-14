echo 1 > /sys/class/leds/cpldwifi/brightness
sleep 1
wpa_supplicant -B -Dwext -iwlan0 -c /etc/usbwifi/wpa_supplicant.conf
udhcpc -S -q -b -i wlan0 -s /etc/usbwifi/dhcpc.script
/etc/usbwifi/reroute
route del default wlan0