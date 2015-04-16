<?php header('Content-type: application/xml'); ?>
<?php echo("<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n"); ?>
<result>

<?php
$con = mysql_connect("localhost","cc");
if (!$con)
  {
  die('Could not connect: ' . mysql_error());
  }

mysql_select_db("mydata", $con);

$daysOld = 0;
$specifiedDaysOld = $_GET['daysold'];

if (strlen($specifiedDaysOld) > 0)
{
   $daysOld = $specifiedDaysOld;
}

$result = mysql_query("SELECT unix_timestamp(timestamp),temperature FROM temperature4 WHERE timestamp between date_sub(curdate(), INTERVAL " . $daysOld . " DAY) AND date_sub(curdate(), INTERVAL " . ($daysOld - 1) . " DAY) order by timestamp")or die(mysql_error());

while($row = mysql_fetch_array($result))
  {
  echo "<nextrow>";
  echo "<timestamp>";
  echo $row['unix_timestamp(timestamp)'];
  echo "</timestamp>";
  echo "<data>";
  echo $row['temperature'] + 100;
  echo "</data>";
  echo "</nextrow>";
  }

mysql_close($con);
?>
</result>
