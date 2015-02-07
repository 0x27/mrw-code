#!/bin/sh

# Send an email to my own phone via twitter...
wget "http://home:home@localhost/ppt/misc/dm.php?message=$1" -O /dev/null >/dev/null 2>&1

