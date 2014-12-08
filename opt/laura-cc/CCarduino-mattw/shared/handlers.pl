# handlers.pl

# Andy Stanford-Clark 26-Mar-99

####################################################################

# handler functions for the various message types
# de-marshals variables out of message packet

####################################################################

#
# sub handle_connack
# no return
#
# sub handle_publish
# return ($subject,$MID,$content);
#
# sub handle_puback
# return ($MID);
#
# sub handle_pubrec
# no return
#
# sub handle_pubrel
# no return
#
# sub handle_pubcomp
# no return
#
# sub handle_suback
# return ($MID,@grantedQoS)
#
# sub handle_unsuback
# return ($MID)
#
# sub handle_pingresp
# no return
#



####################################################################

# client

sub handle_connack
{
  my ($buf)=@_;
  my ($TNCOMP,$rc);
  print "CONNACK\n" if $msg_trace;

  $TNCOMP=&frombyte($buf);
  $buf=substr($buf,1);
  $rc=&frombyte($buf);


  print "rc: $rc\n" if $debug;
  &protlog("rx CONNACK: rc:$connackRC{$rc}");


  return ($TNCOMP,$rc);
}

####################################################################

# broker, client

sub handle_publish
{
  my ($buf)=@_;
  my ($subject,$MID,$content);

  $subject=&fromUTF($buf);
  print "PUBLISH $subject\n" if $msg_trace;


  $buf=substr($buf,length($subject)+2);

  $MID=0;
  if ($QoS==1 || $QoS==2)
  {
    # there is a message ID
    $MID=&fromword($buf);
    $buf=substr($buf,2);
  }

  $content=$buf;

  print "Subject: <$subject>\n" if $debug;
  if ($QoS==1 || $QoS==2)
  {
    print "Message ID: $MID\n" if $debug;
  }
  print "content: <$content>\n" if $debug;

  # $MID will just be zero (which is invalid) if QoS==0

  &protlog("rx PUBLISH: MID:$MID,$DQS,topic:$subject,content:$content");

  return ($subject,$MID,$content);

}

####################################################################

# client, broker

sub handle_puback
{
  my ($buf)=@_;
  my ($MID);
  print "PUBACK\n" if $msg_trace;

  $MID=&fromword($buf);

  print "Message ID: $MID\n" if $debug;

  &protlog("rx PUBACK : MID:$MID");

  return ($MID);
}

####################################################################


# client, broker

sub handle_pubrec
{
  my ($buf)=@_;
  my ($MID);

  print "PUBREC\n" if $msg_trace;


  $MID=&fromword($buf);

  print "Message ID: $MID\n" if $debug;

  &protlog("rx PUBREC: MID:$MID");

  return ($MID);
}

####################################################################

# broker, client

sub handle_pubrel
{
  my ($buf)=@_;
  my ($MID);

  print "PUBREL\n" if $msg_trace;


  $MID=&fromword($buf);

  print "Message ID: $MID\n" if $debug;

  &protlog("rx PUBREL: MID:$MID");

  return ($MID);
}

####################################################################

# client, broker

sub handle_pubcomp
{
  my ($buf)=@_;
  my ($MID);

  print "PUBCOMP\n" if $msg_trace;

  $MID=&fromword($buf);

  print "Message ID: $MID\n" if $debug;

  &protlog("rx PUBCOMP: MID:$MID");

  return ($MID);
  
}

####################################################################

# client

sub handle_suback
{
  my ($buf)=@_;
  my ($MID,@granted,$gQoS);

  print "SUBACK\n" if $msg_trace;

  # return the MID, and the granted QoS values
  $MID=&fromword($buf);
  $buf=substr($buf,2);
  print "Message ID: $MID\n" if $debug;

  # now pull off the vector of granted QoS values
  while (length($buf)!=0)
  {
    $gQoS = &frombyte($buf);
    print "granted QoS: $gQoS\n" if $debug;
    push(@granted,$gQoS);
    $buf=substr($buf,1);
  }
  &protlog("rx SUBACK: MID:$MID granted QoS:".join(",",@granted));
  return ($MID,@granted);

}

####################################################################

# client

sub handle_unsuback
{
  my ($buf)=@_;
  my ($MID);

  print "UNSUBACK\n" if $msg_trace;

  $MID=&fromword($buf);
  $buf=substr($buf,2);
  print "Message ID: $MID\n" if $debug;

  &protlog("rx UNSUBACK: MID:$MID");
  return ($MID);
}

####################################################################

sub handle_illegal
{
  # trap message type 0, which is reserved as "undefined" in V2
  print "received illegal message type 0\n";
  exit;
}

####################################################################

