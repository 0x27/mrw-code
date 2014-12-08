# new micro pub (nupub.pl)

# Andy S-C 16-Jun-05

# new version 5-May-06


# publishes a series of mqtt messages to a broker in the most unobtrusive, 
# yet still readable, way I can think of.
# only does qos 0
# can do LWT

# if clientid is null (""), it makes a random one for you

################################################################################


# {optional}
# upub_lwt ($keepalive,$topic,$qos,$retain,$content);

# upub_connect ($clientid,$broker,$port)

# upub_publish ($topic,$retain,$content)


# {optional, though desirable if you set LWT}
# upub_disconnect


# semantic oddities

# upub_publish will attempt to reconnect if it detects a problem with the connection
# upub_connect with empty parameter list is "reconnect" to where you connected earlier

# NB do not just call upub_connect with NO parameters (i.e. "upub_connect;" ) ...
# that does something horrible and you get the stack contents of the current function
# passed into connect, which is NOT what you want, believe me!
# so it must be called like this: "upub_connect()"... though as it's already in publish
# you almost certainly won't need to anyway!


# global variables:
# $upub_keepalive, $upub_flags, and $upub_strings,
# $upub_clientid, $upub_broker, $upub_port


################################################################################

# test:

# nice simple example
if (0)
{
  if (&upub_connect ("asc","10.0.0.14",1883))
  {

    for ($i=0;$i<10;$i++)
    {
      &upub_publish ("test","n","hello Andy [$i] $$");
      sleep 2;
    }
  }
  &upub_disconnect;
}


# more complicated example, with LWT and forcing a reconnect

if (0)
{
  &upub_lwt (5,"lwt",0,0,"nupub died");

  if (&upub_connect ("asc","10.0.0.14",1883))
  {

    for ($i=0;$i<10;$i++)
    {
      print "sending message $i\n";
      if (!&upub_publish ("test","n","hello Andy [$i] $$"))
      {
        print "message $i may not have got through\n";

        # it's tempting to republish here, but we're not really allowed to
        # as qos0 is AT MOST once, and there's a CHANCE the first one got through!
      }
      if ($i == 4)
      {
        print "going to sleep....";
        sleep 10;
        print "awake\n";
      }
      sleep 2;
    }

    sleep 5;

    # if you comment this line out, it should fire LWT
    #&upub_ping;

    sleep 5;
 
    &upub_disconnect;
  }
  else
  {
    print "couldn't connect\n";
  }
} 

################################################################################
use Socket;

# disable the signal handler for sig pipe
$SIG{'PIPE'}='IGNORE';
    
    

# $upub_keepalive, $upub_flags, and $upub_strings are global

sub upub_lwt 
{
  my ($keepalive,$topic,$qos,$retain,$content) = @_;

  # note that although WE can't do qos>0, the broker, can, so we can set qos on LWT

  $upub_keepalive = $keepalive;
  
  # LWT flags: x, x, ret, qos qos, will, (cleanstart, 0) 
  $upub_flags = ( (lc($retain) eq "y" || $retain==1) ?1:0)*32 + $qos * 8 + 4;
  # now we can just bitwise OR this into the connect flags byte in connect

  # will topic, will content, both UTF encoded
  $upub_strings = &UTF($topic) . &UTF($content);
  # append this to the end of the payload in connect

  # if these variables are set, we'll use them, if not, their default values
  # will do the right things for us (so we don't need a flag to say "LWT set")
}

################################################################################


# returns true if connected, false if not

