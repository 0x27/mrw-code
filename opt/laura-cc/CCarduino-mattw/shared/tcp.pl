# tcp.pl

# TCP layer functions for lightweight protocol

# Andy Stanford-Clark 12-Apr-99



####################################################################

# see
# http://www.mdstud.chalmers.se/|md4marcu/perl/server2.txt
# for some helpful tcp hints
#



sub TCPconnect
{
  # make a socket connection to the broker
  # This is just the socket... the protocol CONNECT packet is done next

  # takes the name and port of the broker as a parameter
  # e.g. "argo.hursley.ibm.com:8000"

  # currently this loops until it connects... it may stay here forever... 
  # but then what else would you want to do? 
  # (Ans: go to a different broker after a few retries)

  # returns the socket handle

  local ($broker)=@_;
  local ($desthost, $port, $proto, $sin);
  local ($name,$aliases);
  # create a local socket instance
  local (*S);
  my ($address);

  ($desthost, $port) = split(/:/,$broker);

  print "TCPconnect to <$desthost> port <$port>\n" if $debug;
      
  ($name,$aliases,$proto) = getprotobyname('tcp');

  # andysc - 29-oct-09
  # put in error checking on the address lookup, so we don't exit with 
  # Bad arg length for Socket::pack_sockaddr_in, length is 0, should be 4 
  # when you're not connected to a network. 
  # Now says "DNS lookup error: Connection timed out"

  $address = inet_aton($desthost);
  if (!$address)
  {
    print "DNS lookup error: $!\n";
    return undef;
  }
  $sin = sockaddr_in($port,$address);

  # what it used to be...
  #$sin = sockaddr_in($port,inet_aton($desthost));


  # Create the connection to a remote server.
  socket(S, AF_INET, SOCK_STREAM, $proto) || die "socket: $!";

  if (!connect(S, $sin))
  {
    print "TCPconnect error: $!\n";
    return undef;
  }
  else
  {
    select(S); $| = 1; select(STDOUT);
    # this should return the socket instance
    return *S;
  }

}


####################################################################


sub TCPlisten
{
  # create a listening socket (for the broker)
  # port number is is passed in as parameter
  local ($port)=@_;

  #local($proto) = getprotobyname('tcp');
  # ***
  local($name,$aliases,$proto) = getprotobyname('tcp');

  # make lsnsock local
  local (*LSNSOCK);

  socket(LSNSOCK, AF_INET, SOCK_STREAM, $proto)    or die "socket: $!";
  setsockopt(LSNSOCK,SOL_SOCKET,SO_REUSEADDR, pack("l",1))
                                                or die "setsockopt: $!";
  bind(LSNSOCK, sockaddr_in($port, INADDR_ANY))    or die "bind: $!";
  listen(LSNSOCK, SOMAXCONN)                       or die "listen: $!";

  print "Broker listening on port $port\n";
  return *LSNSOCK;
}


####################################################################


sub TCPaccept
{
  # accept an incoming socket connection (for the broker)
  # parameter is the socket handle of the listener
  local($listen)=@_;
  
  local($paddr, $cport, $caddr, $name);

  # make the socket handle a local instance\
  local (*S);

  $paddr=accept(S,$listen);
  ($cport,$caddr) = sockaddr_in($paddr);
  $name = gethostbyaddr($caddr,AF_INET);

  print "Connection from $name [",inet_ntoa($caddr), "]\n" if $debug;

  select S; $|=1; select STDOUT;

  # this should pass back the local handle
  return *S;
}


####################################################################
               

sub TCPread
{
  # wait for a packet to come in from the socket
  # takes the socket handle as a parameter
  # returns the received data as a buffer, or undef if there is an error
  
  local ($HANDLE)=@_;
  local ($header,$buf,$read,$errno,$length,$byte);

  print $underline if $debug;

  print "TCPread\n" if $debug;
  #print "** TCPread from fileno ",fileno($HANDLE),"\n";


  $read = &socketread($HANDLE,\$header,1);
  #printf "<%02X>\n",ord($header);


  if (!defined $read)
  {
    # error!
    return undef;
  }
  print "** got: $read\n" if $debug;


  # we have some more to read to find out the remaining length
  $length=0;
  $multiplier=1;
  do
  {
    #print "about to read a byte\n"; 
    if (!defined &socketread($HANDLE,\$byte,1))
    {
      # error!
      return undef;
    }
    #print "got one\n";
    $length += (ord($byte) & 0x7F) * $multiplier;
    $multiplier *= 128;
    # append this byte to the "fixed" header, so we can use it later
    $header .= $byte;
  } while (ord($byte) & 0x80);


  print "expecting $length bytes from socket\n" if $debug;

  # there might be no more message... e.g. pingreq
  if ($length>0)
  {
    $read = &socketread($HANDLE,\$buf,$length);

    if (!defined $read)
    {
      # error!
      return undef
    }
  }

  return $header.$buf;  
}


####################################################################

sub socketread
{
  # reads a specified number of bytes from a socket into a buffer
  # gets round the packet fragmentation problem of sockets
  # returns number of bytes read or undef if there is an error

  # parameters are socket_handle, pointer to buffer, bytes to read
  local ($HANDLE,$buf_ptr,$bytes)=@_;
  local ($read,$buffer,$total);

  $$buf_ptr="";

  while ($bytes>0)
  {
    $read = sysread $HANDLE,$buffer,$bytes;

    #print "got $read\n";

    if (!defined $read || $read==0)
    {
      $errno = int($!);
      if (1) # ($errno==0 || $errno==22)
      {
        # these appear to be indications of socket closed
        print "socket closed: $errno '$!'\n";
        &TCPclose($TCPHANDLE);
        return undef;
      }
    }

    $$buf_ptr .= $buffer;
    $bytes -= $read;
    $total += $read;

  } # wend

  return $total;
}


####################################################################


sub TCPwrite
{
  # send a constructed protocol packet to the socket
  # takes the socket handle and the buffer to be sent as parameters
  # returns 0 for success, or undef if there was a TCP problem
  
  local ($HANDLE,$buf)=@_;
  local ($written);

  print "TCPwrite\n" if $debug;

  print "sending ",length($buf)," bytes\n" if $debug;

  while (length($buf)>0)
  {
    $written = syswrite $HANDLE,$buf,length($buf);
    if (!defined $written)
    {
      $errno = int($!);
      print "socket closed: $errno '$!'\n";
      &TCPclose($HANDLE);
      return undef;
    }

    # chop off the bits of buf that we wrote and go round to send the rest
    $buf = substr($buf,$written);

  }

  print $underline if $debug;

  return 0;
}


####################################################################


sub TCPclose
{
  # disconnect a TCP connection
  # takes socket handle as a parameter
  
  local ($HANDLE)=@_;

  print "TCPclose\n" if $debug;

  close ($HANDLE);
}


####################################################################

1
