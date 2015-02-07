package RFM12Monitor;

use Data::Dumper;
use lib '/home/mwhitehead/Web/apps/mqtt-perl/';
use MQTT::Client;
use Encode;
use strict;
use Devel::Cycle;
use Devel::Leak;

# Create an array to store the last update time for up to (for the time being) 6 nodes
my @lastPubTimes = (time(), time(), time(), time(), time(), time());

sub new
{
	my $class = shift || die;
	my %params = @_;
	die "broker not set" unless $params{broker};
	die "name not set" unless $params{name};
	my $port = $params{port} || 1883;
	my $keepalive = $params{keepalive} || 120;
	my $debug = $params{debug} || 0;
        my $publisher = $params{pub} || 0;
    
        my $mqtt;
        my $db_name = 'mydata';
        my $db_user = 'cc';
        my $db_type = 'mysql';
        my $db;

        if ($publisher) {
          $db = DBI->connect("DBI:$db_type:$db_name",$db_user) or die "Couldn't connect to database: " . DBI->errstr;

          # Since readings might not come for days, we need to automatically 
          # reconnect to the DB after the default 8 hour timeout
          $db->{'AutoCommit'} = 1;
          $db->{mysql_auto_reconnect} = 1;
        }

	my $self = {
			_broker => $params{broker},
			_name   => $params{name},
			_port   => $port,
			_keepalive => $keepalive,
			_debug  => $debug,
                        _db => $db,
                        _mqtt => $mqtt,
	};
	bless $self, $class;

	return $self;
}

sub connect
{
    my $self = shift || die;
    my $clientName = shift || die "No client name specified";
    my $callbackFunction = shift;

    print "Connecting with client id " . $clientName . "\n";

    if (!defined $self->{_mqtt}) {

       print "Creating MQTT client\n";

       if (defined $callbackFunction) {
          $self->{_mqtt} = MQTT::Client->new({
             brokerIP => "localhost",
             clientID => $clientName,
             callback_publish => $callbackFunction
          });   
       } else {
          $self->{_mqtt} = MQTT::Client->new({
            brokerIP => "localhost",
            clientID => $clientName
          });   
       }
    }

    $self->{_mqtt}->connect();

    return 0;
}

sub subscribe {
   my $self = shift || die;
   $self->{_mqtt}->subscribe('local/rfm12/command', 0);
}

sub waitForMessages {
   my $self = shift || die;
   $self->{_mqtt}->listen_for_publishes();
}

sub receive_pub {
   my $self = shift || die;

   my @publication = $self->{_mqtt}->receivePub();
   print "Received publication at " . scalar localtime() . "\n";
   return @publication;
}

