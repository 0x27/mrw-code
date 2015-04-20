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

$result = mysql_query("SELECT unix_timestamp(timestamp),humidity FROM humidity5 WHERE (date(timestamp) = curdate()) order by timestamp") or die(mysql_error());
while($row = mysql_fetch_array($result))
  {
  echo "<nextrow>";
  echo "<timestamp>";
  echo $row['unix_timestamp(timestamp)'];
  echo "</timestamp>";
  echo "<humidity>";
  echo $row['humidity'];
  echo "</humidity>";
  echo "</nextrow>";
  }

mysql_close($con);
?>
</result>
