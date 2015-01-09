<?php
require '../common/globals.php';
require '../common/twitter-php-3.2/src/twitter.class.php';

// Fill in the next 2 variables.
$access_token='15201275-loypZtq58TRB1qoIIu6fTw6TSEqluGZ1aMKgVJjJe';
$access_token_secret='haKjTuzH9N6UPhOBZDKSsu6FAZzIvLNbwGhi5wfy00Y';

$twitter = new Twitter(OAUTH_CONSUMER_KEY, OAUTH_CONSUMER_SECRET, $access_token, $access_token_secret);

try {
$results = $twitter->search('#ibm');

foreach ($results as $result) {
                echo "message: ", $result->text;
                echo "posted at " , $result->created_at;
                echo "posted by " , $result->form_user;
        }

   echo "\n\n\n\n";
} catch (TwitterException $e) {
   echo "Error searching Twitter: ",$e->getMessage();
}

try {
$statuses = $twitter->request('statuses/retweets_of_me', 'GET', array('count' => 20));

foreach ($statuses as $status) {
                echo "message: ", $status->text;
                echo "posted at " , $status->created_at;
                echo "posted by " , $status->form_user;
        }

echo "\n\n\n\n";
} catch (TwitterException $e) {
   echo "Error getting statuses from Twitter: ",$e->getMessage();
}

try {
$results = $twitter->loadUserFollowers('matthew101HS');

foreach ($results as $result) {
                echo "message: ", $result->text;
                echo "posted at " , $result->created_at;
                echo "posted by " , $result->form_user;
        }
} catch (TwitterException $e) {
   echo "Error loading followers from Twitter: ",$e->getMessage();
}

try {
$twitter->request('direct_messages/new', 'POST', array('screen_name' => 'matthew101', 'text' => 'hello'));

echo "\n\n\n\n";
} catch (TwitterException $e) {
   echo "Error posting DM to Twitter: ",$e->getMessage();
}

?>

