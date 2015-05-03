#!/bin/sh
# This script is run by cron once a minute to make sure
# node-red is restarted if it crashes. ON A NEW SYSTEM
# PUT THE FOLLOWING LINE INTO /etc/crontab:
# * * * * *       root    /home/mwhitehead/mrw-code/cron/restart-node-red.sh 


PID=$(pidof node-red)
if [ "$PID" != "" ]; then
    echo "MRW: Node-Red already running so not starting." >>/tmp/nodered.log
    echo "MRW: Node-Red already running so not starting."
else
    echo "MRW: Node-Red isn't running. Restarting..." >>/tmp/nodered.log
    sleep 1
    /etc/init.d/start_nodered
fi

