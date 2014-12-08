# application.pl

# this is all the stuff in the sample subscribe / republish bridge
# application that shouldn't need to be change, leaving just a 
# very small customised piece for each new app

# Andy S-C May '06


# this is the "main" function that gets invoked from the application
# framework when it starts up. 


sub main
{

  # autoflush
  $|=1;

  # disable the signal handler for sig pipe
  $SIG{'PIPE'}='IGNORE';


  #&prepare_log("sublog.txt");


  &app_initialise;

  print "waiting for messages...\n" if $debug;
  $mqtt_state=$TCPCONNECTING;



  while (1)
  {

    if ($mqtt_state==$TCPCONNECTING) {
      &app_tcpconn;
      if ($mqtt_state == $TCPCONNECTING)
      {
        sleep 5;
        next;
      }
    }


    # this gives us a chance to call into the client app
    # let's do it after we get connected to the broker

    if ($mqtt_state==$SUBSCRIBING && $call_initialise)
    { 
      &initialise();
      $call_initialise = 0;
    }



    if ($mqtt_state==$PROTCONNECTING) {
      &app_protconn;
    }

    # publisher never gets into this state
    if ($mqtt_state==$SUBSCRIBING) {
      &app_subscribe;
    } 



    # now we've done the outbound stuff, sit and wait for a message

    print "------- state is: ",&decode_appstate,"\n" if $debug;

    @return=getmsg;

    if (!defined $return[0])
    {
      # problem with the connection
      print "connection to broker broke\n";
      $mqtt_state = $TCPCONNECTING;
    }
    else
    {
      # call the appropriate client handler function

      &{$app_jumptable{$type}}(@return);
    }
 

  } # wend 1 

}

####################################################################


sub app_initialise
{
  # set up all the up front stuff

  # define states

  $TCPCONNECTING=1;
  $PROTCONNECTING=2;
  $WAITCONNACK=3;
  $SUBSCRIBING=4;
  $WAITSUBACK=5;
  $RUNNING=6;

  # describe the application states for the decode_appstate function

  %app_states = ($TCPCONNECTING => "TCP connecting",
		             $PROTCONNECTING => "protocol connecting",
    		         $WAITCONNACK => "waiting for CONNACK",
                 $SUBSCRIBING => "subscribing",
                 $WAITSUBACK => "waiting for SUBACK",
		             $RUNNING => "running");

  # set up the jump table for received messages types

  # things that we could not receive as a subscriber are routed to 
  # the handle_illegal catcher

  # in the general case, though, when we publish from subscriber programs
  # this needs to be expanded to cope with puback and pubcomp messages.
 
  %app_jumptable = (
          $CONNECT=>\&handle_illegal,
          $CONNACK=>\&app_connack,
          $PUBLISH=>\&app_publish,
          $PUBACK=>\&pub_puback,
          $PUBREC=>\&pub_pubrec,
          $PUBREL=>\&app_pubrel,
          $PUBCOMP=>\&pub_pubcomp,
          $SUBSCRIBE=>\&handle_illeegal,
          $SUBACK=>\&app_suback,
          0 => \&handle_illegal,
          $UNSUBSCRIBE=>\&handle_illegal,
          $UNSUBACK=>\&app_unsuback,
          $PINGREQ=>\&app_pingreq,
          $PINGRESP=>\&app_pingresp,
          $DISCONNECT=>\&handle_illegal,
          15 => \&handle_illegal 
       );


  # set up the unique ID for this instance

  if ($client_UID ne "")
  {
    # if the client UID was specified, use it
    $ID=$client_UID;
  }
  else
  {
    # if it wasn't given to us, we'll make our own
    # add the process number in hex to the client ID string to make it unique
    #$ID=sprintf("Andy_sub_%0X",$$);

    $ID = "app_".(time %1000)."_".int(rand(100))."_".($$ % 1000); 

    # this is now officially deprecated, so let's exit, instead
    #print "please specify a client ID\n";
    #exit;
  }
  print "client UID is <$ID>\n" if $debug;

  # set the timeout time for retries
  $timeout = 10;
}

