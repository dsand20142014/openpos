#./wpa_supplicant -iwlan0 -Dwext -c wpa_0_8.conf -dd &
#./wps_cli
#./hostapd rtl_hostapd.conf -B
#insmod 8192cu.ko
#sleep 1
#ifconfig eth0 down
#sleep 1
iwconfig 

ifconfig ra0 172.16.3.103  up
sleep 1
wpa_supplicant -B -Dwext -ira0  -c /wifi/sandjk/wpa_yfii.conf 
sleep 1
wpa_cli -p/var/run/wpa_supplicant scan
sleep 1
wpa_cli -p/var/run/wpa_supplicant scan_results
sleep 1
wpa_cli -p/var/run/wpa_supplicant remove_network 0
sleep 1
wpa_cli -p/var/run/wpa_supplicant ap_scan 1
sleep 1
wpa_cli -p/var/run/wpa_supplicant set_network 0 key_mgmt NONE
sleep 1
wpa_cli -p/var/run/wpa_supplicant set_network 0 ssid '"YF_II"'
sleep 1
wpa_cli -p/var/run/wpa_supplicant select_network 0
sleep 10
ifconfig eth0 down
ifconfig wlan0 172.16.3.103 up
iwconfig

#udhcpc -i eth1 -n