sub publish_rfm12
{
   my $self = shift || die;
   my $value = shift || die "no value to publish";
   my $rc = 0;

   eval {
        my $handle; # apparently this doesn't need to be anything at all
        my $leaveCount = 0;
        my $enterCount = Devel::Leak::NoteSV($handle);
        print STDERR "ENTER: $enterCount SVs\n";

        # Check the prefix that the arduino adds to the sensor reading is present
        if ($value =~ /DATA/) {

          # Strip the prefix 'DATA: ' from the value
          $value = substr($value, 6);

          # Trim whitespace from the RHS
          $value =~ s/\s+$//;

	  if($self->{_debug}){
                my $now_string = localtime;
                print STDERR $now_string . " -> " . $value . "\n";

                # This is the prefix to any data published by my RFM12 nodes.
                # The 'Nd' stands for 'Node' and indicates the number of the
                # node that published this data, e.g. Nd02
                # Strip everything before (and including) 'Nd'...
                my $substr = 'Nd';

                my $result = index($value, $substr);

                $value = substr($value, $result + 2);

                # $value is now 'nn v1 v2 v3 ...' where nn is the number
                # of the node and v1, v2 etc. are the individual values
                # published by the node.

                # Get nn from the string, by looking for the first whitespace...
                my $nextWhitespace = index($value, ' ');

                my $nodeNumber = substr($value, 0, $nextWhitespace);
                print "Node number: " . $nodeNumber . "\n";
                $value = substr($value, $nextWhitespace + 1);

                # Publish the raw data in case it's useful for debugging new applications
	        #&upub_publish ("local/rfm12/raw/" . $nodeNumber,"y",$value);
	        #&upub_publish ("local/rfm12/nodes/" . $nodeNumber . "/lastreceived","y",$now_string);
                $self->{_mqtt}->publish("local/rfm12/raw/" . $nodeNumber, 1, 0, $value);

                # This is the only topic we publish retained messages on. 
                # It's useful for new clients to be able to see the last time 
                # a publication was made, but not the actual values in case
                # we end up putting a duplicate record in the databas, $now_stringe
                $self->{_mqtt}->publish("local/rfm12/nodes/" . $nodeNumber . "/lastreceived", 1, 1, $now_string);

                print STDERR "[Node number: " . $nodeNumber . "\n";
                
                if ($nodeNumber eq "02" || $nodeNumber eq "03") {
                  # Do node specific stuff here...
                  # For nodes 2 and 3, the data are: 
                  #  - a counter (between 0 and 9)
                  #  - the humidity % 
                  #  - the temperature
                  #  - battery voltage (x10, e.g. 33 = 3.3v)

                  # Skip over the counter
                  $nextWhitespace = index($value, ' ');
                  my $counter = substr($value, 0, $nextWhitespace);
                  $value = substr($value, $nextWhitespace + 1);

                  # Get the humidity
                  $nextWhitespace = index($value, ' ');
                  my $humidity = substr($value, 0, $nextWhitespace);
                  $value = substr($value, $nextWhitespace + 1);

                  # Get the temperature
                  $nextWhitespace = index($value, " ");
                  my $temperature = substr($value, 0, $nextWhitespace);
                  $value = substr($value, $nextWhitespace + 1);

                  # Get the voltage
                  $nextWhitespace = index($value, " ");
                  my $voltage = $value;

                  print STDERR " Temp: " . $temperature . "deg\n";
                  print STDERR " Humidity: " . $humidity . "%\n";
                  print STDERR " Voltage: " . ($voltage / 10) . "v]\n";

                  # Publish the temperature part of the data and the
                  # humidity part of the data
                  print STDERR "Publishing node data to MQTT\n";
                  $self->{_mqtt}->publish("local/rfm12/nodes/" . $nodeNumber . "/temp", 1, 1, $temperature);
                  $self->{_mqtt}->publish("local/rfm12/nodes/" . $nodeNumber . "/humidity", 1, 1, $humidity);
                  $self->{_mqtt}->publish("local/rfm12/nodes/" . $nodeNumber . "/voltage", 1, 1, ($voltage/10));

                  # Update the database (if the last update wasn't very recently)
                  if (time() - $lastPubTimes[$nodeNumber] > 30) {
                     my $query_handle = $self->{_db}->prepare_cached('INSERT INTO temperature' . substr($nodeNumber, 1) . ' (`temperature`) VALUES (?)');
                     $query_handle->execute($temperature);
                     $query_handle->finish();
                     my $query_handle2 = $self->{_db}->prepare_cached('INSERT INTO humidity' . substr($nodeNumber, 1) . ' (`humidity`) VALUES (?)');
                     $query_handle2->execute($humidity);
                     $query_handle2->finish();
                     my $query_handle3 = $self->{_db}->prepare_cached('INSERT INTO wirelessnodes (`nodeid`, `voltage`) VALUES (?, ?)');
                     $query_handle3->execute($nodeNumber, $voltage);
                     $query_handle3->finish();
             
                     undef($query_handle);
                     undef($query_handle2);
                     undef($query_handle3);
                  } else {
                    print STDERR "(Probably duplicate message - ignoring)\n";
                  }
                  $lastPubTimes[$nodeNumber] = time();
                  undef($temperature);
                  undef($humidity);
                  undef($counter);
                  undef($voltage);
                }
                elsif ($nodeNumber eq "04") {
                  # Node 4 specific stuff here...
                  # For node 4 the data is: 
                  #  - the moisture/capacitance 
                  #  - battery voltage (x10, e.g. 33 = 3.3v)

                  # Get the moisture/capacitance
                  $nextWhitespace = index($value, ' ');
                  my $moisture = substr($value, 0, $nextWhitespace);
                  $value = substr($value, $nextWhitespace + 1);

                  # Get the voltage
                  $nextWhitespace = index($value, " ");
                  my $voltage = $value;

                  print STDERR " Moisture: " . $moisture . "\n";
                  print STDERR " Voltage: " . ($voltage / 10) . "v]\n";

                  # Publish the moisture part of the data and the
                  print STDERR "Publishing node data to MQTT\n";
                  $self->{_mqtt}->publish("local/rfm12/nodes/" . $nodeNumber . "/moisture", 1, 1, $moisture);
                  $self->{_mqtt}->publish("local/rfm12/nodes/" . $nodeNumber . "/voltage", 1, 1, ($voltage/10));

                  # Update the database (if the last update wasn't very recently)
                  if (time() - $lastPubTimes[$nodeNumber] > 5) {
                     my $query_handle = $self->{_db}->prepare_cached('INSERT INTO moisture (`moisture`) VALUES (?)');
                     $query_handle->execute($moisture);
                     $query_handle->finish();
                     my $query_handle2 = $self->{_db}->prepare_cached('INSERT INTO wirelessnodes (`nodeid`, `voltage`) VALUES (?, ?)');
                     $query_handle2->execute($nodeNumber, $voltage);
                     $query_handle2->finish();
             
                     undef($query_handle);
                     undef($query_handle2);
                  } else {
                    print STDERR "(Probably duplicate message - ignoring)\n";
                  }
                  $lastPubTimes[$nodeNumber] = time();
                  undef($moisture);
                  undef($voltage);
                }
                else {
                   print STDERR $value . "\n";
                   print STDERR "]\n";
                }
                # etc...
          
             undef($now_string);
             undef($substr);
             undef($nextWhitespace);
             undef($nodeNumber);
	  }
     } elsif ($value =~ /TEST/) {
       # Just print the value into the log so we can see it arriving,
       # and publish it to the test topic on the broker
       
       # Strip the prefix 'TEST: ' from the value
       $value = substr($value, 6);

       print STDERR "[Test message received: " . $value . "]\n";
       $self->{_mqtt}->publish("local/rfm12/nodes/test", 1, 0, $value);
     } elsif ($value =~ /RANDOMBYTES/) {
       # If the bytes are random noise (from some other wireless device)
       # just publish them to a topic in case we want to analyse them later

       # Strip the prefix 'RANDOMBYTES: ' from the value
       $value = substr($value, 13);

       # Strip the newline/carriage return from the end of the value
       $value =~ s/\s+$//;

       my $now_string = localtime;

       # Print the random bytes we've seen as a bit pattern
       print STDERR "[Random bytes received: " . $now_string . "\n";
       my $i = 0;
       my $stringval = "";

       for ($i = 0; $i < length($value); $i++) {
          my $x = substr($value, $i, 1);
          my @bitArray = split(//, unpack('b*', $x));
          my @reverseBitArray = reverse(@bitArray);
          $stringval = $stringval . join('', @reverseBitArray);
          undef($x);
          undef(@bitArray);
          undef(@reverseBitArray);
       }

       chomp($stringval);
       $stringval =~ s/\///g;

       my $bitPattern = "";

       for ($i = 0; $i < length($stringval); $i++) {
          print STDERR substr($stringval, $i, 1);
          $bitPattern = $bitPattern . substr($stringval, $i, 1);
          if (($i + 1) % 8 == 0) {
             print STDERR " ";
             $bitPattern = $bitPattern . " ";
          }
          if (($i + 1) % 64 == 0) {
             print STDERR "\n";
             $bitPattern = $bitPattern . "\n";
          }
       }

       print STDERR "]\n";
       $bitPattern = $bitPattern . "]\n";

       #print STDERR "Publishing random bytes.\n";
       #$self->{_mqtt}->publish("local/rfm12/randombytes", 1, 0, $value);
       print STDERR "Publishing random bytes as bitpattern.\n";
       $self->{_mqtt}->publish("local/rfm12/randombytesbitpattern", 1, 0, $bitPattern);

       undef($now_string);
       undef($bitPattern);
       undef($stringval);
       undef($i);
     } else {
       my $now_string = localtime;
       # Trim whitespace from the RHS
       $value =~ s/\s+$//;
       print STDERR "Other data received from RFM12 arduino at " . $now_string . ": \n'" . $value . "'\n";
       #$self->{_mqtt}->publish("Confirmation data received from the RFM12 Arduino at " . $now_string . ": " . $value, "local/rfm12/confirmations", 1, 0);
       #$self->{_mqtt}->publish("local/rfm12/confirmations", 1, 0, "Confirmation data received from the RFM12 Arduino at " . $now_string . ": " . $value);

       undef($now_string);
     }
  $leaveCount = Devel::Leak::CheckSV($handle);
  print STDERR "\nLEAVE: $leaveCount SVs\n";
  };
  if ($@) {
    print STDERR "EXCEPTION OCCURRED! [$@]\n";
    print STDERR "(Hopefully things will carry on OK though)\n";
    undef($value);
    $rc = 1;
  }

  undef($self);
  undef($value);
  return $rc;
}

sub disconnect
{
   my $self = shift||die;
   if($self->{_debug}){
      print "disconnecting\n";
   }
   $self->{_mqtt}->disconnect();
}

sub DESTROY
{
   my $self = shift||die;
   $self->disconnect();	
}

1;
