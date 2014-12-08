#!/usr/bin/perl
use lib '/home/mwhitehead/Web/apps/mqtt-perl/';
print "Content-Type: text/plain", "\n\n";
use MQTT::Client();

my $mqtt = MQTT::Client->new({
   brokerIP => "localhost",
   clientID => "websiteclient8",
   callback_publish => \&publish_callback
});
# To perform a 'one-shot' publish to a topic:
$mqtt->connect();
$mqtt->subscribe("local/rfm12/randombytes",0);
$mqtt->listen_for_publishes();


sub publish_callback {
   print "Publish received\n";
   my $topic = shift;
   my $msg = shift;
   #print "-->" . $msg . "<--\n";

   my $i = 0;
   my $stringval = "";
 
   for ($i = 0; $i < length($msg); $i++) {
     my $x = substr($msg, $i, 1);
     @bitArray = split(//, unpack('b*', $x));
     @reverseBitArray = reverse(@bitArray);
     #print "@reverseBitArray";
     $stringval = $stringval . join('', @reverseBitArray);
   }

   chomp($stringval);
   $stringval =~ s/\///g;

   for ($i = 0; $i < length($stringval); $i++) {
      print substr($stringval, $i, 1);
      if (($i + 1) % 8 == 0) {
        print " ";
      }
      if (($i + 1) % 64 == 0) {
        print "\n";
      }
   }

   print "\n";
   if ($msg eq 'quit') {
      $mqtt->disconnect();
      exit 0;
   }
   # After getting 1 message, quit
   $mqtt->disconnect();
   exit 0;
}
