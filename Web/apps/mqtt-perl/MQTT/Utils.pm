package MQTT::Utils;

=pod

=head1 NAME

MQTT::Utils - a collection of utility methods for use by MQTT::Client.

=head1 COPYRIGHT

(C) Copyright IBM Corp. 2006, 2008

=head1 VERSION

1.0.1

=head1 AUTHOR

Nicholas O'Leary - nick_oleary@uk.ibm.com

=cut

use Exporter;

@ISA = ('Exporter');
@EXPORT = qw(&fixed &toUTF &fromUTF &tobyte &frombyte &toword 
             &fromword &build &dump &ss &decode_state 
             &toSingleFloat &fromSingleFloat &toLong &fromLong 
             &toInt16 &fromInt16 &newMessageID
             $version %PKT_TYPES @protocol_name
             $messageID %connackRC %PKT_NAMES $BUFFSIZE
             $BROKER_BUILT $BROKER_SENT $BROKER_RELEASED $BROKER_PUBACK1
             $BROKER_PUBREC $PUBLISHED $RELEASED %statecodes
             );

our $VERSION   = 0.1;

# helper functions

# fixed($type,$DUP,$QoS,$RETAIN)
# toUTF($string)
# fromUTF($buffer)
# tobyte($value)
# frombyte($buffer)
# toword($value)
# fromword($buffer)
# build
# dump($string)
# ss($value)
# decode_state($state)
# toSingleFloat($buffer)
# fromSingleFloat($buffer)
# toLong(value)
# fromLong($buffer)
# toInt16($value)
# fromInt16($buffer)



####################################################################

sub fixed
{
  # create the fixed header portion in $fixed
  # for version 1, RETAIN was called SEC
  local ($type,$DUP,$QoS,$RETAIN) =@_;

  return &tobyte($type*16 + $DUP*8 + $QoS*2 + $RETAIN);
}

####################################################################

sub toUTF
{
  # return the UTF form of a string parameter
  # UTF encoding using big-endian, regardless of whatever else we do
  my ($string) = @_;
  return pack "n/a*", $string;
}

####################################################################

sub fromUTF
{
  # return the ASCII string version of a UTF string (plus more buffer)
  my ($utf) = @_;
  return unpack "n/a*", $utf;
}

####################################################################

sub tobyte
{
  # return the one byte representation of a 0..255 parameter
  local ($value) = @_;

  return chr($value);
}

####################################################################

sub frombyte
{
  # return the 0..255 value from a one byte char parameter (plus more buffer)
  local ($buf)=@_;

  return ord(substr($buf,0,1));
}

####################################################################

sub toword
{
  # return the two byte representation of a 0..65535 parameter
  local ($value) = @_;
  local ($MSB, $LSB);

  $MSB = $value/256;
  $LSB = $value & 0x00FF;

  # BIG endian
  return chr($MSB).chr($LSB);
}

####################################################################

sub fromword
{
  # return the 0..65535 number from a two byte string parameter
  # (plus more buffer)
  local($word) = @_;
  local ($MSB,$LSB);
  $MSB = ord(substr($word,0,1));
  $LSB = ord(substr($word,1,1));

  # big endian
  return $MSB*256+$LSB;
}

####################################################################

sub build
{
  # DEPRECATED
  # build a buffer ready to send, and send it unless specifically asked
  # not to by $compile_only being true. Returns 0 for success, or undef
  # if there was a TCP problem

  my ($fixed, $variable, $payload) = @_;
  return unless defined TCPwrite($TCPHANDLE, $fixed . $variable . $payload);
  return 0;
}


####################################################################

sub dump
{
  # hex dump the string
  local ($string) = @_;
  local ($i);

  print "\n---------------------------------------------\n";
  print "Message: (",length($string)," bytes)\n";
  for ($i=0; $i<length($string);$i++)
  {
    #printf ("%02X (%c) ",ord(substr($string,$i,1)),
    #                     ord(substr($string,$i,1)));
    printf ("%02X ",ord(substr($string,$i,1)));
  }
  print "\n";
  print "---------------------------------------------\n\n";

}


####################################################################

sub ss
{
  # return an s if the parameter warrants it (i.e. is != 1)

  return ($_[0]==1?"":"s");
}

####################################################################

sub decode_state
{
  # print the word that describes the message state
  my ($state) = @_;

  return $statecodes{$state};
}


####################################################################

sub prepare_log
{
  # declares and clears out an instance of the protocol log file
  # also turns on logging

  ($protLogFile) = @_;

  $logging = 1;

  unlink($protLogFile);

  print "** protocol logging enabled to '$protLogFile'\n";
}



