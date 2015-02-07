<?php
$con = mysql_connect("localhost","cc");
if (!$con)
{
  die('Could not connect: ' . mysql_error());
}

mysql_select_db("mydata", $con);

$queryString = "SELECT * FROM gas WHERE curdate() < timestamp AND timestamp > timestampadd(SECOND, -20, now())";

$result = mysql_query($queryString) or die(mysql_error());

if ($row = mysql_fetch_array($result)) {
  echo "On";
} else {
  echo "Off";
}

mysql_close($con);
?>
