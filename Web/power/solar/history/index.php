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

$queryString = "SELECT unix_timestamp(date(timestamp)),totalsolarwatthrs FROM daily_stats WHERE (year(timestamp) = year(curdate()))";

$result = mysql_query($queryString) or die(mysql_error());

while($row = mysql_fetch_array($result))
{
  if (strlen($row['totalsolarwatthrs']) > 0)
  {
echo "<nextrow>";
echo "<next_timestamp>";
echo $row['unix_timestamp(date(timestamp))'];
echo "</next_timestamp>";
echo "<next_solar_watt_hours>";
echo $row['totalsolarwatthrs'];
echo "</next_solar_watt_hours>";
echo "</nextrow>";
  }
}

mysql_close($con);
?>
</result>
