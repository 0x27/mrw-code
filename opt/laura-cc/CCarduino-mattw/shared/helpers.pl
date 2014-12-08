# helpers.pl

# Andy Stanford-Clark 26-Mar-99


####################################################################

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
# protlog($message)
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
  my ($type,$DUP,$QoS,$RETAIN) =@_;

  $fixed = &tobyte($type*16 + $DUP*8 + $QoS*2 + $RETAIN);
}

####################################################################

sub toUTF
{
  # return the UTF form of a string parameter
  # UTF encoding using big-endian, regardless of whatever else we do
  my ($string) = @_;
  my ($MSB,$LSB);

  $MSB = length($string)/256;
  $LSB = length($string) & 0x00FF;

  return chr($MSB).chr($LSB).$string;
}

####################################################################

sub fromUTF
{
  # return the ASCII string version of a UTF string (plus more buffer)
  my($UTFstring)=@_;
  my($MSB,$LSB);

  $MSB=ord(substr($UTFstring,0,1));
  $LSB=ord(substr($UTFstring,1,1));

  return substr($UTFstring,2,$MSB*256+$LSB);
}

####################################################################

sub tobyte
{
  # return the one byte representation of a 0..255 parameter
  my ($value) = @_;

  return chr($value);
}

####################################################################

sub frombyte
{
  # return the 0..255 value from a one byte char parameter (plus more buffer)
  my ($buf)=@_;

  return ord(substr($buf,0,1));
}

####################################################################

sub toword
{
  # return the two byte representation of a 0..65535 parameter
  my ($value) = @_;
  my ($MSB, $LSB);

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
  my($word) = @_;
  my ($MSB,$LSB);

  $MSB = ord(substr($word,0,1));
  $LSB = ord(substr($word,1,1));

  # big endian
  return $MSB*256+$LSB;
}

####################################################################

sub build
{
  # build a buffer ready to send, and send it unless specifically asked
  # not to by $compile_only being true. Returns 0 for success, or undef
  # if there was a TCP problem

  my ($rc);
  print "build version=$version\n" if $debug;


  $x = length($variable) + length($payload);
  print "$x -> " if $debug;
  do
  {
    $digit = $x % 128;
    $x = int($x/128);

    # if there are more digits to encode, set the top bit of this digit
    if ( $x > 0 )
    {
      $digit |= 0x80;
    }

    $fixed .= chr($digit);
    printf("%02X ",$digit) if $debug;
  } while ($x > 0);

  print "\n" if $debug;


  if ($trace)
  {
    print "fixed: ";
    &dump($fixed);
    print "variable: ";
    &dump($variable);
    print "payload: ";
    &dump($payload);

    if ($alert)
    {
      print "**********************************\n";
      sleep 30;
      $alert=0;
    }
  }

  $buffer = $fixed . $variable . $payload;

  #print "<$TCPHANDLE>\n";

  # the broker can set a global compile_only flag, which means that the buffer is
  # constructed, but not sent out... this is so it can be put in a queue for later
  # transmission

  if (!$compile_only)
  {
    $rc=&TCPwrite($TCPHANDLE,$buffer);
    if (!defined $rc)
    {
      # socket is already TCPclosed if there was an error
      return undef;
    }
  }

  return 0;
}


####################################################################

sub dump
{
  # hex dump the string
  my ($string) = @_;
  my ($i);

  for ($i=0; $i<length($string);$i++)
  {
    #printf ("%02X (%c) ",ord(substr($string,$i,1)),
    #                     ord(substr($string,$i,1)));
    printf ("%02X ",ord(substr($string,$i,1)));
  }
  print "\n";

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


sub protlog
{
  # log message details to a file
  my ($message) = @_;

  if ($logging)
  {
    open (LOG,">>$protLogFile") || die "can't open log file: $!";
    print LOG "$message\n";
    close LOG;
  }
  if ($msg_trace)
  {
    print $message,"\n";
  }
}


####################################################################


sub fromSingleFloat
{
  # return the single precision float from a four byte string parameter
  my($float) = substr($_[0],0,4);

  $float = reverse(split(//,$float));

  return unpack("f",$float);
}

####################################################################


sub toSingleFloat
{
  # this one does NOT reverse the byte order, so fromSingleFloat is
  # not the inverse function of it

  # return the representation of a single precision float
  my($float) = @_;

  return pack("f",$float);
}

####################################################################


sub fromLong
{
  # return the 32-bit integer from a four byte string parameter
  # these are bigendian - network order

  my($long) = @_;

  return unpack("N",$long);
}

####################################################################


sub toLong
{
  # return the representation of a 32-bit integer
  # these are bigendian - network order
  my($long) = @_;

  return pack("N",$long);
}


####################################################################


sub fromInt16
{
  # return the 16-bit integer from a two byte string parameter
  # these are bigendian - network order
  my($short) = @_;

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
  my($long) = @_;

  return pack("n",$long);
}

####################################################################

1
