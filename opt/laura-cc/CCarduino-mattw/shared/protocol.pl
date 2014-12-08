# protocol.pl

# message generation functions

# Andy Stanford-Clark 26-Mar-99

# updated to version 2: 6-Jul-99

####################################################################

# parameters and useage examples

# all of these functions return 0 for successful completion, or undef if there was
# a TCP transmission problem


# CONNECT
# connect (UID, keepAlive, cleanStart
#              {, Will_QoS, Will_retain, Will_topic, Will_message});
#               
# &connect("Andy1",0,0,  0,0,"obituary","I have died");


# CONNACK
# connack (return_code)
#
# &connack(0);


# PUBLISH
# publish (Subject, DUP, QoS, MID, RETAIN, content)
# &publish("a.b",0,1,$MID,0,"Hello World!");


# PUBACK
# puback (Message_ID)
# &puback($MID);


# PUBREC/PUBREL/PUBCOMP
# &pubrec($MID);
# &pubrel($MID);
# &pubcomp($MID);


# SUBSCRIBE
# array of topic name and requested QoS pairs
# @subjects = ("a.b",1,"b.c",0,"d.e",2)
# &subscribe (messageID,@subjects)


# SUBACK
# array of granted QoS's, one per subscribed topic
# suback(messageID,@grantedQoS)

# UNSUBSCRIBE
# &unsubscribe (messageID,@topics)

# UNSUBACK
# &unsuback (messageID)

# PINGREQ
# &pingreq 

# PINGRESP
# &pingresp

# DISCONNECT
# &disconnect




####################################################################



sub connect
{
  # connect a client to a broker
  local($UID);
  local($Will_flag, $Will_Qos, $Will_retain, $Will_topic, $Will_message);


  # parameters are unique client name, 
  # keep alive timer, and clean start flag
  # and then optionally:
  # Will_QoS, Will_retain, Will_topic, Will_message

  ($UID, $keepAlive, $cleanStart, $Will_QoS, $Will_retain,
                                      $Will_topic, $Will_message) = @_;

  if ($Will_topic ne "")
  {
    $Will_flag = 1;
    $logstring = "topic: $Will_topic message: '$Will_message' QoS: $Will_QoS RETAIN: $Will_retain";
  }
  else
  {
    $logstring = "none";
  }

  &protlog("tx CONNECT: name:$protocol_name[$version],version:$version,".
          "TNCOMP:0,keep alive:$keepAlive, clean start:$cleanStart,".
          "UID:$UID Will: $logstring");


  print "CONNECT\n" if $debug;

  # CONNECT, DUP=x, QoS=x, SEC/RETAIN=x
  &fixed($CONNECT,0,0,0);
  
  # protocol name
  $variable = &toUTF($protocol_name[$version]);
  # protocol version
  $variable .= &tobyte($version);

  # Subject compression and clean start flag
  if ($Will_flag)
  {
      $variable .= &tobyte(0+$cleanStart*2+
                          4+
                          $Will_QoS*8+
                          $Will_retain*32);
      #print "byte is ",(0+$cleanStart*2+4+$Will_QoS*8+$Will_retain*32),"\n";

      # keep-alive timer
      $variable .= &toword($keepAlive);
  }
  else
  {
    # Subject compression and clean start flag
    $variable .= &tobyte(0+$cleanStart*2+0*4);
    # keep-alive timer
    $variable .= &toword($keepAlive);
  }


  # unique identifier
  $payload = &toUTF($UID);

  if ($Will_flag)
  {
    $payload .= &toUTF($Will_topic);
    $payload .= &toUTF($Will_message);
  }

  return (&build);
}


####################################################################


sub connack
{
  # connect acknowledge from broker to client
  print "CONNACK\n" if $debug;

  # parameter: return code to be sent

  local ($rc) = @_;

  &protlog("tx CONNACK: TNCOMP:0,rc:$connackRC{$rc}");

  &fixed($CONNACK,0,0,0);

  # COMPSUB flag
  $variable=&tobyte(0);
  # connect return code
  $variable .= &tobyte($rc);

  # no payload
  $payload="";

  return (&build);
}


####################################################################

sub publish
{
  local($subject,$DUP,$QoS,$MID,$content,$RETAIN);

  # publish a message to a named Subject
  print "PUBLISH\n" if $debug;


  # parameters Subject, DUP, QoS, MID, RETAIN, content
  # MID is ignored for QoS0 messages
  ($subject,$DUP,$QoS,$MID,$RETAIN,$content) = @_;

  &protlog("tx PUBLISH: MID:$MID,DUP:$DUP,QoS:$QoS,RETAIN:$RETAIN,$subject,$content");


  &fixed($PUBLISH,$DUP,$QoS,$RETAIN);

  # subject name
  $variable = &toUTF($subject);
  # message ID for QoS1 and QoS2
  if ($QoS==1 || $QoS==2)
  {
    $variable .= &toword($MID);
  }

  $payload = $content;

  return (&build);
}


####################################################################

sub newMID
{ 
  # returns a new Message ID
  $OTWP_messageID++;
  return $OTWP_messageID;
}

####################################################################



sub puback
{
  # acknowledge a publish with QoS 1
  # note that the DUP flag is not used in the protocol... we decided that
  # it is not needed. However, there is still a place for it, so we just
  # set it to 0

  print "PUBACK\n" if $debug;

  # parameter is Message ID from PUBLISH
  local($MID) = @_;

  &protlog("tx PUBACK: MID:$MID");

  &fixed($PUBACK,0,0,0);

  # message ID
  $variable = &toword($MID);

  $payload = "";

  return (&build);
}


####################################################################



