##Automated upload the test scripts using a Raspberry Pi with an RFM12Pi##

To make script run at startup add the following to /etc/rc.local 

printf "Attempting to pull latest firmware from GitHub/n"
sleep 5
cd /home/pi/emonTxFirmware
git pull
sleep 1
cd /home/pi

python /home/pi/scriptname.py)

