#!/bin/sh

# Example playlist script that has the behavior required by ezstream.

currentTrack=`mpc current --format %file%`
if [ -z "$currentTrack" ]
  then
    echo /home/mwhitehead/ambienttrack.mp3
  else
    echo $currentTrack
fi

