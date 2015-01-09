<?php
function getOauthToken($user) {
  $con = mysql_connect("localhost","cc");
  if (!$con)
  {
    die('Could not connect: ' . mysql_error());
  }

  mysql_select_db("paypertweet", $con);

  $queryString = "";

  $queryString = "SELECT oauth_token, oauth_token_secret FROM paypertweet.authed_users WHERE id LIKE '".$user."'";

  $result = mysql_query($queryString) or die(mysql_error());
 
  $array=mysql_fetch_assoc($result);

  mysql_close($con);

  return $array;
}
?>

