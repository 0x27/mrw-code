#!/bin/bash

xml=$(curl -s -X GET http://192.168.0.8:8080/EventMgmt/EventTable)
url=$(cat testurl.txt)

echo "Comparing the following:"
echo "$xml"
echo "$url"

if [[ "$xml" == *"$url"* ]]; then
   echo "The xml contains the url"
else
   echo "The URL isn't present"
fi