####################################################################

sub app_tcpconn
{
  # do what we have to do when we're in TCPCONNECTING state

  $sent_time=time;       

  $TCPHANDLE=&TCPconnect($broker);
  # TCPHANDLE is the socket to which all TCP I/O happens

  if (defined $TCPHANDLE)
  {
    print "socket connected\n" if $debug;
    $mqtt_state=$PROTCONNECTING;

    # time of last connect attempt. Zero it here to allow us to connect immediately
    # after we get the socket, otherwise it might have to wait up to 10 sec in 
    # a pathological disconnect/reconnect scenario
    $sent_time=0;
  }
  # otherwise we just stay in TCPconnecting state
}

####################################################################

sub app_protconn
{
  # do what we have to do when we're in PROTCONNECTING state

  if (time >= $sent_time + $timeout)
  {
    $sent_time=time;

    # ID, keepalive, cleanstart
    #@parms = ($ID,0,1);

    # UID, keepalive, cleanstart {Will_QoS,Will_retain,Will_topic,Will_message}
    @parms = ($ID,$keepalive_timer,1,0,1,"status/$application_name","0");


    if (!defined &connect(@parms))
    {
      print "error during connect\n" if $debug;
      $mqtt_state = $TCPCONNECTING;
    }
    else
    { 
      print "connect sent\n" if $debug;
      $mqtt_state = $WAITCONNACK;
    }
  }
  # the select will sleep for the remaining timeout period. So next time we come
  # back into here, we can just send another connect.
}

####################################################################

sub app_subscribe
{

  if (@topics)
  {

        $MID=&newMID;

        # @ topics defined elsewhere

        &subscribe($MID,@topics);
        print "subscribe MID:$MID\n" if $debug;
        $SUB_MID{$MID}=1;
        # put this message onto the outputQ, in case we need to retry
        $outputQ{$MID}->{type}=$SUBSCRIBE;
        $outputQ{$MID}->{time}=time;
        $outputQ{$MID}->{function}=\&subscribe;
        @{$outputQ{$MID}->{parms}}=($MID,@topics);

  
        $mqtt_state = $WAITSUBACK;
  }
  else 
  {
    $mqtt_state = $RUNNING;

    print "no topics to subscribe to\n" if $debug;

  }
}

####################################################################

sub app_connack
{
  # we received a CONNACK

  my ($TNCOMP,$rc)=@return;
  print "$connackRC{$rc}\n" if $debug;
  
  if ($mqtt_state != $WAITCONNACK)
  {
    print "received a CONNACK in state: ",&decode_appstate,"\n";
    exit;
  }

  if ($rc==0) 
  {
    print "Connected to broker!\n" if $debug;
    # for a publisher, should bump this state to "RUNNING"
    $mqtt_state = $SUBSCRIBING;
    #$mqtt_state = $RUNNING;

    # clear timer out for the next use
    $sent_time=0;

    # publish the birth certificate
    &publish("status/$application_name",0,0,0,1,"1");
  }
  else
  {
    print "connection failed: $connackRC{$rc}\n";
    $mqtt_state=$PROTCONNECTING;
    # pause for a moment, as this is unlikely to get better immediately
    # (avoids thrashing client machine)
    sleep 5;
  }
}

####################################################################

sub app_publish
{
  # we received a PUBLISH

  # parameters are: subject, MID, content    
  my ($subject,$MID,$content) = @return;


  if ($QoS == 2)
  {
    print "Received a QoS2 publish - MID $MID\n" if $debug;
    $QoS2{$MID}->{subject}=$subject;
    $QoS2{$MID}->{content}=$content;
    $QoS2{$MID}->{retain}=$RETAIN;

    # note that this does duplicate removal in case the same msg
    # comes again

    &pubrec($MID);

  }
  else # not QoS 2
  {
    if (!$silent)
    {
      # for QoS 0 and 1, display the message now
      print "\n>>>>QoS $QoS message MID:$MID on topic: '$subject'\n";
      #print ">>>>Content: <$content>\n";
      #&dump($content);

    }
    &parse($subject,$content);

  }

  if ($QoS == 1)
  {
    # QoS 1 requires a PUBACK
    &puback($MID);
    print "message $MID acknowledged\n" if $debug;
  }


}

