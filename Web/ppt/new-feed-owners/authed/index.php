<html>
<head>
<?php require "../../common/globals.php"; ?>
<?php require "../../common/oauth_helper.php"; ?>
<?php require "../getacctok.php"; ?>
<?php require "add_user_to_db.php"; ?>
<title>Pay Per Tweet</title>
</head>
<body>
<h2>Welcome to Pay Per Tweet, an application which allows you to generate income from your twitter feed.</h2>
<p>Thank you for authorising Pay Per Tweet&#153;. You're almost ready to start making money from your Twitter feed.</p>
<?php
// Now pass the interim parameters into getacctoken.php
$response_array = get_acc_tokens(htmlspecialchars($_GET["oauth_token"]), htmlspecialchars($_GET["oauth_verifier"]));
add_to_db($response_array["user_id"], $response_array["screen_name"], $response_array["oauth_token"], $response_array["oauth_token_secret"]);
?>
<p>Now all you have to do is configure the payment settings for your Twitter feed and you're done!</p>
<h3>Payment Settings</h3>
<form name="paysettings">
<p>
<input type="text" value="10.00" size="6"/>GBP 
<select name="paymentperiod">
<option name="monthly" value="Monthly"> Monthly </option>
<option name="yearly" value="Yearly"> Yearly </option>
</select>
</p>
<p>
<input type="button" value="Apply settings"/>
</p>
</form>
</body>
</html>

