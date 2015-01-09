<html>
<head>
<?php require "common/globals.php"; ?>
<?php require "common/oauth_helper.php"; ?>
<?php require "new-feed-owners/getreqtoken.php"; ?>
<title>Pay Per Tweet</title>
</head>
<body>
<h2>Welcome to Pay Per Tweet, an application which allows you to generate income from your twitter feed.</h2>
<p>To start making money from your tweets you'll need to make sure your Twitter feed is private. Once you've done this, all you need to do is authorise the Pay Per Tweet&#153; application to manage your private Twitter feed.</p> 
<p>To authorise Pay Per Tweet&#153; to manage your feed, click the following link and log in with your Twitter details:</p>
<p><a href="<?php get_auth_url(); ?>">Authorise Pay Per Tweet&#153;</a></p>
<p>When you complete the authorisation process you'll be redirected back to the Pay Per Tweett&#153; website to complete the setup process.</p>
</body>
</html>

