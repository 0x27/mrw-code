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

$i = 0;
$queryString = "";

#$queryString = "SELECT unix_timestamp(timestamp),voltage FROM wirelessnodes";
$queryString = "SELECT unix_timestamp(timestamp),voltage FROM wirelessnodes WHERE nodeid = 3 ORDER BY timestamp";

$result = mysql_query($queryString) or die(mysql_error());

$firstValue = 1;
$lastValue = "";
$lastTimestamp = "";

while($row = mysql_fetch_array($result))
{
  if (strlen($row['voltage']) > 0)
  {
    if ($row['voltage'] != $lastValue) {

      if (!$firstValue) {
        # First, print the previous value
        echo "<nextrow>";
        echo "<timestamp>";
        echo $lastTimestamp;
        echo "</timestamp>";
        echo "<voltage>";
        echo $lastValue / 10;
        echo "</voltage>";
        echo "</nextrow>";
      }
      
      # Then print the new value
      echo "<nextrow>";
      echo "<timestamp>";
      echo $row['unix_timestamp(timestamp)'];
      echo "</timestamp>";
      echo "<voltage>";
      echo $row['voltage'] / 10;
      echo "</voltage>";
      echo "</nextrow>";
    }
  }
  $lastValue = $row['voltage'];
  $lastTimestamp = $row['unix_timestamp(timestamp)'];
  $firstValue = 0;
}

mysql_close($con);
?>
</result>
