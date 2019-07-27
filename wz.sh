#!/bin/bash
PIDFILE=$1
mkdir -p /home/pi/wodomierze/logs
while [ 1=1 ]
do
#exec /home/pi/CO/co >>/home/pi/logs/co.log 2>&1 </dev/null
/home/pi/wodomierze/WZ/wz_new >>/home/pi/wodomierze/logs/wz_new.log 2>&1 </dev/null
/home/pi/wodomierze/WZ/usbreset /dev/bus/usb/001/004
#CHILD=$!
#echo $CHILD > $PIDFILE
done
