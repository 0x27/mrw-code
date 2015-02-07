#!/bin/bash

AVERAGE=`/home/mwhitehead/opt/querydb/query_last_10mins.sh`

#sendemail -t matthew1001@hotmail.com -f matthew1001@hotmail.com -u "Email from home" -s smtp.tiscali.co.uk -m "10 minute power check: $AVERAGE watts read from the database"

sendemail -t matthew1001@hotmail.com -f matthew1001@hotmail.com -u "Email from home" -s smtp.virginmedia.com:465 -xu matthew1001@virginmedia.com -o tls=yes -xp suka621o -m "10 minute power check: $AVERAGE watts read from the database"

if [ -n $AVERAGE ]
then
  if [ $AVERAGE -gt 500 ]
  then
  echo "hello"
  #sendemail -t matthew1001@hotmail.com -f matthew1001@hotmail.com -u "Email from home" -s smtp.virginmedia.com -m "Average power consumption in the last 10 minutes is too high: $AVERAGE watts"
  fi
fi
 
