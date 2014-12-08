<?php
function add_to_db($user, $screen_name, $oauth_token, $oauth_token_secret) {
  $con = mysql_connect("localhost","cc");
  if (!$con)
  {
    die('Could not connect: ' . mysql_error());
  }

  mysql_select_db("paypertweet", $con);

  $queryString = "";

  $queryString = "INSERT IGNORE INTO paypertweet.authed_users (id, oauth_token, oauth_token_secret, screen_name) values ('".$user."', '".$oauth_token."', '".$oauth_token_secret."', '".$screen_name."')";

  $result = mysql_query($queryString) or die(mysql_error());

  $queryString = "UPDATE paypertweet.authed_users set oauth_token='".$oauth_token."', oauth_token_secret='".$oauth_token_secret."', screen_name='".$screen_name."' where id='".$user."'";

  $result = mysql_query($queryString) or die(mysql_error());

  mysql_close($con);
}
?>

