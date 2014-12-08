<?php
require '../common/globals.php';
require '../common/twitter-php-3.2/src/twitter.class.php';

// Fill in the next 2 variables.
$access_token='15201275-loypZtq58TRB1qoIIu6fTw6TSEqluGZ1aMKgVJjJe';
$access_token_secret='haKjTuzH9N6UPhOBZDKSsu6FAZzIvLNbwGhi5wfy00Y';

$twitter = new Twitter(OAUTH_CONSUMER_KEY, OAUTH_CONSUMER_SECRET, $access_token, $access_token_secret);

$message = $_GET['message'];

try {
$twitter->request('direct_messages/new', 'POST', array('screen_name' => 'matthew101', 'text' => $message));

echo "\n\n\n\n";
} catch (TwitterException $e) {
   echo "Error posting DM to Twitter: ",$e->getMessage();
}

?>

