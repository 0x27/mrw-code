# Read the gas photoreflectivity sensor and publish whenever a tick has
# occurred to an MQTT broker and record it in our database.

# Matt Whitehead 20-Feb-13

########################################################################

$your_name = "mattw2";

$serial_port = "/dev/ttyUSBGAS";

$broker = "127.0.0.1";

$retry_attempts = 10;

$retries = 0;

########################################################################

use GasMonitor;
use DBI;
use POSIX strftime;

$0 = "gasmonitor";

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

  open (SERIAL,"+<$serial_port") || $retries++;

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
     $cc = new GasMonitor(broker=>$broker,name=>$your_name,debug=>1);

     while (<SERIAL>)
     {
        $cc->publish_gas($_);
     }
  }
  else
  {
        print STDERR strftime ("%d%b%Y-%H:%M:%S - Can't open serial port, retrying in 1 second...\n",localtime(time()));
        sleep(1);
        $serial_port = "/dev/ttyUSB2";
  }
}
                                
print STDERR strftime ("%d%b%Y-%H:%M:%S",localtime(time()))." - Tried ".$retries." times so quitting\n";

                                
