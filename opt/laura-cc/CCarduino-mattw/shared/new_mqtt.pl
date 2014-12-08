#!/usr/local/bin/perl

# Perl MQTT client implementation



# "Quick and dirty implementation of the Argo OTWP protocol."
# Andy Stanford-Clark 26-Mar-99


# re-jigged May '06


####################################################################

use Socket;


require "../shared/variables.pl";

require "../shared/helpers.pl";
require "../shared/protocol.pl";
require "../shared/handlers.pl";
require "../shared/tcp.pl";
require "../shared/application.pl";


####################################################################


if ($#ARGV != 0 && $#ARGV != 1)
{
  &usage;
}
else
{
  print "require: '$ARGV[0]', UID: '$ARGV[1]'\n" if $debug;

  require $ARGV[0];

  $client_UID=$ARGV[1];


  &main;
}



####################################################################


sub usage
{
  print "usage: mqtt.pl app.pl {UID}\n";
}

####################################################################

