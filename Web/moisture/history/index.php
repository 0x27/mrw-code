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

$result = mysql_query("SELECT timestamp,unix_timestamp(timestamp),moisture FROM moisture WHERE timestamp > timestampadd(DAY,-170,curdate()) and timestamp < timestampadd(DAY, -140, curdate()) order by timestamp") or die(mysql_error());
while($row = mysql_fetch_array($result))
{
  if ($row['moisture'] < 1000) {
    echo "<nextrow>";
    echo "<timestamp>";
    echo $row['unix_timestamp(timestamp)'];
    echo "</timestamp>";
    echo "<moisture>";
    echo $row['moisture'];
    echo "</moisture>";
    echo "</nextrow>";
  }
}

mysql_close($con);
?>
</result>
