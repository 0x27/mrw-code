<?php
$result=shell_exec("cat /proc/swaps | grep partition | awk '{print $4}'");
echo $result;
?>
