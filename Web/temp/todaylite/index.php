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


$numRowsToAggregate = 5;
$timestamp1 = 0;
$timestamp2 = 0;
$aggTemp = 0;
$i = 0;

$result = mysql_query("SELECT unix_timestamp(timestamp),temperature FROM temperature3 WHERE timestamp between date_sub(curdate(), INTERVAL " . $daysOld . " DAY) AND date_sub(curdate(), INTERVAL " . ($daysOld - 1) . " DAY) order by timestamp")or die(mysql_error());

while($row = mysql_fetch_array($result))
{
  if (($i + 1) % ($numRowsToAggregate + 1))
  {
    if ($i == 0)
    {
      $timestamp1 = $row['unix_timestamp(timestamp)'];
    }

    $aggTemp = $aggTemp + $row['temperature'];
    $i++;
  } else
  {
    $aggTemp = $aggTemp / ($numRowsToAggregate);
    $timestamp2 = $row['unix_timestamp(timestamp)'];
    echo "<nextrow>";
    echo "<timestamp>";
    echo intval(($timestamp1 + $timestamp2) / 2);
    echo "</timestamp>";
    echo "<temperature>";
    echo $aggTemp;
    echo "</temperature>";
    echo "</nextrow>";
    $timestamp1 = 0;
    $timestamp2 = 0;
    $aggTemp = 0;
    $i = 0;
  }
}

mysql_close($con);
?>
</result>
