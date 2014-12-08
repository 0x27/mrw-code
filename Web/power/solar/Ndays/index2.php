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

$queryString = "SELECT unix_timestamp(timestamp),watts FROM solar WHERE timestamp > timestampadd(DAY,-55,curdate()) and timestamp < timestampadd(DAY,-40,curdate()) ORDER BY timestamp";

$result = mysql_query($queryString) or die(mysql_error());

while($row = mysql_fetch_array($result))
{
  if (strlen($row['watts']) > 0)
  {
    echo "<nextrow>";
    echo "<timestamp>";
    echo $row['unix_timestamp(timestamp)'];
    echo "</timestamp>";
    echo "<watts>";
    echo $row['watts'];
    echo "</watts>";
    echo "</nextrow>";
  }
}

mysql_close($con);
?>
</result>
