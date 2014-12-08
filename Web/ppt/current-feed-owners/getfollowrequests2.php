<?php
// Fill in the next 2 variables based on the user we're doing this for. 
// Currently these are hard coded for matthew101HS:
//$access_token='15201275-loypZtq58TRB1qoIIu6fTw6TSEqluGZ1aMKgVJjJe';
//$access_token_secret='haKjTuzH9N6UPhOBZDKSsu6FAZzIvLNbwGhi5wfy00Y';
//$tweet = 'Hello World!';

// GET a list of the current follow requests for this user
//$retarr = getFollowers($oauth_token, $oauth_token_secret);
//exit(0);

/**
* Call twitter to get a list of the user's follow requests
* @param string $access_token obtained from get_request_token
* @param string $access_token_secret obtained from get_request_token
* @return response string or empty array on error
*/
function getFollowers($access_token, $access_token_secret)
{
  $retarr = array(); // return value
  $response = array();

  //$url = 'http://api.twitter.com/1/statuses/update.json';
  $url = 'http://api.twitter.com/1.1/friendships/incoming.json';
  //$params['status'] = $status_message;
  $params['oauth_version'] = '1.0';
  $params['oauth_nonce'] = mt_rand();
  $params['oauth_timestamp'] = time();
  $params['oauth_consumer_key'] = OAUTH_CONSUMER_KEY;
  $params['oauth_token'] = $access_token;

  // compute hmac-sha1 signature and add it to the params list
  $params['oauth_signature_method'] = 'HMAC-SHA1';
  $params['oauth_signature'] =
      oauth_compute_hmac_sig('GET', $url, $params,
                             OAUTH_CONSUMER_SECRET, $access_token_secret);

  // Pass OAuth credentials in a separate header or in the query string
  $query_parameter_string = oauth_http_build_query($params, true);
  $header = build_oauth_header($params, "Twitter API");
  $headers[] = $header;

  // POST or GET the request
  $request_url = $url . ($query_parameter_string ?
                           ('?' . $query_parameter_string) : '' );
  //logit("tweet:INFO:request_url:$request_url");
  $response = do_get($request_url, 80, $headers);

  // extract successful response
  if (! empty($response)) {
    list($info, $header, $body) = $response;
    if ($body) {
      //logit("tweet:INFO:response:");
      //print(json_pretty_print($body));
    }
    $retarr = $response;
  }

  return $retarr;
}
?>

