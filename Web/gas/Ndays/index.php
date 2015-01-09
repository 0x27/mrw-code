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
$numberOfDaysHistory = 20;
$specifiedNumberOfDaysHistory = $_GET['numberofdayshistory'];

if (strlen($specifiedNumberOfDaysHistory) > 0)
{
   $numberOfDaysHistory = $specifiedNumberOfDaysHistory;
}

for ($i = 0; $i < $numberOfDaysHistory; $i++)
{
   if ($i == 0)
   {
      $queryString = "SELECT unix_timestamp(curdate()),count(tick) FROM gas WHERE timestamp > curdate()";
   } else
   {
      $queryString = $queryString." UNION ALL SELECT unix_timestamp(timestampadd(DAY,-".$i.",curdate())),count(tick) FROM gas WHERE timestamp > timestampadd(DAY,-".$i.",curdate()) and timestamp < timestampadd(DAY,-".($i-1).",curdate())";
   }
}

$result = mysql_query($queryString) or die(mysql_error());
while($row = mysql_fetch_array($result))
{
  if (strlen($row[0]) > 0)
  {
    echo "<nextrow>";
    echo "<timestamp_of_day>";
    echo $row[0];
    echo "</timestamp_of_day>";
    echo "<total_gas_m3>";
    echo $row['count(tick)'] * 0.01;
    echo "</total_gas_m3>";
    echo "</nextrow>";
  }
}

mysql_close($con);
?>
</result>
