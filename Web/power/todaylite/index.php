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

$numRowsToAggregate = 6;
$aggTimestamp = 0;
$aggWatts = 0;
$i = 1;

$result = mysql_query("SELECT unix_timestamp(timestamp),watts FROM electricity WHERE (date(timestamp) = curdate())") or die(mysql_error());
while($row = mysql_fetch_array($result))
{
  if ($i % $numRowsToAggregate) 
  {
    //$aggTimestamp = $aggTimeStamp + $row['unix_timestamp(timestamp)'];
    $aggTimestamp = $row['unix_timestamp(timestamp)'];
    $aggWatts = $aggWatts + $row['watts'];
    $i++;
  } else
  {
    $aggWatts = $aggWatts / ($numRowsToAggregate - 1);
    //$aggTimestamp = $aggTimestamp / ($numRowsToAggregate - 1);
    echo "<nextrow>";
    echo "<timestamp>";
    echo $aggTimestamp;
    echo "</timestamp>";
    echo "<watts>";
    echo intval($aggWatts); 
    echo "</watts>";
    echo "</nextrow>";
    $aggTimestamp = 0;
    $aggWatts = 0;
    $i = 1;
  }
}

mysql_close($con);
?>
</result>
