#./wpa_supplicant -iwlan0 -Dwext -c wpa_0_8.conf -dd &
#./wps_cli
#./hostapd rtl_hostapd.conf -B
insmod 8192cu.ko
sleep 10
ifconfig eth0 down
sleep 10
iwconfig 
ifconfig wlan0 172.16.3.103 up
wpa_supplicant -B -Dwext -iwlan0  -c /usbwifi/wpa.conf 
sleep 10
wpa_cli -p/var/run/wpa_supplicant scan
sleep 10
wpa_cli -p/var/run/wpa_supplicant scan_results
sleep 10
wpa_cli -p/var/run/wpa_supplicant remove_network 0
sleep 10
wpa_cli -p/var/run/wpa_supplicant ap_scan 1
sleep 10
#wpa_cli -p/var/run/wpa_supplicant set_network 0 key_mgmt NONE
sleep 10
wpa_cli -p/var/run/wpa_supplicant set_network 0 ssid '"sandyf2"'
sleep 10
wpa_cli -p/var/run/wpa_supplicant set_network 0 wep_key0 66666666666666666666666666
sleep 10
wpa_cli -p/var/run/wpa_supplicant select_network 0
sleep 10
ifconfig eth0 down
ifconfig wlan0 172.16.3.103 up
iwconfig

#udhcpc -i eth1 -n




