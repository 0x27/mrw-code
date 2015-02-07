<?php header('Content-type: application/xml'); ?>
<?php echo("<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n"); ?>
<result>

<?php
$con = mysql_connect("localhost","cc");

$numberOfHoursFromMidnight = $_GET['numhoursfrommidnight'];
$daysOld = $_GET['daysold'];

if (strlen($numberOfHoursFromMidnight) == 0)
{
  $numberOfHoursFromMidnight = 24;
}

if (strlen($daysOld) == 0)
{
  $daysOld = 0;
}

if (!$con)
{
  die('Could not connect: ' . mysql_error());
}

mysql_select_db("mydata", $con);

if ($daysOld == 0) {
  $result = mysql_query("SELECT unix_timestamp(timestampadd(HOUR, 1, timestamp)),watts FROM electricity WHERE (timestamp >= curdate()) AND HOUR(timestamp) < " . $numberOfHoursFromMidnight) or die(mysql_error());
} else {
  $result = mysql_query("SELECT unix_timestamp(timestampadd(HOUR, 1, timestamp)),watts FROM electricity WHERE (timestamp >= date_add(curdate(), interval -" . $daysOld . " day)) AND (timestamp < curdate()) AND HOUR(timestamp) < " . $numberOfHoursFromMidnight) or die(mysql_error());
}

while($row = mysql_fetch_array($result))
{
  echo "<nextrow>";
  echo "<timestamp>";
  echo $row['unix_timestamp(timestampadd(HOUR, 1, timestamp))'];
  echo "</timestamp>";
  echo "<watts>";
  echo $row['watts'];
  echo "</watts>";
  echo "</nextrow>";
}

mysql_close($con);
?>
</result>
