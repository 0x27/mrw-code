<?php header('Content-type: application/xml'); ?>
<?php echo("<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n"); ?>
<?php require "../common/globals.php"; ?>
<?php require "../common/oauth_helper.php"; ?>
<?php require "getuseroauthtokens.php"; ?>
<?php require "getfollowrequests.php"; ?>
<users>
<?php
getListOfUsers();

function getListOfUsers() {
  $conn = mysql_connect("localhost","cc");
  if (!$conn)
  {
    die('Could not connect: ' . mysql_error());
  }

  mysql_select_db("paypertweet", $conn);

  $queryString = "";

  // Get list of all current users
  $queryString = "SELECT * FROM paypertweet.authed_users";

  $result = mysql_query($queryString) or die(mysql_error());
 
  while ($array=mysql_fetch_assoc($result))
  {
     $nextUserID = $array['id'];
     $nextUserScreenName = $array['screen_name'];
     $nextUserOauthToken = $array['oauth_token'];
     $nextUserOauthTokenSecret = $array['oauth_token_secret'];
     $nextUserOauthTokens = getOauthToken($nextUser);
     $nextUsersFollowRequests = getFollowers($nextUserOauthTokens['oauth_token'], $nextUserOauthTokens['oauth_token_secret']);

     print("<nextuser>");

     print("<id>");
     print($nextUserID);
     print("</id>");

     print("<screen_name>");
     print($nextUserScreenName);
     print("</screen_name>");

     print("<oauth_token>");
     print($nextUserOauthToken);
     print("</oauth_token>");

     print("<oauth_token_secret>");
     print($nextUserOauthTokenSecret);
     print("</oauth_token_secret>");

     print("</nextuser>");

     // TODO - print this array out properly to show all the follow requests
     //print($nextUsersFollowRequests."\n");
  }

  if (isset($conn) && is_resource($conn)) {
    mysql_close($conn);
  }
}
?>
</users>

