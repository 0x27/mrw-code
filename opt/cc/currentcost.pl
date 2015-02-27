# read the currentcost meter and publish the power in KW, to the nearest
# 100 watts, to an MQTT broker

# Andy S-C 16-Sep-07

########################################################################

$your_name = "mattw";

$serial_port = "/dev/ttyUSBCC";

$broker = "127.0.0.1";

$retry_attempts = 10;

$retries = 0;

########################################################################

use CurrentCost;
use DBI;
use POSIX strftime;

$0 = "currentcost";

# Wait until the nanobroker has started and the /ttyUSB device is ready
print STDERR "Waiting for nanobroker to be started...\n";
$file = '/tmp/nanobroker.log';
sleep 5 until -e $file;
print STDERR "Nanobroker started. Waiting for $serial_port to exist...\n";
sleep 5 until -e $serial_port;
print STDERR "Serial port ready. Starting application\n";

while ($retries < $retry_attempts)
{
  print STDERR "Trying serial port ".$serial_port."...\n";

  open (SERIAL, "<", "$serial_port") || $retries++;

  if (<SERIAL>)
  {
    $retries = 0;
    print STDERR "Serial port ".$serial_port." OK after ".$retries." retries\n";
  }
  else
  {
    print STDERR strftime ("%d%b%Y-%H:%M:%S - Serial port not OK\n",localtime(time()));
  }

  if ($retries == 0)
  {
     $cc = new CurrentCost(broker=>$broker,name=>$your_name,debug=>1);

     while (<SERIAL>)
     {
        print $_ . "\n";
        my $is_ascii = $_ =~ /^[\x00-\x7F]*\z/;

        if ($is_ascii) {
          $cc->publish_all($_);
        } else {
          print "Not ascii. Restart?\n";
          sleep 10;
        }
     }
  }
  else
  {
        print STDERR strftime ("%d%b%Y-%H:%M:%S - Can't open serial port, retrying in 1 second...\n",localtime(time()));
        sleep(1);
        $serial_port = "/dev/ttyUSBCC";
  }
}
                                
print STDERR strftime ("%d%b%Y-%H:%M:%S",localtime(time()))." - Tried ".$retries." times so quitting\n";

                                
