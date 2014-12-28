#!/bin/bash
killall ezstream
cd /home/mwhitehead/iTunesMedia/Music
ezstream -c /home/mwhitehead/ezstream_conf.xml 1>&2 2>/tmp/ezstream.log

