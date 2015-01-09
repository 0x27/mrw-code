<?php header('Content-type: application/xml'); ?>
<?php echo("<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n"); ?>
<result>

<?php
$con = mysql_connect("localhost","cc");
if (!$con)
{
  die('Could not connect: ' . mysql_error());
}

$comment = $_GET['comment'];

mysql_select_db("mydata", $con);

$result = mysql_query("INSERT into comments(comment) values('" . $comment . "')") or die (mysql_error());

mysql_close($con);
?>
</result>
