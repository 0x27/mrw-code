#!/bin/bash
xml="`curl -s -X GET http://192.168.0.8:8080/EventMgmt/EventTable`"

echo "$xml"

