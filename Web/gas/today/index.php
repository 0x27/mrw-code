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

# Construct a large & ugly union query
$queryString = "select count(tick),unix_timestamp(timestampadd(HOUR, 1, date(curdate()))) from gas where timestamp > curdate() AND hour(timestamp) = hour(timestampadd(HOUR,0,date(timestamp)))";
$queryString = $queryString . " UNION ALL select count(tick),unix_timestamp(timestampadd(HOUR, 2, date(curdate()))) from gas where timestamp > curdate() AND hour(timestamp) = hour(timestampadd(HOUR,1,date(timestamp)))";
$queryString = $queryString . " UNION ALL select count(tick),unix_timestamp(timestampadd(HOUR, 3, date(curdate()))) from gas where timestamp > curdate() AND hour(timestamp) = hour(timestampadd(HOUR,2,date(timestamp)))";
$queryString = $queryString . " UNION ALL select count(tick),unix_timestamp(timestampadd(HOUR, 4, date(curdate()))) from gas where timestamp > curdate() AND hour(timestamp) = hour(timestampadd(HOUR,3,date(timestamp)))";
$queryString = $queryString . " UNION ALL select count(tick),unix_timestamp(timestampadd(HOUR, 5, date(curdate()))) from gas where timestamp > curdate() AND hour(timestamp) = hour(timestampadd(HOUR,4,date(timestamp)))";
$queryString = $queryString . " UNION ALL select count(tick),unix_timestamp(timestampadd(HOUR, 6, date(curdate()))) from gas where timestamp > curdate() AND hour(timestamp) = hour(timestampadd(HOUR,5,date(timestamp)))";
$queryString = $queryString . " UNION ALL select count(tick),unix_timestamp(timestampadd(HOUR, 7, date(curdate()))) from gas where timestamp > curdate() AND hour(timestamp) = hour(timestampadd(HOUR,6,date(timestamp)))";
$queryString = $queryString . " UNION ALL select count(tick),unix_timestamp(timestampadd(HOUR, 8, date(curdate()))) from gas where timestamp > curdate() AND hour(timestamp) = hour(timestampadd(HOUR,7,date(timestamp)))";
$queryString = $queryString . " UNION ALL select count(tick),unix_timestamp(timestampadd(HOUR, 9, date(curdate()))) from gas where timestamp > curdate() AND hour(timestamp) = hour(timestampadd(HOUR,8,date(timestamp)))";
$queryString = $queryString . " UNION ALL select count(tick),unix_timestamp(timestampadd(HOUR, 10, date(curdate()))) from gas where timestamp > curdate() AND hour(timestamp) = hour(timestampadd(HOUR,9,date(timestamp)))";
$queryString = $queryString . " UNION ALL select count(tick),unix_timestamp(timestampadd(HOUR, 11, date(curdate()))) from gas where timestamp > curdate() AND hour(timestamp) = hour(timestampadd(HOUR,10,date(timestamp)))";
$queryString = $queryString . " UNION ALL select count(tick),unix_timestamp(timestampadd(HOUR, 12, date(curdate()))) from gas where timestamp > curdate() AND hour(timestamp) = hour(timestampadd(HOUR,11,date(timestamp)))";
$queryString = $queryString . " UNION ALL select count(tick),unix_timestamp(timestampadd(HOUR, 13, date(curdate()))) from gas where timestamp > curdate() AND hour(timestamp) = hour(timestampadd(HOUR,12,date(timestamp)))";
$queryString = $queryString . " UNION ALL select count(tick),unix_timestamp(timestampadd(HOUR, 14, date(curdate()))) from gas where timestamp > curdate() AND hour(timestamp) = hour(timestampadd(HOUR,13,date(timestamp)))";
$queryString = $queryString . " UNION ALL select count(tick),unix_timestamp(timestampadd(HOUR, 15, date(curdate()))) from gas where timestamp > curdate() AND hour(timestamp) = hour(timestampadd(HOUR,14,date(timestamp)))";
$queryString = $queryString . " UNION ALL select count(tick),unix_timestamp(timestampadd(HOUR, 16, date(curdate()))) from gas where timestamp > curdate() AND hour(timestamp) = hour(timestampadd(HOUR,15,date(timestamp)))";
$queryString = $queryString . " UNION ALL select count(tick),unix_timestamp(timestampadd(HOUR, 17, date(curdate()))) from gas where timestamp > curdate() AND hour(timestamp) = hour(timestampadd(HOUR,16,date(timestamp)))";
$queryString = $queryString . " UNION ALL select count(tick),unix_timestamp(timestampadd(HOUR, 18, date(curdate()))) from gas where timestamp > curdate() AND hour(timestamp) = hour(timestampadd(HOUR,17,date(timestamp)))";
$queryString = $queryString . " UNION ALL select count(tick),unix_timestamp(timestampadd(HOUR, 19, date(curdate()))) from gas where timestamp > curdate() AND hour(timestamp) = hour(timestampadd(HOUR,18,date(timestamp)))";
$queryString = $queryString . " UNION ALL select count(tick),unix_timestamp(timestampadd(HOUR, 20, date(curdate()))) from gas where timestamp > curdate() AND hour(timestamp) = hour(timestampadd(HOUR,19,date(timestamp)))";
$queryString = $queryString . " UNION ALL select count(tick),unix_timestamp(timestampadd(HOUR, 21, date(curdate()))) from gas where timestamp > curdate() AND hour(timestamp) = hour(timestampadd(HOUR,20,date(timestamp)))";
$queryString = $queryString . " UNION ALL select count(tick),unix_timestamp(timestampadd(HOUR, 22, date(curdate()))) from gas where timestamp > curdate() AND hour(timestamp) = hour(timestampadd(HOUR,21,date(timestamp)))";
$queryString = $queryString . " UNION ALL select count(tick),unix_timestamp(timestampadd(HOUR, 23, date(curdate()))) from gas where timestamp > curdate() AND hour(timestamp) = hour(timestampadd(HOUR,22,date(timestamp)))";
$queryString = $queryString . " UNION ALL select count(tick),unix_timestamp(timestampadd(HOUR, 24, date(curdate()))) from gas where timestamp > curdate() AND hour(timestamp) = hour(timestampadd(HOUR,23,date(timestamp)))";

$result = mysql_query($queryString) or die(mysql_error());
while($row = mysql_fetch_array($result))
{
  if (strlen($row[1]) > 0)
  {
    echo "<nextrow>";
    echo "<hour>";
    echo $row[1];
    echo "</hour>";
    echo "<total_ticks>";
    echo $row['count(tick)'];
    echo "</total_ticks>";
    echo "</nextrow>";
  }
}

mysql_close($con);
?>
</result>
