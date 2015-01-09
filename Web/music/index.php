<?php header('Content-type: application/xml'); ?>
<?php echo("<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n"); ?>
<result>
<?php
$music=shell_exec('mpc playlist');
$parts=explode("\n", $music);
$size=count($parts);
$i=0;

while ($i < $size-1)
{
?>
<item>
<position>
<?php
  //$tune=explode(")", $parts[$i]);
  // $position=substr($tune[0], 1);
  //echo trim(htmlspecialchars($position));
  echo ($i + 1);
?>
</position>
<artist>
<?php
  $tune=explode("-", $parts[$i]); 
  $artist=$tune[0];
  echo trim(htmlspecialchars($artist));
?>
</artist>
<track>
<?php
//echo urlencode($tune[1])."\n";
echo trim(htmlspecialchars($tune[1]));
?>
</track>
</item>
<?php
$i++;
}
?>
</result>

