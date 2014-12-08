<?php header('Content-type: application/xml'); ?>
<?php echo("<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n"); ?>
<?php
echo "<entries>";
$result=shell_exec("cat /temp/memmonitor.log | grep Swap | awk 'NR % 80 == 0 {print $4}' | sed s'/^/<element>/' | sed s'/.$/<\/element>/'");
echo $result;
echo "</entries>";
?>
