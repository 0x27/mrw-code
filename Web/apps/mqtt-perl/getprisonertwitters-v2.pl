#!/usr/bin/perl
use strict;
use lib '/home/mwhitehead/mrw-code/Web/apps/mqtt-perl/';
use MQTT::Client;
use CGI;

# Force the print buffer to flush each time
$| = 1;

$0 = "twitter-compiler";

my $pid;
my $message_prefix = "Hi Mark, here are some messages from you friends and supporters...\r\n\r\n";
my $message_suffix = "\r\n";
my $currentemail = "";
my $lastsenttime = time;
my $now = localtime time;

my $mqtt = MQTT::Client->new({
   brokerIP => "localhost",
   clientID => "tap-client",
   callback_publish => \&publish_callback
});

# Check the fork() method is available
die "Fork libs not available$!\n" unless defined($pid=fork());

if($pid) {
   print "Starting Twitter-a-Prisoner (Version 2)\n";

   # Connect to the broker and subscribe to the topics we need 
   $mqtt->connect();
   $mqtt->subscribe("mentions",0);
   $mqtt->subscribe("DM",0);
   $mqtt->subscribe("hashtag",0);
   $mqtt->subscribe("tapbridge/pending",0);
   $mqtt->subscribe("tapbridge/test",0);
   $mqtt->subscribe("tapbridge/bump",0);
   $mqtt->subscribe("tapbridge/force",0);
   $mqtt->listen_for_publishes();
}
else {
   # No need to do this any more. While cron wasn't working
   # we needed to manually bump the code, but now we have
   # a cronjob to do it every 10 days
   #LOOP1:
      # several days in seconds
   #   sleep 777600;
   #   print "[Timer Fired]\n";
      
      # Create a client to ping the main app to send any pending messages
   #   my $mqttT = MQTT::Client->new({
   #      brokerIP => "localhost",
   #      clientID => "tap-timer",
   #      callback_publish => \&publish_callback
   #   });
   #   $mqttT->connect();
   #   $mqttT->publish("tapbridge/bump",0,0,"bump");
   #   $mqttT->disconnect();
   #goto LOOP1;
}

# The method called when we receive a published message
sub publish_callback {

   # We've received a message
   my $topic = shift;
   my $newmsg = shift;
   my $now = localtime time;
   my $nowseconds = time;

   if ($newmsg eq 'quit') {
      $mqtt->disconnect();
      kill ("TERM", $pid); 
      exit 0;
   } elsif ($topic eq 'tapbridge/pending') {
      # The message is the persisted publication of pending messages
      print "Pending message list received at " . $now . "\n";
      $currentemail = $newmsg;
      print "Current message: " . $currentemail . "\n";
   } elsif ($topic eq 'mentions') {
      # The message is a twitter '@tap_MA' message
      print "\@tapbridge message received at " . $now . ":\n";
      # Insert '@' into the name, i.e. [matthew101] = [matthew101@]
      substr($newmsg, index($newmsg, ']'), 1) = '@]';
      substr($newmsg, index(lc($newmsg), "\@tap_ma") < 0 ? length $newmsg : index(lc($newmsg), "\@tap_ma"), 7) = '';
      # Trim the whitespace from the rhs
      $newmsg =~ s/\s+$//;
      $newmsg = $newmsg . " [" . $now . "]";
      print $newmsg . "\n";
      &check_email($currentemail, $newmsg);
   } elsif ($topic eq 'hashtag') {
      # The message is a twitter '#ma_campaign' message
      print "#ma_campaign message received at " . $now . ":\n";
      # Insert '#' into the name, i.e. [matthew101] = [matthew101#]
      substr($newmsg, index($newmsg, ']'), 1) = '#]';
      #substr($newmsg, index(lc($newmsg), "#ma") < 0 ? length $newmsg : index(lc($newmsg), "#ma"), 3) = '';
      # Trim the whitespace from the rhs
      $newmsg =~ s/\s+$//;
      $newmsg = $newmsg . " [" . $now . "]";
      print $newmsg . "\n";
      &check_email($currentemail, $newmsg);
   } elsif ($topic eq 'DM') {
      # The message is direct message to tapbridge
      print "DM message received at " . $now . ":\n";
      # Insert a DM '*' into the name, i.e. [matthew101] = [matthew101*]
      substr($newmsg, index($newmsg, ']'), 1) = '*]';
      $newmsg = $newmsg . " [" . $now . "]";
      print $newmsg . "\n";
      &check_email($currentemail, $newmsg);
   } elsif ($topic eq 'tapbridge/test') {
      # The message is a test message
      print "Test message received at " . $now . "\n";
      &check_email($currentemail, $newmsg);
   } elsif ($topic eq 'tapbridge/bump') {
      # The message is a message from the timer - send any pending messages
      print "Bump message received at " . $now . "\n";
      if (($nowseconds - $lastsenttime) > 2000000 || ($newmsg eq "force")) {
         &send_email($currentemail);
      } else {
         print "Bumped but we sent a mail in the last 28 days so waiting for the next one\n";
      }
   } elsif ($topic eq 'tapbridge/force') {
      # The message is a message to force the email to send, regardless of timers etc.
      print "Force message received at " . $now . "\n";
      &send_email($currentemail);
   } else {
      print "Unknown message received at " .$now . ":\n";
      print $newmsg . "\n";
   }
}

