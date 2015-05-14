ifconfig eth0 down
insmod 8192cu.ko
ifconfig wlan0 172.16.44.144 up
mkdir -p /var/run/wpa_supplicant

wpa_supplicant -B -Dwext -iwlan0  -c ./wpa_yf2.conf

