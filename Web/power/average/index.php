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

for ($i = 0; $i < $numberOfDaysHistory; $i++)
{
   if ($i == 0)
   {
      $queryString = "SELECT unix_timestamp(date(timestamp)),avg(watts) FROM electricity WHERE (date(timestamp) = curdate())";
   } else
   {
      $queryString = $queryString." UNION SELECT unix_timestamp(date(timestamp)),avg(watts) FROM electricity WHERE (date(timestamp) = date_sub(curdate(), interval ".$i." day))";
   }
}

$result = mysql_query($queryString) or die(mysql_error());

while($row = mysql_fetch_array($result))
{
  if (strlen($row['avg(watts)']) > 0)
  {
    echo "<nextrow>";
    echo "<timestamp_of_day>";
    echo $row['unix_timestamp(date(timestamp))'];
    echo "</timestamp_of_day>";
    echo "<average_watts>";
    echo $row['avg(watts)'];
    echo "</average_watts>";
    echo "</nextrow>";
  }
}

mysql_close($con);
?>
</result>
