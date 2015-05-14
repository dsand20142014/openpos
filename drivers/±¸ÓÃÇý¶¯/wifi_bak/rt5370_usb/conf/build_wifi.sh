mkdir -p   /etc/Wireless/RT2870STA/RT2870STA.dat
\cp -rvf RT2870STA.dat  /etc/Wireless/RT2870STA/

mkdir -p  /var/run/wpa_supplicant
mkdir -p  /etc/wifi

insmod  rt5370sta.ko
ifconfig ra0 172.16.3.103 netmask 255.255.0.0 up

\cp -rvf  wpa_rt5370.conf /etc/wifi/
wpa_supplicant  -B -Dwext -ira0 -c /etc/wifi/wpa_rt5370.conf

ifconfig ra0 172.16.3.103 netmask 255.255.0.0 up
