# get the @replies to a user, and publish them
# also friends' tweets and publish them too


# case insensitive hashtag to pick out and publish to the hashtag topic
$hashtag = "#ma_campaign";

$0 = "twitter-receiver";


require "upub.pl";
#&upub("a","127.0.0.1",1883,"A",0,"n","123");


use Net::Twitter;
#use Data::Dumper;

&main;

sub main 
{
  print STDERR "\no_replies starting up...\n";
  # tell @andysc that we've started up
  #&upub("","127.0.0.1",1883,"twitterDM/andysc",0,"n","tap_ma restarting ".
  # scalar localtime());
  while (1) {
  eval {
  print STDERR "\nConnecting to Twitter...\n";
  $twit = Net::Twitter->new(
      traits	=> ['API::RESTv1_1','OAuth'],
      apiurl => 'https://api.twitter.com/1.1', 
      consumer_key    => "aYleSWW0tdPj8ethyWwUtA",
      consumer_secret => "AptG0ZLVWaCQckGOAA6XavUF0Y9ykXEsQZJHQLQYA"
  );
  print STDERR "Connected\n";

  $first_time = 1;

  open (FILE,"<last_tweet.dat");
  $last_seen = <FILE>;
  chomp $last_seen;
  close FILE;

  print "last seen: '$last_seen'\n";
 
  open (FILE,"<last_timeline.dat");
  $last_timeline = <FILE>;
  chomp $last_timeline;
  close FILE;
  print "last timeline: '$last_timeline'\n"; 

  open (FILE,"<last_dm.dat");
  $last_dm = <FILE>;
  chomp $last_dm;
  close FILE;
  print "last dm: '$last_dm'\n"; 

  #$access_token = "96728198-l6XNjHol0qe7TzLXmMH8w5wWG2YNEOvS7r3aoC2S8";
  #$access_token_secret = "yiSdehhnF54PX56axdGh9cdGob1dUuEFxxdpkIKtBY";
  $access_token = "96728198-rE4bhmoIRkIKm6DUJ80YAZqGaqQ93Q9JbozwYr0o";
  $access_token_secret = "1cMxdjk91vJLLGUwqOGkdCQl6FzoBAu4Ut6OfIR0";
  
  $twit->access_token($access_token);
  $twit->access_token_secret($access_token_secret);

  unless ($twit->authorized)
  {
    print STDERR "not authorised for ibmlights\n";
    exit; 
  } 

  while (1)
  {
   print STDERR "Checking Twitter at " . (localtime) . "\n"; 
      $dms = $twit->direct_messages;
      print "DMs: " . $dms . "\n";

   if (1)
   {
    undef $dms;
    if (!$last_dm)
    {
      print "first time for DMs\n";

      $dms = $twit->direct_messages;
      print "DMs: " . $dms . "\n";
      print "here\n";
    }
    else
    {
      #print "Getting DMs since ID $last_dm\n";
     
      $dms = $twit->direct_messages({since_id=>$last_dm});
      #eval { $dms = $twit->direct_messages({since_id=>$last_dm}) };
      print "error during twitter stuff\n\n" if $@;
    }

    if ($dms)
    {
      foreach $tweet (reverse @{$dms})
      {
        #print Dumper($tweet);
        
        $text = $tweet->{text};

        $text =~ s/[^[:ascii:]]+//g;  # get rid of non-ASCII characters

        $sender = $tweet->{sender_screen_name};

        $date_stamp = $tweet->{created_at};
        $msgID = $tweet->{id};
        print "$sender: $text\n\n";

        print "$date_stamp\t$msgID\t$sender\t$text\n";
        print "'$msgID' > '$last_dm' ?\n";
        
        if ($msgID gt $last_dm)
        {
          # remove any URLs and convert them to [url]
          $text =~ s/(http:\/\/.*?(\s|$))/[url]$2/ig;
          
          print "new DM!\n";
          &upub("","127.0.0.1",1883,"DM",0,"n","[$sender] $text");
          print STDERR ">>>DM from $sender - $text\n";

          open (FILE,">last_dm.dat");
          print FILE $msgID,"\n";
          close FILE;

          $last_dm = $msgID;
        }
        else
        {
          print "(seen before)\n";
        }
      }
    }
  }

    undef $replies;

    if (!$last_seen)
    {
      print "first time for replies\n";

      $replies = $twit->replies;
    }
    else
    {
      #print "Getting replies since ID $last_seen\n";
      eval {  $replies = $twit->replies({since_id=>$last_seen}) };
      print "error during twitter stuff\n\n" if $@;
    }

    if ($replies)
    {

      foreach $tweet (reverse @{$replies})
      {
        #print Dumper($tweet);
        
        $text = $tweet->{text};

        $text =~ s/[^[:ascii:]]+//g;  # get rid of non-ASCII characters

        $sender = $tweet->{user}{screen_name};

        $date_stamp = $tweet->{created_at};
        $msgID = $tweet->{id};
        #print "$sender: $text\n\n";

        print "$date_stamp\t$msgID\t$sender\t$text\n";
        #print "'$msgID' '$last_seen'\n";
         
        if ($msgID gt $last_seen)
        {
          # remove any URLs and convert them to [url]
          $text =~ s/(http:\/\/.*?(\s|$))/[url]$2/ig;

          # map "@tap_ma" to "MA"
          # unless it's at the start of the line (an @reply)
          #if ($text !~ /^\@MA_Campaign/i)
          #{
          #  $text =~ s/\@MA_Campaign/MA/i;
          #}
          
          if ($text =~ /\@MQ_Campaign/) {
             print "new reply tweet!\n";
             &upub("","127.0.0.1",1883,"mentions",0,"n","[$sender] $text");
             print STDERR ">>>tweet from $sender - $text\n";
          }

          open (FILE,">last_tweet.dat");
          print FILE $msgID,"\n";
          close FILE;

          $last_seen = $msgID;
        }
      }
    }

   undef $tweets;

    if (!$last_timeline)
    {
      print "first time for timeline\n";

      $tweets = $twit->home_timeline;
    }
    else
    {
      #print "Getting timeline since ID $last_timeline\n";
      eval { $tweets = $twit->home_timeline({since_id=>$last_timeline}) };
      print "error during twitter stuff\n\n" if $@;
    }

    if ($tweets)
    {
      foreach $tweet (reverse @{$tweets})
      {
        #print Dumper($tweet);
        
        $text = $tweet->{text};

        $text =~ s/[^[:ascii:]]+//g;  # get rid of non-ASCII characters

        $sender = $tweet->{user}{screen_name};

        $date_stamp = $tweet->{created_at};
        $msgID = $tweet->{id};
        print "$sender: $text\n\n";

        print "$date_stamp\t$msgID\t$sender\t$text\n";
        print "'$msgID' '$last_timeline'\n";

        if ($msgID gt $last_timeline)
        {
          # remove any URLs and convert them to [url]
          $text =~ s/(http:\/\/.*?(\s|$))/[url]$2/ig;
          
          print "new tweet!\n";

          if ($text =~ /\@ma_campaign/i) {
              print "@mention detected!\n";
              &upub("","127.0.0.1",1883,"mentions",0,"n","[$sender] $text");
              print STDERR ">>>mention from $sender - $text\n";
          }

          if (($text =~ /$hashtag\b/i)
             # send the entire timeline for "us" [DISABLED FOR NOW UNDER NEW RULES]
             # ||
             #($sender eq "andysc" || 
             # $sender eq "lauracowen" || 
             # $sender eq "matthew101")
          )
          {
            # if it's an @reply, but it's not to one of "us", then
            # it's the second half of a conversation he won't have seen the
            # first half of, so dump it
            # also dump it if it's got a URL in it
            #if (((substr($text,0,1) eq "\@") &&  
               #($text !~ /^(\@andysc|\@lauracowen|\@matthew101) /)) ||
            #    ($text =~ /\[url\]/)
            #   )
            #{ 
               # don't publish it
            #}
            #else
            #{
              # it has the #ma_campaign hashtag - publish to the hashtag topic
              print "hashtag detected!\n";
              #&upub("","127.0.0.1",1883,"hashtag",0,"n","[$sender] $text");
              print STDERR ">>>hashtag from $sender - $text\n";
            #}
          }
          else
          { # From now on we won't publish general timeline messages
            #&upub("","127.0.0.1",1883,"timeline",0,"n","[$sender] $text");
          }
          print ">>>tweet from $sender - $text\n";

          open (FILE,">last_timeline.dat");
          print FILE $msgID,"\n";
          close FILE;

          $last_timeline = $msgID;
        }
      }
    }

    if (1) #(!$first_time)
    {
      print STDERR "sleeeeeeep.....\n";
      sleep 180;
    }
   
    $first_time = 0;
  }
  };
  if ( my $err = $@ ) {
     print STDERR $@ . "\n";
     print STDERR "HTTP Response Code: ", $err->code, "\n",
           "HTTP Message......: ", $err->message, "\n",
           "Twitter error.....: ", $err->error, "\n";
     print STDERR "Error doing Twitter stuff. Will try again in 1 hour\n";
     sleep 3600;
  }
  }
}




###############################################################################   


sub get_rate_limit
{
   print "\ngetting rate limit status...\n";
   $rate_limit = $twit->rate_limit_status;
 

   print $rate_limit->{remaining_hits}," calls left this hour\n";
   print "reset time: $rate_limit->{reset_time}\n";
   print "\n";

   return $rate_limit->{remaining_hits}; 
}


###############################################################################

