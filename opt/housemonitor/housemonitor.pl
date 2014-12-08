########################################################################


########################################################################

use DBI;
use POSIX strftime;

my $db_name = 'mydata';
my $db_user = 'cc';
my $db_type = 'mysql';
my $db = DBI->connect("DBI:$db_type:$db_name",$db_user) or die "Couldn't connect to database: " . DBI->errstr;
my $kettle_event_set = 0;
my $shower_event_set = 0;
my $gas_event_set = 0;
my $wireless_node_two_event_set = 0;
my $wireless_node_three_event_set = 0;
my $gas_on = 0;
my $gas_i = 0;

# Variables to use for checking system load and restarting lighttpd
my $lighttpd_restart_count = 0;

print STDERR "Starting main loop...\n";

# Only select electricity data for between 9am and 5pm, and between Monday-Fri inclusive
my $select_watts_handle = $db->prepare_cached('SELECT * FROM electricity WHERE curdate() < timestamp AND timestampadd(SECOND,-20,now()) < timestamp(timestamp) AND HOUR(timestamp) < 17 AND HOUR(timestamp) > 8 AND DAYOFWEEK(timestamp) > 1 AND DAYOFWEEK(timestamp) < 7');

# Only select gas data for between 11pm and 4am, regardless of the day. If the gas is on, I want to know
my $select_gas_handle = $db->prepare_cached('SELECT * FROM gas WHERE curdate() < timestamp AND timestampadd(SECOND,-30,now()) < timestamp(timestamp) AND (HOUR(timestamp) > 22 OR HOUR(timestamp) < 4)');

# Select wireless node data for node2 for the last 75 minutes
my $select_wireless_node_two_handle = $db->prepare_cached('SELECT * FROM wirelessnodes WHERE curdate() < timestamp AND nodeid = 2 AND timestampadd(MINUTE,-75,now()) < timestamp(timestamp)');

# Select wireless node data for node3 for the last 75 minutes
my $select_wireless_node_three_handle = $db->prepare_cached('SELECT * FROM wirelessnodes WHERE curdate() < timestamp AND nodeid = 3 AND timestampadd(MINUTE,-75,now()) < timestamp(timestamp)');