sub handle_disconnect
{
  print "DISCONNECT\n" if $msg_trace;

  &protlog("rx DISCONNECT");

  return ("");
}

####################################################################

sub handle_pingresp
{
  print "PINGRESP\n" if $msg_trace;

  &protlog("rx PINGRESP");

  return ("");
}

####################################################################


sub getmsg
{
  # get a message from TCP and then process it
  print "waiting for message...\n" if $debug;
  my ($rin);
  my ($got_byte);
  my ($connected,$rc);
  my ($now,$sleep_time);
  
  if (1)
  {
    # sleep for some period of time, or until a byte comes in, whichever
    #happens first
    
    $got_byte=0;
    
    while (!$got_byte)
    {
      # work out how long we're going to sleep for
      $now = time;
      if (!defined $get_timeout)
      {
        $get_timeout = $now + $keepalive_timer;
        print "first time around - set timeout to $keepalive_timer\n" if $debug;
      }
      elsif ($get_timeout - $now <= 0)
      {
        # time to do a ping
        print "timeout (",$get_timeout-$now,")\n" if $debug;
        
        $connected=1; # unless we hear otherwise                                                                   
        $rc = &pingreq;
        print "ping sent at ",scalar gmtime(),"\n" if $debug;
        

        # if we're interested in things timing out, call back to the app

        if ($keepalive_timeout_callback)
        {
          &keepalive_timeout_function;
        }

       
        #$going_round_count = 0;
        #while ($connected && !defined $rc)                                                                
        if (!defined $rc)
        {                                                                                                          
          print "** lost connection to broker **\n";                                                               
          sleep 5;                            
          print "going round again... (",++$going_round_count,")\n" if $debug;
          # no point in doing this here...
          #$connected = &client_connect($ID);                                                                       
          return undef;
        }
        
        # reset timer
        $get_timeout = $now + $keepalive_timer; 
        print "timeout reset to $keepalive_timer\n" if $debug;
      }
      # OK - now we're ready to roll
          
      $rin="";
      vec($rin,fileno($TCPHANDLE),1)=1;  
      
      $sleep_time = $get_timeout - time;
      if ($sleep_time < 0)
      {
        print "sleep time was $sleep_time - resetting to 0\n";
        sleep 5;
        
        $sleep_time = 0;
      }
      
      print "waiting for a byte for $sleep_time sec at ",scalar gmtime(),"\n" if $debug;
      
      $nfound = select($rout=$rin,undef,undef,$sleep_time); 
      
      if ($nfound == 0)
      {
        # um.... 
        # do nothing - just go round again
        print "timeout - going round again...\n" if $debug;        
      }
      else
      {
        $got_byte=1;
        #print "(nfound=$nfound) got a byte\n"
      }
    }
  }
   
  # OK - now we've got a byte ready to read... 
  
  $rawbuffer = &TCPread($TCPHANDLE);
  # returns undef if there was an error

  # rawbuffer does not get changed, and so can be forwarded as-is by the broker

  if (!defined $rawbuffer)
  {
    # socket was closed
    undef $type;
    return undef;
  }
  else
  {
    # returns an array of values
    return &process($rawbuffer);
  }
}

####################################################################

sub process
{
  # unpack the data in a received message
  my ($buf) = @_;
  my ($fixed);
  my ($byte,$multiplier,$offset);
  
  $fixed = ord(substr($buf,0,1));
  $type =($fixed & 0xF0)/16;
  $DUP = ($fixed & 0x08)/8;
  $QoS = ($fixed & 0x06)/2;

  $RETAIN = ($fixed & 0x01);
  

  # make a trace string out of these for everyone in handlers.pl to use
  #$DQS = "DUP:$DUP,QoS:$QoS,SEC:$SEC";
  $DQS = "DUP:$DUP,QoS:$QoS";

  # count the bytes as we decode them
  # set to the first byte of the remaining length field
  $offset=1;

  $length=0;
  $multiplier=1;
  do 
  {
    $byte = ord(substr($buf,$offset,1));
    $length += ($byte & 0x7F) * $multiplier;
    $multiplier *= 128;
    $offset++;
  } while ($byte & 0x80);
  print "TYPE: $type\tDUP:$DUP QoS:$QoS RETAIN:$RETAIN remlength: $length\n" if $debug;
  print "offset=$offset\n" if $debug;



  # global $buffer contains the variable header and payload sections of the message
  # length is the size of this buffer

  # we have to also skip over the extension bytes of the
  # remaining length field, so offset has been calculated during the decoding

  $buffer = substr($buf,$offset);

  # call the handler function for this type of message, as defined in %table
  # handlers are in handlers.pl

  print "Received: " if $debug;
  # returns an array of values
  return &{$table{$type}}($buffer);

}

####################################################################

1
