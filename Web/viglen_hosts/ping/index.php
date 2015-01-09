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

$viglenName = $_GET["name"];
$remoteIP = substr($_SERVER["REMOTE_ADDR"], 7);
echo $viglenName;
echo $remoteIP;
$queryString = "";

$queryString = "SELECT name,ip FROM viglen_hosts WHERE name='".$viglenName."'";

$result = mysql_query($queryString) or die(mysql_error());

if (mysql_num_rows($result) == 0) 
{
   echo "inserted";
   $queryString = "INSERT into viglen_hosts (name, ip) values ('".$viglenName."', '".$remoteIP."')";
   $result = mysql_query($queryString) or die(mysql_error());
} else {
   echo "updated";
   
   // Do a dummy update - this forces the timestamp of the row in the database to update
   $queryString = "UPDATE viglen_hosts set ip = '1234' where name = '".$viglenName."'";
   $result = mysql_query($queryString) or die(mysql_error());
   $queryString = "UPDATE viglen_hosts set ip = '".$remoteIP."' where name = '".$viglenName."'";
   $result = mysql_query($queryString) or die(mysql_error());
}

mysql_close($con);
?>
</result>
