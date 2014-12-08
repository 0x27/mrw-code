use Net::Twitter;

&main;


sub main
{
  print "\nPay Per Twitter starting...\n";

  $twit = Net::Twitter->new(
      traits    => ['API::REST','OAuth'],
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



          $last_dm = $msgID;
        }
        else
        {
          print "(seen before)\n";
        }
 }
    }

}