sub upub_connect
{
  my ($clientid,$broker,$port) = @_;
  my ($fixed,$variable,$payload,$msg,$rc);

  # if only the client id is null
  if (!$clientid && $broker && $port)
  {
    # make a random client id:
    # upub_{last 3 digits of time}_{random 1-99}_{process id last 3 digits}
    $clientid = "upub_".(time %1000)."_".int(rand(100))."_".($$ % 1000); 
  }


  # if there are NO parameters
  if ($#_ == -1)
  {
    # it's a reconnect, use the parameters we used last time
    $clientid = $upub_clientid;
    $broker = $upub_broker;
    $port = $upub_port;
    print "(reconnecting to $broker:$port as $clientid)\n";
  }
  else
  {
    # store the parameters we used, in case there's a reconnect
    $upub_clientid = $clientid;
    $upub_broker = $broker;
    $upub_port = $port;    
    print "connecting to $broker:$port as $clientid\n";
  }


  # construct mqtt message

  ###########
  # connect #
  ###########

  # fixed header: msg type (4) dup (1) qos (2) retain (1)
  $fixed = chr(1*16);

  # variable header: protocol name, protocol version
  $variable = &UTF("MQIsdp").chr(3);

  # cleverly, thanks to auto-vivification and guaranteed zeroing of new variables,
  # we don't actually need to test if LWT has been set or not

  # connect flags
  $variable .= chr(2 | $upub_flags);

  # keepalive (2 bytes)
  $variable .= chr($upub_keepalive/256) . chr($upub_keepalive%256);

  # payload: client ID
  $payload = &UTF($clientid) . $upub_strings;

  # add in the remaining length field and fix it together
  $msg = $fixed . &remaining_length(length($variable)+length($payload)) . $variable . $payload;


  if (socket(upub_S, PF_INET, SOCK_STREAM, getprotobyname('tcp')))
  {
    if (connect(upub_S, sockaddr_in($port,inet_aton($broker))))
    {
      select upub_S; $|=1;
      select STDOUT;
 
      # print returns true if successful
      $rc = print upub_S $msg;

      return $rc;
    }
  }
  return 0;
}


################################################################################

# returns true if write if "successful" (i.e. if write to socket works)

sub upub_publish
{
  my ($topic,$retain,$content) = @_;
  my ($qos) = 0;
  my ($fixed,$variable,$payload,$msg);

  ###########
  # publish #
  ###########

  # fixed header: msg type (4) dup (1) qos (2) retain (1)
  $fixed = chr(3*16 + $qos*2 + ( (lc($retain) eq "y" || $retain eq 1) ?1:0));

  # variable header 
  $variable = &UTF($topic);

  # NOT POSSIBLE!!! This is defensive coding gone too far!
  #if ($qos == 1)
  #{
  #  $variable .= chr(0).chr(1);
  #}
 

  # payload
  $payload = $content;

  $msg = $fixed . &remaining_length(length($variable)+length($payload)) . $variable . $payload;

  # send it
  
  # print returns true if successful
  # if unsuccessful, try to reconnect

  $rc = print upub_S $msg;

  if (!$rc)
  {
    # connect with empty parameters is "reconnect"
    &upub_connect(); 
  }

  return $rc;

}


################################################################################

# returns true if sent successfully, but not sure how useful that is!

sub upub_disconnect
{
  my ($fixed,$msg);

  ##############
  # disconnect #
  ##############

  if (1)
  {
    # fixed header: msg type (4) dup (1) qos (2) retain (1)
    $fixed = chr(14*16);

    $msg = $fixed . chr(0);
  }

  return (print upub_S $msg);

  # sleep for a bit to make sure the socket doesn't close before the messages arrives
  select undef,undef,undef,0.5;

  close upub_S;

}



################################################################################


# if we're doing LWT, we should include PING for completeness
# returns true if it was sent successfully, but that's not really an indicator
# of connectedness - you really need to wait for the pong to come back

sub upub_ping
{

  my ($fixed,$msg);

  ##############
  # pingreq    #
  ##############


  # fixed header: msg type (4) dup (1) qos (2) retain (1)
  $fixed = chr(12*16);

  $msg = $fixed . chr(0);

  return (print upub_S $msg);

  # note we don't make any attempt to read the pong out of the socket
  # so eventually, something horrible might happen (TCP fills up and the
  # Internet stops, or similar).
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