while (true) {

  my $starttime = time();

  # Check system load
  my $loadint = `uptime | awk -F "\$FTEXT" '{ print \$2 }' | cut -d, -f2 | sed 's/ //g'`;

  if ($loadint >= 9 && $lighttpd_restart_count >= 100) {
    print "Processor load too high. Restarting lighttpd\n";
    #system('/etc/init.d/lighttpd restart');
    system("/home/mwhitehead/opt/sendtweet/sendtweet.sh", "Load is too high on the server ($starttime)");
    $lighttpd_restart_count = 0;
  }

  # This stops lighttpd being reset more than once every 40 minutes or so
  if ($lighttpd_restart_count < 100) {
    $lighttpd_restart_count++;
  }

  # Check electricity and gas usage
  $select_watts_handle->execute();
  $select_gas_handle->execute();
  $select_wireless_node_two_handle->execute();
  $select_wireless_node_three_handle->execute();

  $select_watts_handle->bind_columns(undef, \$timestamp, \$watts);
  $select_gas_handle->bind_columns(undef, \$timestampgas, \$tick);
  $select_wireless_node_two_handle->bind_columns(undef, \$timestampwirelesstwo, \$nodeidtwo, \$voltagetwo);
  $select_wireless_node_three_handle->bind_columns(undef, \$timestampwirelessthree, \$nodeidthree, \$voltagethree);

  my $avg = 0;
  my $i = 0;
  my $gas_i++;

  # LOOP THROUGH ELECTRICITY RESULTS
  while($select_watts_handle->fetch()) {
    $avg = $avg + $watts;
    $i++;
    print "$timestamp, $watts\n";
  } 

  # SEE IF WE HAVE ANY GAS RESULTS IN THE LAST 20 SECONDS (SHOULD BE NONE)
  if ($select_gas_handle->fetch()) {
    print STDERR "Gas is on @ $timestampgas\n";
    $gas_on = 1;
  } else {
    #print STDERR "No gas data points to use\n";
    $gas_on = 0;
  }

  # SEE IF WE HAVE ANY WIRELESS RESULTS FROM NODE 2 IN THE LAST 75 MINUTES (SHOULD BE AT LEAST 1)
  if ($select_wireless_node_two_handle->fetch()) {
    #print STDERR "Wireless data received in the last 75 minutes from node 2\n";

    if ($wireless_node_two_event_set) {
       print STDERR "Wireless node 2 up and running again - sending tweet\n";
       #system("/home/mwhitehead/opt/sendtweet/sendtweet.sh", "Wireless node 2 up and running again ($timestampwirelesstwo)");
    }
    $wireless_node_two_event_set = 0;
  } else {
    if (!$wireless_node_two_event_set) {
       print STDERR "No wireless data received in the last 75 minutes for node 2 - sending tweet\n";
       #system("/home/mwhitehead/opt/sendtweet/sendtweet.sh", "No data received from wireless node 2 in the last 75 mins ($timestampwirelesstwo)");
       $wireless_node_two_event_set = 1;
    }
  }

  # SEE IF WE HAVE ANY WIRELESS RESULTS FROM NODE 3 IN THE LAST 75 MINUTES (SHOULD BE AT LEAST 1)
  if ($select_wireless_node_three_handle->fetch()) {
    #print STDERR "Wireless data received in the last 75 minutes from node 3\n";

    if ($wireless_node_three_event_set) {
       print STDERR "Wireless node 3 up and running again - sending tweet\n";
       #system("/home/mwhitehead/opt/sendtweet/sendtweet.sh", "Wireless node 3 up and running again ($timestampwirelessthree)");
    }
    $wireless_node_three_event_set = 0;
  } else {
    if (!$wireless_node_three_event_set) {
       print STDERR "No wireless data received in the last 75 minutes for node 3 - sending tweet\n";
       #system("/home/mwhitehead/opt/sendtweet/sendtweet.sh", "No data received from wireless node 3 in the last 75 mins ($timestampwirelessthree)");
       $wireless_node_three_event_set = 1;
    }
  }

  if ($i > 0) {
    $avg = $avg / $i;

    if (!$kettle_event_set && $avg > 2100 && $avg < 3000) {
      print STDERR "Average power ($avg) > 2100 watts: Kettle on\n";
      $kettle_event_set = 1;
      system("/home/mwhitehead/opt/sendtweet/sendtweet.sh", "The kettle's on in the house");
    } elsif (!$shower_event_set && $avg > 8000) {
      print STDERR "Average power ($avg) > 8000 watts: Shower on\n";
      $shower_event_set = 1;
      system("/home/mwhitehead/opt/sendtweet/sendtweet.sh", "The shower's on in the house");
    } elsif ($avg < 1500) {
      $kettle_event_set = 0;
      $shower_event_set = 0;
      #print STDERR "Average power = $avg watts: Sleeping\n";
    }
  } else {
    #print STDERR "No electricity data points to use\n";
  }

  if (!$gas_event_set && $gas_on) {
     print STDERR "The gas is on - sending tweet\n";
     $gas_event_set = 1;
     system("/home/mwhitehead/opt/sendtweet/sendtweet.sh", "The gas is on in the house - $timestampgas");
  } elsif ($gas_i > 90) {
     # We only want to sent the alert at most every 30 minutes
     $gas_event_set = 0;
     $gas_i = 0;
     print STDERR "Resetting gas counter after 30 minutes\n";
  }

  my $endtime = time();

  #print STDERR "Checks took " . ($endtime - $starttime) . " seconds to complete\n";
 
  sleep(20);

}
print "\ndone\n";


