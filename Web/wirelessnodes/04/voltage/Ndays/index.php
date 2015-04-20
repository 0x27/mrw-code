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
$numberOfDaysHistory = 10;
$specifiedNumberOfDaysHistory = $_GET['numberofdayshistory'];

if (strlen($specifiedNumberOfDaysHistory) > 0)
{
   $numberOfDaysHistory = $specifiedNumberOfDaysHistory;
}

$queryString = "SELECT unix_timestamp(timestamp),voltage FROM wirelessnodes WHERE nodeid = 4 AND timestamp > date_sub(curdate(), INTERVAL " . ($numberOfDaysHistory + 2) . " DAY) ORDER BY timestamp";

$result = mysql_query($queryString) or die(mysql_error());
$nRows = 0;

while($row = mysql_fetch_array($result))
{
  $nRows++;
  if ($nRows == 6 && strlen($row['voltage']) > 0)
  {
    echo "<nextrow>";
    echo "<timestamp>";
    echo $row['unix_timestamp(timestamp)'];
    echo "</timestamp>";
    echo "<voltage>";
    echo $row['voltage'] / 10;
    echo "</voltage>";
    echo "</nextrow>";
    $nRows = 0;
  }
}

mysql_close($con);
?>
</result>
