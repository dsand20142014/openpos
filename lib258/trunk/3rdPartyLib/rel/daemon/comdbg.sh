#!/bin/sh
touch /daemon/debug
/sbin/getty -L ttymxc4 115200 vt100
rm -rf /daemon/debug