sub pubrec
{
  # acknowledge a publish with QoS 2
  # note that the DUP flag is not used in the protocol... we decided that
  # it is not needed. However, there is still a place for it, so we just
  # set it to 0

  print "PUBREC\n" if $debug;

  # parameter is Message ID from PUBLISH
  local($MID) = @_;

  &protlog("tx PUBREC: MID:$MID");

  &fixed($PUBREC,0,0,0);

  # message ID
  $variable = &toword($MID);

  $payload = "";

  return (&build);
}


####################################################################


sub pubrel
{
  # acknowledge a PUBREC in the QoS 2 protocol flow
  # note that the DUP flag is not used in the protocol... we decided that
  # it is not needed. However, there is still a place for it, so we just
  # set it to 0

  print "PUBREL\n" if $debug;

  # parameter is Message ID from PUBLISH
  local($MID) = @_;

  &protlog("tx PUBREL: MID:$MID");

  &fixed($PUBREL,0,0,0);

  # message ID
  $variable = &toword($MID);

  $payload = "";

  return (&build);
}


####################################################################


sub pubcomp
{
  # acknowledge a PUBREL in the QoS 2 protocol flow
  # note that the DUP flag is not used in the protocol... we decided that
  # it is not needed. However, there is still a place for it, so we just
  # set it to 0

  print "PUBCOMP\n" if $debug;

  # parameter is Message ID from PUBLISH
  local($MID) = @_;

  &protlog("tx PUBCOMP: MID:$MID");

  &fixed($PUBCOMP,0,0,0);

  # message ID
  $variable = &toword($MID);

  $payload = "";

  return (&build);
}


####################################################################


sub pubackn
{
  # send one of the QoS 2 acknowledgements

  # parameters are n (1-3), Message ID
  local($num,$MID) = @_;
  local ($msg);

  &protlog("tx PUBACK$num: MID:$MID");

  print "PUBACKN($num)\n" if $debug;

  if ($num==1)
  {
    $msg=$PUBACK1;
  }
  elsif ($num==2)
  {
    $msg=$PUBACK2;
  }
  elsif ($num==3)
  {
    $msg=$PUBACK3;
  }
  else
  {
    die "invalid PUBACKn code: $num\n";
  }

  &fixed($msg,0,0,0);

  # message ID
  $variable = &toword($MID);

  $payload = "";

  return (&build);
}


####################################################################


sub subscribe
{
  local($MID,$tmp,@tmpsub,@subject);


  # subscribe to an array of Topic Names
  # we also have a Message ID passed in, and
  # each topic name has an associated Requested QoS
  # so the array has topic name, requested QoS, topic name, requested QoS...
  ($MID,@subject) = @_;

  @tmpsub=@subject;

  while (@tmpsub)
  {
    $tmp .= "\t".$tmpsub[0]." [".$tmpsub[1]."]\n";
    # now pop the first two values off the front
    shift(@tmpsub);
    shift(@tmpsub);
  }
  &protlog("tx SUBSCRIBE: MID:$MID\n".$tmp);


  local ($topic);

  print "SUBSCRIBE\n" if $debug;


  # QoS 1
  &fixed($SUBSCRIBE,0,1,0);
  # message ID
  $variable = &toword($MID);

  $payload = "";

  while (@subject)
  {
    $topic = shift(@subject);
    $payload .= &toUTF($topic);

    $rQoS = shift(@subject);
    $payload .= &tobyte($rQoS);
  }

  return (&build);
}


####################################################################


sub suback
{
  # subscribe acknowledge from broker to client
  local($MID,@granted);

  # parameter is the message ID that we are returning from the
  # corresponding SUBSCRIBE
  ($MID,@granted) = @_;

  print "SUBACK\n" if $debug;

  &protlog("tx SUBACK");

  &fixed($SUBACK,0,0,0);

 
  $variable=&toword($MID);
  # convert the QoS numbers into binary 0x32 -> 0x02
  $payload=join('',map {chr(ord($_)-ord('0'))} @granted);
  

  return (&build);
}

####################################################################


sub unsubscribe
{

  # unsubscribe from an array of Topic Names
  # we also have a Message ID passed in
  local ($MID,@subject) = @_;
  local ($topic);

  &protlog("tx UNSUBSCRIBE: ".join(",",@subject));


  print "UNSUBSCRIBE\n" if $debug;

  # QoS 1
  &fixed($UNSUBSCRIBE,0,1,0);
  # message ID
  $variable = &toword($MID);


  $payload = "";

  while (@subject)
  {
    $topic = shift(@subject);
    $payload .= &toUTF($topic)
  }

  return (&build);
}


####################################################################


sub unsuback
{
  # unsubscribe acknowledge from broker to client

  # parameter is the message ID that we are returning from the
  # corresponding UNSUBSCRIBE
  local ($MID) = @_;

  print "UNSUBACK\n" if $debug;

  &protlog("tx UNSUBACK");

  &fixed($UNSUBACK,0,0,0);

  $variable=&toword($MID);

  $payload="";

  return (&build);
}

####################################################################

sub pingreq
{
  # request a ping response

  print "PINGREQ\n" if $debug;

  &protlog("tx PINGREQ");

  &fixed($PINGREQ,0,0,0);

  $variable="";
  $payload="";

  return (&build);
}

####################################################################

sub pingresp
{
  # respond to a ping request

  print "PINGRESP\n" if $debug;

  &protlog("tx PINGRESP");

  &fixed($PINGRESP,0,0,0);

  $variable="";
  $payload="";

  return (&build);
}

####################################################################

sub disconnect
{
  # send a disconnect packet

  print "DISCONNECT\n" if $debug;

  &protlog("tx DISCONNECT");

  &fixed($DISCONNECT,0,0,0);

  $variable="";
  $payload="";

  return (&build);

}

####################################################################

1
