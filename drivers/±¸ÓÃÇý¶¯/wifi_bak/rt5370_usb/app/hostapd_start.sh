ifconfig eth0 down
mkdir -p /var/run/hostapd
mv /dev/random /dev/random.bak
ln -s  /dev/urandom  /dev/random
insmod 8192cu.ko
ifconfig wlan0 172.16.44.144 up
./hostapd rtl_hostapd.conf -B
#./hostapd_cli  all_sta &
udhcpd -fS /etc/dhcpd/udhcpd.conf &

