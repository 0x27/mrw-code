<?php header('Content-type: application/xml'); ?>
<?php echo("<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n"); ?>
<result>
<?php
$music=shell_exec('mpc');
$parts=explode("\n", $music);
$playing=strpos($parts[0], "volume:");

if ( !($playing === false) )
{
  // Do nothing
}
else
{
?>
<state><?php
$state = explode(" ", $parts[1]);
$state = $state[0];
echo trim(htmlspecialchars(substr($state, 1, strlen($state) - 2)));
?>
</state>
<position>
<?php
  $tune=explode(" ", $parts[1]);

  // Paused gives a slightly different output
  if ($tune[1] == "")
  {
    $tune=explode("/", $tune[2]);
  }
  else
  {
    $tune=explode("/", $tune[1]);
  }
  $position=substr($tune[0], 1);
  echo trim(htmlspecialchars($position));
?>
</position>
<timepercent>
<?php
  $tune=explode("(", $parts[1]);

  $tune=explode("%", $tune[1]);

  echo trim(htmlspecialchars($tune[0]));
?>
</timepercent>
<artist>
<?php
  $tune=explode(" - ", $parts[0]); 
  echo $tune[0]."\n";
?>
</artist>
<track>
<?php
echo $tune[1]."\n";
?>
</track>
<volume>
<?php
$volume=explode(":", $parts[2]);
$volume=explode("%", $volume[1]);
echo $volume[0]."\n";
?>
</volume>
<random>
<?php
$random=explode(":", $parts[2]);
$random=explode(" ", $random[3]);
echo $random[1]."\n";
?>
</random>
<?php
}
?>
</result>