####################################################################

sub app_pubrel
{
  # we received a PUBREL

  # parameter is MID
  my ($MID) = @return;
  my ($topic,$content);

  if (exists $QoS2{$MID})
  {
    if (!$silent)
    {
      print "\n>>>>QoS 2 message MID:$MID on topic: '",
             $QoS2{$MID}->{subject},"'\n";
      #print ">>>>Content: <$QoS2{$MID}->{content}>\n";
      #&dump($QoS2{$MID}->{content});
    }
 
    $RETAIN = $QoS2{$MID}->{retain};

    $topic = $QoS2{$MID}->{subject};
    $content = $QoS2{$MID}->{content};


    delete $QoS2{$MID};
  }
  else
  {
    print "duplicate pubrel received for MID $MID\n";
    # this isn't bad.. it just needs to be pubcomp'd
  }


  &pubcomp($MID);


  print "pubcomp sent\n" if $debug;

  # process the message after we send the pubcomp, in case it takes
  # more than 10 sec to process it 

  &parse($topic,$content);

}

####################################################################

sub app_suback
{
  # we received a SUBACK
  my ($MID,@gQoS);

  if ($mqtt_state != $WAITSUBACK)
  {
    print "received a SUBACK in state: ",&decode_appstate,"\n";
    exit;
  }


  ($MID,@gQoS)=@return;
  print "SUBACK MID:$MID granted QoS: [",join(',',@gQoS),"]\n" if $debug;
  if (exists $SUB_MID{$MID})
  {
    delete $SUB_MID{$MID};
    print "MID validated\n" if $debug;
  }
  else
  {
    print "Unexpected MID on SUBACK: $MID\n";
    exit;
  }


  $mqtt_state = $RUNNING;

}

####################################################################

sub app_unsuback
{
  # we received an UNSUBACK
  
  #
  # ***** if you do an unsubscribe, remember to set $UNSUB_MID{$MID}=1
  #

  my ($MID)=@return;
  print "got UNSUBACK... MID:$MID\n" if $debug;
  if (exists $UNSUB_MID{$MID})
  {
    delete $UNSUB_MID{$MID};
    print "MID validated\n" if $debug;
  }
  else
  {
    print "Unexpected MID on UNSUBACK: $MID\n";
    exit;
  }
}

####################################################################

sub app_pingreq
{
  # we received a PINGREQ
  # client shouldn't receive this!!

  print "PING!!!\n";
  &pingresp;
  print "... PONG sent\n";
}

####################################################################

sub app_pingresp
{
  # we received a PINGRESP

  print "PONG received!\n" if $debug;
}


####################################################################


sub decode_appstate
{
  # print the word that describes the current application state

  return $app_states{$mqtt_state};
  
}

####################################################################

sub pub_puback
{
  # handle puback, which would be returned from the broker if we
  # published a qos1 message

  my (@return)=@_;

          $rxMID=$return[0];
          if (exists $outputQ{$rxMID})
          {
            if ($outputQ{$rxMID}->{QoS}==1)
            {
              print "received PUBACK for MID $rxMID\n" if $debug;
              delete $outputQ{$rxMID};
            }
            elsif ($outputQ{$rxMID}->{QoS}==2)
            {
              print "received unexpected PUBACK MID $rxMID to QoS2 message\n";
              exit;
            }
          }
          # if it doesn't exist, we just ignore it... that's the lightweight
          # simplicity of the QoS1 protocol flow
}

####################################################################