# Method which checks if the email is long enough to send yet
sub check_email {

   # Check that the current message doesn't contain the new message already
   if (index($_[0], $_[1]) == -1) {

      # If we add this new message will we exceed our character limit? If so,
      # send the latest email WITHOUT the new message, then store the new message
      # away ready for the next set of messages to be added to it and sent.

      if ((length($_[0]) + 
           length("\n") + 
           length($_[1]) + 
           length($message_prefix) + 
           length($message_suffix)) > 2482) {
 
         &send_email($_[0]);
         $mqtt->publish("tapbridge/pending",1,1,$_[1]);
         $mqtt->publish("tapbridge/debug",1,1,$_[1]);
      } else {
         # If it's not too long there's no point in sending it yet. Just add
         # it to the list of pending messages and wait for more to come along
         if (length($_[0]) > 0) {
            $mqtt->publish("tapbridge/pending",1,1, ($_[0] . "\n" . $_[1]));
            $mqtt->publish("tapbridge/debug",1,1, ($_[0] . "\n" . $_[1]));
         } else {
            $mqtt->publish("tapbridge/pending",1,1, $_[1]);
            $mqtt->publish("tapbridge/debug",1,1, $_[1]);
         }
      } 
   }
}

sub send_email {
   if (length($_[0]) > 0) {
      
      my $now = localtime time;
      my @timeelements = split(/[ :]+/, $now);
      my $date_suffix = " (" . @timeelements[1] . " " . @timeelements[2] . ", " . @timeelements[3] . ":" . @timeelements[4] . ")";

      print "Sending the email:\n";
      $lastsenttime = time;
      print $message_prefix . $currentemail . $message_suffix . $date_suffix . "\n";
      $mqtt->publish("tapbridge/lastsent",1,1,$message_prefix . $currentemail . $message_suffix . $date_suffix . "\n\n[Sent]\n");
      $mqtt->publish("tapbridge/pending",1,1,"");

      # Convert the text to application/x-www-form-urlencoded
      my $escapedemail = CGI::escape($message_prefix . $currentemail . $message_suffix . $date_suffix);
      $escapedemail =~ s/\%20/+/g;

      # Add the rest of the query parameters required to send the form to emailaprisoner.com
      $escapedemail = "rid=101641&txtMessage=" . $escapedemail . "&btnSubmitMessage=Send+Message&mid=0&crcount=&messageform=true";

      # Write the email to file so we can upload it to emailaprisoner.com
      open FILE, ">/temp/twitpris-email-v2.txt";
      print FILE $escapedemail;
      close FILE;

   } else {
      print "[Nothing to send]";
   }
}

