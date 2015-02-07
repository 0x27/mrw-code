#!/usr/bin/perl
use lib '/home/mwhitehead/Web/apps/mqtt-perl/';
use MQTT::Client;
use CGI;

# Force the print buffer to flush each time
$| = 1;

my $message_prefix = "Hi Mark, here are your twitters for the last few days:\r\n";
my $message_suffix = "\r\n\r\nCheers, Matt";
my $currentemail = $message_prefix;
my $current_pending_messages = 0;

my $mqtt = MQTT::Client->new({
   brokerIP => "192.168.1.2",
   clientID => "twitaprisclient",
   callback_publish => \&publish_callback
});

# To perform a 'one-shot' publish to a topic:
$mqtt->connect();
$mqtt->subscribe("ibmlights/mentions",0);
$mqtt->subscribe("ibmlights/DM",0);
$mqtt->subscribe("ibmlights/hashtag",0);
$mqtt->listen_for_publishes();

sub publish_callback {

   # We've received a message
   my $topic = shift;
   my $newmsg = shift;
   my $now = localtime time;
   my $bumped = 0;
   print "Message received at " . $now . ":\n";
   print $newmsg . "\n";
  
   if ($newmsg eq 'quit') {
      $mqtt->disconnect();
      exit 0;
   } elsif ($newmsg eq 'bump') {
     $bumped = 1;
   } else {
      # Append the new message to the current email
      $currentemail = $currentemail . "\r\n" . $newmsg;
      $current_pending_messages++;
      print "Current pending messages: " . $current_pending_messages ."\n";

      # Publish to a debug topic so we can see that the email is being built up
      $mqtt->publish("tapbridge/debug",1,1,$currentemail);
      print "Published message to tapbridge/debug\n";
   }

   # Check if we've hit our max length limit, or if too much time
   # has elapsed since the last email was sent
   if ((length($currentemail) > 2300) || ($bumped && $current_pending_messages)) {

      if ($bumped && $current_pending_messages) {
         print "Sending mail after receiving timeout bump\n";
      } elsif ($bumped) {
         print "Bumped but no messages to send\n";
      }
      
      # Add the message suffix
      $currentemail = $currentemail . $message_suffix;
      print "Max message length reached - writing email file\n";
      print $currentemail;
      print "(" . length($currentemail) . ")";
      print "\n";

      # Convert the text to application/x-www-form-urlencoded
      $escapedemail = CGI::escape($currentemail);
      $escapedemail =~ s/\%20/+/g;

      # Add the rest of the query parameters required to send the form to emailaprisoner.com
      $escapedemail = "drpRecipient=101641&txtMessage=" . $escapedemail . "&btnSubmitMessage=Send+Message&mid=0&crcount=&messageform=true";
 
      # Write the email to file so we can upload it to emailaprisoner.com
      open FILE, ">/tmp/twitpris-email.txt";
      print FILE $escapedemail;
      close FILE;

      # Publish a message to the @tapbridge twitter feed to let people know 
      # that a message has been sent to Mark
      $mqtt->publish("hur_link/ibmlights",0,0,"An aggregated message has been sent to e-a-p");
      $mqtt->publish("tapbridge/debug",1,1,$currentemail . "\n\n[Sent]");
      $mqtt->publish("tapbridge/lastsent",1,1,$currentemail . "\n\n[Sent]");
      print "Published message to hur_link/ibmlights\n";

      # Reset the current message 
      $currentemail = $message_prefix;
      $bumped = 0;
      $current_pending_messages = 0;
   }

   #$mqtt->disconnect();
   #exit 0;
}
