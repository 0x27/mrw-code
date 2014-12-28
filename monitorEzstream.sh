#!/bin/sh

# Example playlist script that has the behavior required by ezstream.
date=`date`
echo $date
echo Sleeping to let icecast and ezstream start
#sleep 500
#sleep 10
echo Starting EZStream monitor

ezstreamPid=

while [ -z "$ezstreamPid" ]
do
  ezstreamPid=`ps -ef | grep "ezstream -c" | grep -v "grep" | awk 'BEGIN {FS=" "}{print $2}'`
  echo ezstream PID is $ezstreamPid
  sleep 60
done

previousTrack=""

# Loop and check what's playing every 6 (arbitrary number) seconds
while true; do 
  sleep 5

  currentTrack=`mpc current --format %file%`
  if [ "$previousTrack" != "$currentTrack" ]
    then
      previousTrack=$currentTrack
      if [ -z "$currentTrack" ]
        then
          echo Doing nothing
           echo DO NOTHING >/dev/null
        else
          echo Skipping
          kill -10 $ezstreamPid 
      fi
  fi
done

