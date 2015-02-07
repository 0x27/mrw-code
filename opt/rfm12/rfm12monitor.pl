# Read the gas photoreflectivity sensor and publish whenever a tick has
# occurred to an MQTT broker and record it in our database.

# Matt Whitehead 20-Feb-13

########################################################################

$your_name = "mattw3";

$serial_port = "/dev/ttyUSBRFM";

$broker = "127.0.0.1";

$retry_attempts = 50;

$retries = 0;

$subscriberClientID = "rfmSubID";
$publisherClientID = "rfmPubID";

use bytes;

########################################################################

use RFM12Monitor;
use DBI;
use POSIX strftime;
use Fcntl;

# Wait until the nanobroker has started and the /ttyUSB device is ready
print STDERR "Waiting for nanobroker to be started...\n";
$file = '/tmp/nanobroker.log';
sleep 5 until -e $file;
print STDERR "Nanobroker started. Waiting for $serial_port to exist...\n";
sleep 5 until -e $serial_port;
print STDERR "Serial port ready. Starting application\n";

my $cc = new RFM12Monitor(broker=>$broker,name=>$your_name,debug=>1,pub=>1);

while ($retries < $retry_attempts)
{

  if ($retries == 0)
  {

     # Fork a separate process to receive MQTT commands to pass on to the RFM12 chip
     my $pid = fork();

     if ($pid) {
        # parent
        $0 = "rfm12listener";
        print STDERR "Trying serial port ".$serial_port." for reading...\n";
        open (my $serialFH,"<$serial_port") || die;

        if (my $nextLine = <$serialFH>) {
           print STDERR "Serial port ".$serial_port." OK after ".$retries." retries\n";
        } else {
           die "Couldn't open " . $serial_port . " for reading. Exiting";
        }
        $cc->connect($publisherClientID);
        while (my $nextLine = <$serialFH>) {
           $rc = $cc->publish_rfm12($nextLine);
           
           if ($rc) {
              print STDERR "RC problem - pausing for a minute\n";
              sleep(60);
              print STDERR "Closing serial port\n";
              close($serialFH);
              print STDERR "Re-opening serial port\n";
              open($serialFH,"<$serial_port") || die;
              print STDERR "Port re-opened. Re-connecting to MQTT...\n";
              $cc->disconnect();
              $cc->connect($publisherClientID);
              print STDERR "Reconnected to broker\n";
           }
        } 
     } elsif ($pid == 0) {
        # child
        $0 = "rfm12cmdserver";
        print STDERR "Trying serial port ".$serial_port." for writing...\n";
        open (SERIALB,">>$serial_port") || die;

        # This disables buffering in the serial USB port, causing
        # any writes to be pushed down to the RFM12 arduino immediately
        $old_fh = select(SERIALB);
        $| = 1;
        select($old_fh);

        # For the subscriber, set a keep alive > 0 (otherwise it 
        # thrashes the broker for some reason)
        $cc->connect($subscriberClientID, \&publicationReceived);
        $cc->subscribe();
        $cc->waitForMessages();
        exit 0;
     } else {
        die "couldnt fork: $!\n";
     }
  }
  else
  {
        print STDERR strftime ("%d%b%Y-%H:%M:%S - Can't open serial port, retrying in 30 second...\n",localtime(time()));
        sleep(30);
        $serial_port = "/dev/ttyUSB1";
  }
}
                                
print STDERR strftime ("%d%b%Y-%H:%M:%S",localtime(time()))." - Tried ".$retries." times so quitting\n";

sub publicationReceived {
   my $topic = shift;
   my $data = shift;

   print "Passing command to RFM12 chip: '" . $data . "'\n";
   # Write the command to the serial port
   print SERIALB $data;
}
                                
