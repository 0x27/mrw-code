#!/bin/bash
cd /home/mwhitehead/iTunesMedia/Music
date=`date`
echo $date >>/tmp/ezstream.log
echo Sleeping to let icecast start...>>/tmp/ezstream.log
sleep 200
nohup ezstream -c /home/mwhitehead/ezstream_conf.xml &>>/tmp/ezstream.log &

