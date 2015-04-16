<?php header('Content-type: application/xml'); ?>
<?php echo("<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n"); ?>
<result>

<?php
$con = mysql_connect("localhost","lighttpd");
if (!$con)
{
  die('Could not connect: ' . mysql_error());
}

mysql_select_db("mydata", $con);

$i = 0;
$queryString = "";
$numberOfMonthsHistory = 12;
$specifiedNumberOfMonthsHistory = $_GET['numberofmonthshistory'];

if (strlen($specifiedNumberOfMonthsHistory) > 0)
{
   $numberOfMonthsHistory = $specifiedNumberOfMonthsHistory;
}

for ($i = 0; $i < $numberOfMonthsHistory; $i++)
{
   if ($i == 0)
   {
      $queryString = "SELECT unix_timestamp(date(timestamp)),avg(temperature) FROM temperature WHERE (month(timestamp) = month(curdate()))";
   } else
   {
      $queryString = $queryString." UNION SELECT unix_timestamp(date(timestamp)),avg(temperature) FROM temperature WHERE (month(timestamp) = month(curdate())-".$i.")";
   }
}

$result = mysql_query($queryString) or die(mysql_error());

//$result = mysql_query("SELECT unix_timestamp(date(timestamp)),avg(temperature) FROM temperature WHERE (date(timestamp) = curdate()) UNION SELECT unix_timestamp(date(timestamp)),avg(temperature) FROM temperature WHERE (date(timestamp) = curdate()-1) UNION SELECT unix_timestamp(date(timestamp)),avg(temperature) FROM temperature WHERE (date(timestamp) = curdate()-2) UNION SELECT unix_timestamp(date(timestamp)),avg(temperature) FROM temperature WHERE (date(timestamp) = curdate()-3) UNION SELECT unix_timestamp(date(timestamp)),avg(temperature) FROM temperature WHERE (date(timestamp) = curdate()-4)") or die(mysql_error());
while($row = mysql_fetch_array($result))
{
  if (strlen($row['avg(temperature)']) > 0)
  {
    echo "<nextrow>";
    echo "<timestamp_of_month>";
    echo $row['unix_timestamp(date(timestamp))'];
    echo "</timestamp_of_month>";
    echo "<average_temperature>";
    echo $row['avg(temperature)'];
    echo "</average_temperature>";
    echo "</nextrow>";
  }
}

mysql_close($con);
?>
</result>
