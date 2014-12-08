# micro pub (upub.pl)

# publishes an mqtt message to a broker in the most unobtrusive, yet still readable, 
# way I can think of.
# only does qos 0

################################################################################

# &upub(clientid,broker,port,topic,qos,retain,content);

# if clientid is null (""), it makes a random one for you


# Andy S-C 16-Jun-05

# Copyright IBM Corporation 2005

# This code is provided "as-is", and comes with no warranty as to its correct
# function or usefulness for any particular purpose.


################################################################################

# test:
#&upub("a","127.0.0.1",1883,"A",0,"n","123");



################################################################################


sub upub
{
  my ($clientid,$broker,$port,$topic,$qos,$retain,$content) = @_;

  if ($qos != 0)
  {
    print "can only do qos0 - sorry!\n";
    exit;
  }

  if (!$clientid)
  {
    # make a random client id:
    # upub_{last 3 digits of time}_{random 1-99}_{process id last 3 digits}
    $clientid = "upub_".(time %1000)."_".int(rand(100))."_".($$ % 1000); 
  }

  # construct mqtt message

  ###########
  # connect #
  ###########

  # fixed header: msg type (4) dup (1) qos (2) retain (1)
  $fixed = chr(1*16);

  # variable header: protocol name, protocol version, connect flags, keepalive
  $variable = &UTF("MQIsdp").chr(3).chr(2).chr(0).chr(0);

  # payload: client ID
  $payload = &UTF($clientid);

  # add in the remaining length field and fix it together
  $msg = $fixed . &remaining_length(length($variable)+length($payload)) . $variable . $payload;

  ###########
  # publish #
  ###########

  # fixed header: msg type (4) dup (1) qos (2) retain (1)
  $fixed = chr(3*16 + $qos*2 + ( ($retain eq "y" || $retain eq "1") ?1:0));

  # variable header 
  $variable = &UTF($topic);
  if ($qos == 1)
  {
    $variable .= chr(0).chr(1);
  }
 

  # payload
  $payload = $content;

  $msg .= $fixed . &remaining_length(length($variable)+length($payload)) . $variable . $payload;

  ##############
  # disconnect #
  ##############

  if (1)
  {
    # fixed header: msg type (4) dup (1) qos (2) retain (1)
    $fixed = chr(14*16);

    $msg .= $fixed . chr(0);
  }



  # now send connect, publish, disconnect in one shot


  if (0)
  {
    @msg = split(//,$msg);
    foreach $byte (@msg)
    {
      printf "%02X\n",ord($byte);
    }
    #exit;
  }

  

  if (0)
  {
    #use IO; 
    $connection = new IO::Socket::INET("$broker:$port") or die "can't connect to $broker:$port";

    if ($connection)
    {
      $connection->send($msg);
    }
    # sleep for a fraction to make sure the socket doesn't close before the messages arrives
    select undef,undef,undef,0.25;

    $connection->close;
  }
  else
  {
    use Socket;
    socket(S, AF_INET, SOCK_STREAM, getprotobyname('tcp')) || die "socket: $!";

    $a = sockaddr_in($port,inet_aton($broker));
    #print "port '$port' address '$broker' sock struct is length ",length($a),"\n";

    $connection = connect(S, sockaddr_in($port,inet_aton($broker))); # || die "connect: $!";
   
    if ($connection)
    {
      select S; $|=1;
      select STDOUT;
      print S $msg;
    }
    # sleep for a fraction to make sure the socket doesn't close before the messages arrives
    select undef,undef,undef,0.3;

    close S;
  }


  # job done
}


################################################################################

sub UTF
{
  # return the UTF-8 encoded version of the parameter
  my ($string) = @_;
  my ($length) = length($string);

  return chr($length/256).chr($length%256).$string;
}
  

################################################################################

sub remaining_length
{
  # return the remaining length field bytes for an integer input parameter
  my ($x) = @_;
  my ($rlf,$digit);

    do
    {
      $digit = $x % 128;
      $x = int($x/128);

      # if there are more digits to encode, set the top bit of this digit
      if ( $x > 0 )
      {
        $digit |= 0x80;
      }

      $rlf .= chr($digit);
    } while ($x > 0);

    return $rlf;
}


################################################################################


1