sub pub_pubrec
{
  # handle pubrec messages, which would be returned from the broker
  # if we published a qos2 message



  my (@return)=@_;                  

  print "received PUBREC\n" if $debug;

          $rxMID=$return[0];

#          if (exists $outputQ{$rxMID} && 
#              $outputQ{$rxMID}->{QoS}==2 && 
#              $outputQ{$rxMID}->{state}==$PUBLISHED)

          if (exists $outputQ{$rxMID} &&
              $outputQ{$rxMID}->{QoS}==2)
          {
            if ($outputQ{$rxMID}->{state}==$PUBLISHED)
            {
              # it's OK, so pubel it

              if (!defined &pubrel($rxMID))
              {
                $mqtt_state = $TCPonnecting;
              }

              # it is correct to move on to the next state regardless of whether or not
              # we successfully sent the puback2
              $outputQ{$rxMID}->{state}=$RELEASED;
            }
            elsif ($outputQ{$rxMID}->{state}==$RELEASED)
            {
              # just a duplicate pubrec... ignore it

              print "ignoring pubrec... we've moved on\n";

              #sleep 5;
            }
            else
            {
              print "unexpected pubrec\n";

              print "$rxMID, QoS: $outputQ{$rxMID}->{QoS}, state: ",
                        &decode_state($outputQ{$rxMID}->{state}),"\n";
              exit;
            }
          }
          else
          {

            print "received unexpected PUBREC MID $rxMID\n" if $debug;

            if (!exists $outputQ{$rxMID})
            {
              print "record not in outputQ\n";
            }
            else
            {
              print "$rxMID, QoS: $outputQ{$rxMID}->{QoS}, state: ",
                        &decode_state($outputQ{$rxMID}->{state}),"\n";
            }
            #sleep 5;
            &dump($rawbuffer);
            exit;
          }
}

####################################################################

sub pub_pubcomp
{
  # handle pubcomp, which would be sent from the broker as the final
  # step of handling a qos2 publish from this application

 
  my (@return)=@_;

  print "received PUBCOMP\n" if $debug;


  $rxMID=$return[0];
  if (exists $outputQ{$rxMID})
  {
    if ($outputQ{$rxMID}->{QoS}==2 && $outputQ{$rxMID}->{state}==$RELEASED)
    {
      print "QoS 2 message MID $rxMID completed successfully!\n" if $debug;
      delete $outputQ{$rxMID};
    }
    elsif ($outputQ{$rxMID}->{QoS}==1 || $outputQ{$rxMID}->{state}==$PUBLISHED)
    {
      print "received unexpected PUBCOMP MID $rxMID to QoS2 message\n";

      exit;
    }
  }

	# if it doesn't exist we just ignore it... it must be a duplicate
}

####################################################################


sub do_publish # (topic, qos, ret, content)
{
  # this is what you should call to send a message from this app
  my ($topic,$sentQoS,$retain,$message) = @_;

#    &publish("status/$application_name",0,0,0,1,"1");
 
  # do what we have to do when we're in sending state
  my ($MID)=0;

  # get ourselves a message ID, for QoS1 and 2
  if ($sentQoS != 0)
  {
    $MID = &newMID;
  }

  # V2: publish (Subject, DUP, QoS, MID, RETAIN, content)
  @parms = ($topic,0,$sentQoS,$MID,$retain,$message);




  if (!defined &publish(@parms))
  {
    print "error during publish\n" if $debug;
    $mqtt_state = $TCPconnecting;
  }
  else
  {
    print "sent Qos $sentQoS message\n" if $debug;
    #&log($message,$MID,$sentQoS);
  }

  # regardless of whether or not there was an error, add it to the outputQ, so if it
  # did fail, then the timeout skulker will catch it and resend in a short while
  if ($sentQoS==1 || $sentQoS==2)
  { 
    # message ID is correct value at this point
    $outputQ{$MID}->{message}=$message;
    $outputQ{$MID}->{topic}=$topic;
    $outputQ{$MID}->{time}=time;
    $outputQ{$MID}->{state}=$PUBLISHED;
    $outputQ{$MID}->{QoS}=$sentQoS;
    $outputQ{$MID}->{retain}=$retain;

    print "MID $MID\n" if $debug;
  }

}


####################################################################





####################################################################


1