####################################################################


sub fromSingleFloat
{
  # return the single precision float from a four byte string parameter
  local($float) = substr($_[0],0,4);

  if (1)
  {
    # swap round the byte order into Intel format

    my (@a) = split(//,$float);
    my (@b);

    $b[0]=$a[3];$b[1]=$a[2];$b[2]=$a[1];$b[3]=$a[0];

    $float = join('',@b);
  }
  return unpack("f",$float);
}

####################################################################


sub toSingleFloat
{
  # this one does NOT reverse the byte order, so fromSingleFloat is
  # not the inverse function of it

  # return the representation of a single precision float
  local($float) = @_;

  return pack("f",$float);
}

####################################################################


sub fromLong
{
  # return the 32-bit integer from a four byte string parameter
  # these are bigendian - network order

  local($long) = @_;

  return unpack("N",$long);
}


####################################################################


sub fromLongLE
{
  # return the 32-bit integer from a four byte string parameter
  # these are littleendian - "VAX order" (!)

  local($long) = @_;

  return unpack("V",$long);
}

####################################################################


sub toLong
{
  # return the representation of a 32-bit integer
  # these are bigendian - network order
  local($long) = @_;

  return pack("N",$long);
}


####################################################################


sub fromInt16
{
  # return the 16-bit integer from a two byte string parameter
  # these are bigendian - network order
  local($short) = @_;

  return unpack("n",$short);
}

####################################################################


sub fromInt16LE
{
  # return the 16-bit integer from a two byte string parameter
  # ******* these are little-endian ********
  
  my ($msb,$lsb)=split(//,$_[0],2);

  return ord($msb)+ord($lsb)*256;
}


####################################################################


sub toInt16
{
  # return the representation of a 16-bit integer
  # these are bigendian - network order
  local($long) = @_;

  return pack("n",$long);
}


####################################################################

sub newMessageID
{
  $messageID++;
  return $messageID;
}

####################################################################




# variables.pl

# constants and globals

# enumerated types for message types

$version = 2;

#$PING=0;

%PKT_TYPES = (
   INVALID=>0,
   CONNECT=>1,
   CONNACK=>2,
   PUBLISH=>3,
   PUBACK=>4,
   PUBACK1=>5,
   PUBACK2=>6,
   PUBACK3=>7,
   SUBSCRIBE=>8,
   SUBACK=>9,
   PUBREC=>5,
   PUBREL=>6,
   PUBCOMP=>7,
   UNSUBSCRIBE=>10,
   UNSUBACK=>11,
   PINGREQ=>12,
   PINGRESP=>13,
   DISCONNECT=>14
   );


# if you change these, don't forget to change the packet_name vector below

$protocol_name[1] = "ArgoOTWP";
$protocol_name[2] = "MQIpdp";
$protocol_name[3] = "MQIsdp";




# rolling message identifier
# supplied to the user before a publish by calling &newMID, which is in protocol.pl

$messageID = 0;


# Jump table for message handler functions




# error messages from CONNACK return code

%connackRC = (0 =>"Connection Accepted",
              1 =>"Connection Refused: unacceptable protocol version",
              2 =>"Connection Refused: identifier rejected",
              3 =>"Connection Refused: broker unavailable");


# names of packet types - for informational messages
# don't forget to change these if the packet names change!

                                                       
%PKT_NAMES = (
   $PKT_TYPES{INVALID}     => "** INVALID **",
   $PKT_TYPES{CONNECT}     => "CONNECT",
   $PKT_TYPES{CONNACK}     => "CONNACK",
   $PKT_TYPES{PUBLISH}     => "PUBLISH",
   $PKT_TYPES{PUBACK}      => "PUBACK",
   $PKT_TYPES{PUBREC}      => "PUBREC",
   $PKT_TYPES{PUBREL}      => "PUBREL",
   $PKT_TYPES{PUBCOMP}     => "PUBCOMP",
   $PKT_TYPES{SUBSCRIBE}   => "SUBSCRIBE",
   $PKT_TYPES{SUBACK}      => "SUBACK",
   $PKT_TYPES{UNSUBSCRIBE} => "UNSUBSCRIBE",
   $PKT_TYPES{UNSUBACK}    => "UNSUBACK",
   $PKT_TYPES{PINGREQ}     => "PINGREQ",
   $PKT_TYPES{PINGRESP}    => "PINGRESP",
   $PKT_TYPES{DISCONNECT}  => "DISCONNECT"
   );

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

%STATES = (
   DISCONNECTED            => 0,
   CONNECTING              => 1,
   CONNECTED               => 2
   );

####################################################################




1;

