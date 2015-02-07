<?php
echo("<html><head><title>TAP Pending Email</title></head><body><p><font face=arial size=4>");
$result=shell_exec('perl ../apps/mqtt-perl/getpendingtapmail.pl');

// Replace opening [ brackets with <br/>[ so each entry is on a new line
$result=str_replace('[', '<br/>[', $result);

echo($result);
echo("</font></p></body></html>");
?>
