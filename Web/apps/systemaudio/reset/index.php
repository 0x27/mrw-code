<?php
shell_exec('amixer set PCM 13 >>/tmp/mpd.log 2>>/tmp/mpd.log');
shell_exec('amixer set PCM unmute >>/tmp/mpd.log 2>>/tmp/mpd.log');
shell_exec('amixer set Master 100% >>/tmp/mpd.log 2>>/tmp/mpd.log');
shell_exec('amixer set Master unmute >>/tmp/mpd.log 2>>/tmp/mpd.log');
?>
