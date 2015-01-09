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

$queryString = "";
      
$queryString = "SELECT avg(watts) FROM electricity WHERE (date(timestamp) = curdate() AND time(timestamp) > subtime(curtime(), '00:10:00'))";

$result = mysql_query($queryString) or die(mysql_error());

while($row = mysql_fetch_array($result))
{
  if (strlen($row['avg(watts)']) > 0)
  {
    echo "<nextrow>";
    echo "<average_watts>";
    echo $row['avg(watts)'];
    echo "</average_watts>";
    echo "</nextrow>";
  }
}

mysql_close($con);
?>
</result>
