#!/usr/bin/perl
# get the @replies to a user, and publish them
# also friends' tweets and publish them too


# case insensitive hashtag to pick out and publish to the hashtag topic

require "upub.pl";


use Net::Twitter;
#use Data::Dumper;

&main;


sub main 
{
  print "\nPay Per Twitter starting...\n";

  $twit = Net::Twitter->new(
      traits	=> ['API::REST','OAuth'],
      consumer_key    => "QvffeAkDss4USFwd6OrXw",
      consumer_secret => "6iqQ2quPVXr8vZ4rm4Vmum09XMZkpLs3Jk8XOhqB8"
  );

  $first_time = 1;

  $access_token = "14867391-3bhNQ7pn9e1c2cGYjfQgvRJPIr1wF1JiqQs8dswgu";
  $access_token_secret = "p9K57OezGND7kef2lOy4Ma2yXa2jvxeeVG8lCms";
  
  $twit->access_token($access_token);
  $twit->access_token_secret($access_token_secret);

  unless ($twit->authorized)
  {
    print "not authorised for Pay Per Tweet\n";
    exit; 
  } 

  while (1)
  {
   if (1)
   {
    undef $dms;
    if (!$last_dm)
    {
      print "first time for DMs\n";

      $dms = $twit->direct_messages;
      print "here\n";
    }
    else
    {
      print "getting DMs since ID $last_dm\n";
     
      eval { $dms = $twit->direct_messages({since_id=>$last_dm}) };
      print "error during twitter stuff\n\n" if $@;
      print "done\n";
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
          print ">>>DM from $sender - $text\n";

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
      print "getting replies since ID $last_seen\n";
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
        print "$sender: $text\n\n";

        print "$date_stamp\t$msgID\t$sender\t$text\n";
        print "'$msgID' '$last_seen'\n";
         
        
        if ($msgID gt $last_seen)
        {
          # remove any URLs and convert them to [url]
          $text =~ s/(http:\/\/.*?(\s|$))/[url]$2/ig;

          # map "@tap_ma" to "MA"
          # unless it's at the start of the line (an @reply)
          if ($text !~ /^\@tap_ma/i)
          {
            $text =~ s/\@tap_ma/MA/i;
          }
          
          print "new tweet!\n";
          &upub("","127.0.0.1",1883,"mentions",0,"n","[$sender] $text");
          print ">>>tweet from $sender - $text\n";

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
      print "getting timeline since ID $last_timeline\n";
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

          if ($text =~ /$hashtag\b/i ||
              # send the entire timeline for "us"
             ($sender eq "andysc" || 
              $sender eq "lauracowen" || 
              $sender eq "matthew101")
          )
          {
            # if it's an @reply, but it's not to one of "us", then
            # it's the second half of a conversation he won't have seen the
            # first half of, so dump it
            # also dump it if it's got a URL in it
            if (((substr($text,0,1) eq "\@") &&  
               ($text !~ /^(\@andysc|\@lauracowen|\@matthew101) /)) ||
                ($text =~ /\[url\]/)
               )
            { 
               # don't publish it
            }
            else
            {
              # it has the #ma hashtag - publish to the hashtag topic
              print "hashtag detected!\n";
              &upub("","127.0.0.1",1883,"hashtag",0,"n","[$sender] $text");
            }
          }
          else
          {
            &upub("","127.0.0.1",1883,"timeline",0,"n","[$sender] $text");
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
      print "sleeeeeeep.....\n";
      sleep 2*60;
    }
   
    $first_time = 0;


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

