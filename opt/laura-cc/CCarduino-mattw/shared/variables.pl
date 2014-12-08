# variables.pl

# constants and globals


####################################################################

# num_messages is the number of messages to send/receive in the benchmark

$num_messages=0;

####################################################################

$debug=0;

# trace is even more nitty gritty detail
$trace=0;

# silent muffles message received by the subscriber... for benchmarking
$silent=1;

# msg_trace is to trace the message arrival through the broker
# (brlogic.pl and brqos.pl
$msg_trace=0;


####################################################################

# randomly close 5% of connections... that'll annoy them :-)
$random_tolerance = 0.05;

####################################################################

$version = 3;

####################################################################


# enumerated types for message types

#$PING=0;
$INVALID=0;
$CONNECT=1;
$CONNACK=2;
$PUBLISH=3;
$PUBACK=4;

#these are maintained for backwards compatability
$PUBACK1=5;
$PUBACK2=6;
$PUBACK3=7;

$SUBSCRIBE=8;
$SUBACK=9;



# V2 changes and additions
$PUBREC=5;
$PUBREL=6;
$PUBCOMP=7;

$UNSUBSCRIBE=10;
$UNSUBACK=11;
$PINGREQ=12;
$PINGRESP=13;
$DISCONNECT=14;


# if you change these, don't forget to change the packet_name vector below

$protocol_name[1] = "ArgoOTWP";
$protocol_name[2] = "MQIpdp";
$protocol_name[3] = "MQIsdp";


# rolling message identifier
# supplied to the user before a publish by calling &newMID, which is in protocol.pl

$OTWP_messageID = 0;


# Jump table for message handler functions


%table = ($CONNECT=>\&handle_connect,
          $CONNACK=>\&handle_connack,
          $PUBLISH=>\&handle_publish,
          $PUBACK=>\&handle_puback,
          $PUBREC=>\&handle_pubrec,
          $PUBREL=>\&handle_pubrel,
          $PUBCOMP=>\&handle_pubcomp,
          $SUBSCRIBE=>\&handle_subscribe,
          $SUBACK=>\&handle_suback,
          # V2 additions...
          0 => \&handle_illegal,
          $UNSUBSCRIBE=>\&handle_unsubscribe,
          $UNSUBACK=>\&handle_unsuback,
          $PINGREQ=>\&handle_pingreq,
          $PINGRESP=>\&handle_pingresp,
          $DISCONNECT=>\&handle_disconnect);


# error messages from CONNACK return code

%connackRC = (0 =>"Connection Accepted",
              1 =>"Connection Refused: unacceptable protocol version",
              2 =>"Connection Refused: identifier rejected",
              3 =>"Connection Refused: broker unavailable");


# names of packet types - for informational messages
# don't forget to change these if the packet names change!


$packet_name[$INVALID]     ="** INVALID **";
$packet_name[$CONNECT]     ="CONNECT";
$packet_name[$CONNACK]     ="CONNACK";
$packet_name[$PUBLISH]     ="PUBLISH";
$packet_name[$PUBACK]      ="PUBACK";
$packet_name[$PUBREC]      ="PUBREC";        # fka PUBACK1
$packet_name[$PUBREL]      ="PUBREL";        # fka PUBACK2
$packet_name[$PUBCOMP]     ="PUBCOMP";       # fka PUBACK3
$packet_name[$SUBSCRIBE]   ="SUBSCRIBE";
$packet_name[$SUBACK]      ="SUBACK";
$packet_name[$UNSUBSCRIBE] ="UNSUBSCRIBE";   # V2
$packet_name[$UNSUBACK]    ="UNSUBACK";      # V2
$packet_name[$PINGREQ]     ="PINGREQ";       # V2
$packet_name[$PINGRESP]    ="PINGRESP";      # V2
$packet_name[$DISCONNECT]  ="DISCONNECT";    # V2

# buffsize is the maximum size a single message can be
# do we need this?
#$BUFFSIZE=65535+3;
$BUFFSIZE = 128**4;

# delimiter is the thing we put in between things to be delimited, like subscription topics
# in the broker

#$delimiter = "#!#";



# underline is just a row of dashes used to delimit messages

$underline = "------------------------------------------------------\n";


# broker states - these are used in brlogic to drive the QoS message flows

$BROKER_BUILT=1;

$BROKER_SENT=2;        # published

$BROKER_RELEASED=3;    # puback2/pubrel sent

$BROKER_PUBACK1=4;     # puback1/pubrec sent
# V2
$BROKER_PUBREC=4;

$PUBLISHED=5;          # same as broker_sent
$RELEASED=6;           # same as broker_released


# this table is used by decode_state in publish.pl
%statecodes = ($BROKER_BUILT =>"BROKER_BUILT",
               $BROKER_SENT =>"BROKER_SENT",
               $BROKER_RELEASED =>"BROKER_RELEASED",
               $BROKER_PUBREC => "BROKER_PUBREC",
               $PUBLISHED => "PUBLISHED",
               $RELEASED => "RELEASED"
              );


####################################################################

1
