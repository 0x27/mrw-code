package MQTT::Client;

=pod

=head1 NAME

MQTT::Client - Perl module for using the MQTT protocol.

=head1 SYNOPSIS

   ### To perform a 'one-shot' publish to a topic:

   use MQTT::Client;
   
   my $mqtt = MQTT::Client->new({
      brokerIP => "192.168.0.1",
      clientID => "MyPerlClient",
   });
   $mqtt->connect();
   $mqtt->publish("a/topic",0,0,"my example message");
   $mqtt->disconnect();
   
   
   ### To subscribe to a topic and wait for publishes:
   
   use MQTT::Client;
   
   my $mqtt = MQTT::Client->new({
      brokerIP => "192.168.0.1",
      clientID => "MyPerlClient",
      callback_publish => \&publish_callback
   });
   
   $mqtt->connect();
   $mqtt->subscribe("a/topic",0);
   $mqtt->listen_for_publishes();
   
   
   sub publish_callback {
      my $topic = shift;
      my $msg = shift;
      print $topic.":".$msg."\n";
      if ($msg eq 'quit') {
         $mqtt->disconnect();
         exit 0;
      }
   }

=head1 DESCRIPTION

This module provides an interface to the MQ Telemetry Transport protocol, 
allowing both publishing and subscribing to topics.

It current has full support for quality of service (QoS) level 0. There is
some support for the higher QoS levels, but further work is needed to
complete this support. See the project readme for more information.

=head1 COPYRIGHT

(C) Copyright IBM Corp. 2006, 2008

=head1 VERSION

1.0.1

=head1 AUTHOR

Nicholas O'Leary - nick_oleary@uk.ibm.com

=head1 METHODS

=cut

use MQTT::Socket;
use MQTT::Utils;
use Devel::Leak;

our @ISA = ();
our $VERSION   = 0.1;

=head2 C<new( %arguments )>

The constructor creates a new MQTT::Client object. A hash is passed as the sole 
argument. Valid keys are:

=over 4

=item * B<brokerIP> - the IP address of the broker to connect to. (default: 127.0.0.1)

=item * B<brokerPort> - the port to connect to. (default: 1883)

=item * B<clientID> - the client ID to connect as. (default: "MQTTPerlClientXX" where XX is the process number)

=item * B<keepAlive> - the keep alive setting. (default: 0)

=item * B<cleanStart> - the clean start setting. (default: 1)

=item * B<QoS> - the Quality of Service to connect with. (default: 0)

=item * B<callback_publish> - a pointer to a function to call when a publish is received.

=back

=cut

sub new
{
   my $module = shift @_;
   my $self = $_[0];
   $self->{brokerIP} = "127.0.0.1" if (!exists $self->{brokerIP});

   $self->{brokerPort} = 1883 if (!exists $self->{brokerPort});
   $self->{clientID} = sprintf("MQTTPerlClient%0X",$$)  if (!exists $self->{clientID});
   $self->{keepAlive} = 0 if (!exists $self->{keepAlive});
   $self->{cleanStart} = 1 if (!exists $self->{cleanStart});
   $self->{QoS} = 0 if (!exists $self->{QoS});
   $self->{state} = $STATES{DISCONNECTED};
   $self->{pkt_parse_table} = {
      0                       => sub { $self->parse_illegal(@_)},
      $PKT_TYPES{CONNECT}     => sub { $self->parse_connect(@_)},
      $PKT_TYPES{CONNACK}     => sub { $self->parse_connack(@_)},
      $PKT_TYPES{PUBLISH}     => sub { $self->parse_publish(@_)},
      $PKT_TYPES{PUBACK}      => sub { $self->parse_puback(@_)},
      $PKT_TYPES{PUBREC}      => sub { $self->parse_pubrec(@_)},
      $PKT_TYPES{PUBREL}      => sub { $self->parse_pubrel(@_)},
      $PKT_TYPES{PUBCOMP}     => sub { $self->parse_pubcomp(@_)},
      $PKT_TYPES{SUBSCRIBE}   => sub { $self->parse_subscribe(@_)},
      $PKT_TYPES{SUBACK}      => sub { $self->parse_suback(@_)},
      $PKT_TYPES{UNSUBSCRIBE} => sub { $self->parse_unsubscribe(@_)},
      $PKT_TYPES{UNSUBACK}    => sub { $self->parse_unsuback(@_)},
      $PKT_TYPES{PINGREQ}     => sub { $self->parse_pingreq(@_)},
      $PKT_TYPES{PINGRESP}    => sub { $self->parse_pingresp(@_)},
      $PKT_TYPES{DISCONNECT}  => sub { $self->parse_disconnect(@_)}
   };
   $self->{QoS2} = {};
   $self->{pkt_handler_table} = {
      0                       => sub { $self->handle_illegal(@_)},
      $PKT_TYPES{CONNECT}     => sub { $self->handle_illegal(@_)},
      $PKT_TYPES{CONNACK}     => sub { $self->handle_connack(@_)},
      $PKT_TYPES{PUBLISH}     => sub { $self->handle_publish(@_)},
      $PKT_TYPES{PUBACK}      => sub { $self->handle_puback(@_)},
      $PKT_TYPES{PUBREC}      => sub { $self->handle_pubrec(@_)},
      $PKT_TYPES{PUBREL}      => sub { $self->handle_pubrel(@_)},
      $PKT_TYPES{PUBCOMP}     => sub { $self->handle_pubcomp(@_)},
      $PKT_TYPES{SUBSCRIBE}   => sub { $self->handle_illegal(@_)},
      $PKT_TYPES{SUBACK}      => sub { $self->handle_suback(@_)},
      $PKT_TYPES{UNSUBSCRIBE} => sub { $self->handle_illegal(@_)},
      $PKT_TYPES{UNSUBACK}    => sub { $self->handle_unsuback(@_)},
      $PKT_TYPES{PINGREQ}     => sub { $self->handle_pingreq(@_)},
      $PKT_TYPES{PINGRESP}    => sub { $self->handle_pingresp(@_)},
      $PKT_TYPES{DISCONNECT}  => sub { $self->handle_illegal(@_)},
      15                      => sub { $self->handle_illegal(@_)}
   };
   bless ($self,$module);
}


=head2 C<connect>

A call to this connects the client to the broker.

=cut

sub connect
{
   my $self = shift;
   
   $self->{state} = $STATES{CONNECTING};
   # SPAWN LISTENER
   $self->{socket} = MQTT::Socket->new($self->{brokerIP}.":".$self->{brokerPort});
   if (!defined $self->{socket}) {
      return 1;
   }
   
#      while(1) {
#         my $data = $self->{socket}->get_packet();
#         $self->processBuffer($data);
#      }
#      exit;
   
   
   my $fixed = &fixed($PKT_TYPES{CONNECT},0,$self->{QoS},0);
   # protocol name
   my $variable = &toUTF($protocol_name[$version]);
   $variable .= &tobyte($version);
   $variable .= &tobyte(0+$self->{cleanStart}*2+0*4);
   $variable .= &toword($self->{keepAlive});
   my $payload = &toUTF($self->{clientID});
   $self->{socket}->send_packet($fixed . $variable . $payload);
   # wait for the conack
   $self->processBuffer($self->{socket}->get_packet());

   #   while ($self->{state} == $STATES{CONNECTING}){}
   #use Data::Dumper;
   #while (1) { #$self->{state} != $STATES{DISCONNECTED}) {
   #
   #   # Poll the input queue
   #   if ($self->{socket}->is_ready()) {
   #      $self->processBuffer($self->{socket}->get_packet());
   #   }
   #   # Send the output queue
   #}
   return 0;
}

=head2 C<publish($topic,$QoS,$retain,$message)>

Publishes a message to the specified topic with the specified quality of service
and retain flag.

=cut

sub publish
{
   my $self = shift;
   my ($topic,$QoS,$retain,$message) = @_;
   my $fixed = &fixed($PKT_TYPES{PUBLISH},0,$QoS,$retain);
   my $variable = &toUTF($topic);
   my $MID = &newMessageID();

   if ($QoS==1 || $QoS==2)
   {
      $variable .= &toword($MID);
   }
   my $payload = $message;
   if ($QoS > 0)
   {
      $self->{publish_queue}->{$MID}->{QoS} = $QoS;
      $self->{publish_queue}->{$MID}->{state} = $PUBLISHED;
      $self->{publish_queue}->{$MID}->{payload} = $fixed.$variable.$payload;
   }
   
   $self->{socket}->send_packet($fixed . $variable . $payload);
   undef($fixed);
   undef($variable);
   undef($payload);
   undef($self->{publish_queue}->{$MID}->{QoS});
   undef($self->{publish_queue}->{$MID}->{state});
   undef($self->{publish_queue}->{$MID}->{payload});
   delete($self->{publish_queue}->{$MID});
   undef($MID);
}

=head2 C<subscribe($topic1,$QoS1,$topic2,$QoS2, ...)>

Subscribes to the desired topics using the corresponding quality of service.

=cut

sub subscribe
{
   my $self = shift;
   my @topic = @_;
   my $MID = &newMessageID();
   $self->{SUB_MID}{$MID} = 1;
#   print " > Subscribe ($MID)\n";
   my $fixed = fixed($PKT_TYPES{SUBSCRIBE},0,1,0);
   my $variable = pack "n", $MID;
   my $payload =  "";
   while (@topic) {
      my ($topic,$QoS) = splice @topic,0,2;
#      print "   - [$topic][$QoS]\n";
      $payload .= pack "n/a*C", $topic,$QoS;
   }
   $self->{socket}->send_packet($fixed . $variable . $payload);
}

=head2 C<disconnect()>

Cleanly disconnects from the broker.

=cut

sub disconnect
{
   my $self = shift;
   my $fixed = &fixed($PKT_TYPES{DISCONNECT},0,0,0);
   $self->{socket}->send_packet($fixed . "" . "");
   $self->{state} = $STATES{DISCONNECTED};
   $self->{socket}->disconnect();
   undef($self->{socket});
}
sub processBuffer
{
   my $self = shift;
   my $buffer = shift;
   my %packet = ();
   my $fixed = ord(substr($buffer,0,1));
   $packet{type} = ($fixed & 0xF0)/16;
   $packet{DUP} = ($fixed & 0x08)/8;
   $packet{QoS} = ($fixed & 0x06)/2;
   $packet{Retain} = ($fixed & 0x01);
   # $DQS = "DUP:$DUP,QoS:$QoS";
   
   my $offset=1;
   my $length=0;
   my $multiplier=1;
   my $byte;
   do 
   {
      $byte = ord(substr($buffer,$offset,1));
      $length += ($byte & 0x7F) * $multiplier;
      $multiplier *= 128;
      $offset++;
   } while ($byte & 0x80);
   $packet{length} = $length;
   $packet{buffer} = substr($buffer,$offset);
#    print "PROCESS: ".$PKT_NAMES{$packet{type}}."\n";
   my @return = &{$self->{pkt_parse_table}{ $packet{type}} }(\%packet);
   # handle if !defined $return[0] - lost connection?
#    print " ->handler: ".$PKT_NAMES{$packet{type}}."\n";

   return &{$self->{pkt_handler_table}{ $packet{type} }}(\%packet,@return);
}

=head2 C<listen_for_publishes()>

For a client that wants to receive publishes, this needs to be called to
process data as the broker sends it. When a publish is receieved, a call
is made to the callback_publish function specified in the constructor.

This function does not return.

=cut

sub listen_for_publishes
{
   my $self = shift;
   while(1) {
      my $data = $self->{socket}->get_packet();
      $self->processBuffer($data);
   }
}
      



####################################################################

#{{{ PARSERS

sub parse_connect
{
   my $self = shift;
   local($packet)=@_;
   local $buf = $packet->{buffer};
   local $QoS = $packet->{QoS};
   
   local($prot_name,$prot_version,$COMPSUB,$UID);
   # Version 2 variables
   local($TNCOMP,$keepAlive,$cleanStart);
   $prot_name=&fromUTF($buf);
   $buf=substr($buf,length($prot_name)+2);
   $prot_version=&frombyte($buf);
   $buf=substr($buf,1);
   $flags = &frombyte($buf);
   $TNCOMP = $flags & 0x01;
   $cleanStart = ($flags & 0x02)/2;
   $buf = substr($buf,1);
   $keepAlive = &fromword($buf);
   $buf = substr($buf,2);
   $UID=&fromUTF($buf);
   return ($prot_name,$prot_version,$TNCOMP,$keepAlive,$cleanStart,$UID);
}


####################################################################

# client

sub parse_connack
{
   my $self = shift;
   local($packet)=@_;
   local $buf = $packet->{buffer};
   local $QoS = $packet->{QoS};
   local ($TNCOMP,$rc);
   $TNCOMP=&frombyte($buf);
   $buf=substr($buf,1);
   $rc=&frombyte($buf);
   return ($TNCOMP,$rc);
}

####################################################################

# broker, client

sub parse_publish
{
   my $self = shift;
   local($packet)=@_;
   local $buf = $packet->{buffer};
   local $QoS = $packet->{QoS};
   local ($subject,$MID,$content);
   
   $subject=&fromUTF($buf);
   $buf=substr($buf,length($subject)+2);
   
   $MID=0;
   if ($QoS==1 || $QoS==2)
   {
      # there is a message ID
      $MID=&fromword($buf);
      $buf=substr($buf,2);
   }
   $content=$buf;
   # $MID will just be zero (which is invalid) if QoS==0
   return ($subject,$MID,$content);
   
}

####################################################################

# client, broker

sub parse_puback
{
   my $self = shift;
   local($packet)=@_;
   local $buf = $packet->{buffer};
   local $QoS = $packet->{QoS};
   local ($MID);
   $MID=&fromword($buf);
   return ($MID);
}

####################################################################


# client, broker

sub parse_pubrec
{
   my $self = shift;
   local($packet)=@_;
   local $buf = $packet->{buffer};
   local $QoS = $packet->{QoS};
   local ($MID);
   $MID=&fromword($buf);
   return ($MID);
}

####################################################################

# broker, client

sub parse_pubrel
{
   my $self = shift;
   local($packet)=@_;
   local $buf = $packet->{buffer};
   local $QoS = $packet->{QoS};
   local ($MID);
   $MID=&fromword($buf);
   return ($MID);
}

####################################################################

# client, broker

sub parse_pubcomp
{
   my $self = shift;
   local($packet)=@_;
   local $buf = $packet->{buffer};
   local $QoS = $packet->{QoS};
   local ($MID);
   $MID=&fromword($buf);
   return ($MID);
}

####################################################################

# broker

sub parse_subscribe
{
   my $self = shift;
   local($packet)=@_;
   local $buf = $packet->{buffer};
   local $QoS = $packet->{QoS};
   local (@subjects,$subject,@tmp,$MID);
   if ($version>=2)
   {
      $MID=&fromword($buf);
      $buf=substr($buf,2);
   }
   
   while (length($buf)!=0)
   {
      $subject=&fromUTF($buf);
      push(@subjects,$subject);
      $buf=substr($buf,length($subject)+2);
      if ($version>=2)
      {
         $reqQoS = &frombyte($buf);
         push(@subjects,$reqQoS);
         $buf=substr($buf,1);
      }
   }
   
   return ($MID,@subjects);
}

####################################################################

# client

sub parse_suback
{
   my $self = shift;
   local($packet)=@_;
   local $buf = $packet->{buffer};
   local $QoS = $packet->{QoS};
   local ($MID,@granted,$gQoS);
   
   # in version 2, we bring back the MID, and the granted QoS values
   $MID=&fromword($buf);
   $buf=substr($buf,2);
   
   # now pull off the vector of granted QoS values
   while (length($buf)!=0)
   {
      $gQoS = &frombyte($buf);
      push(@granted,$gQoS);
      $buf=substr($buf,1);
   }
   return ($MID,@granted);
}

####################################################################

# broker

sub parse_unsubscribe
{
   my $self = shift;
   local($packet)=@_;
   local $buf = $packet->{buffer};
   local $QoS = $packet->{QoS};
   local (@subjects,$subject,$MID);
   $MID=&fromword($buf);
   $buf=substr($buf,2);
   while (length($buf)!=0)
   {
      $subject=&fromUTF($buf);
      push(@subjects,$subject);
      $buf=substr($buf,length($subject)+2);
   }
   
   return ($MID,@subjects);
}

####################################################################

# client

sub parse_unsuback
{
   my $self = shift;
   local($packet)=@_;
   local $buf = $packet->{buffer};
   local $QoS = $packet->{QoS};
   local ($MID);
   $MID=&fromword($buf);
   $buf=substr($buf,2);
   return ($MID);
}

####################################################################

sub parse_illegal
{
   my $self = shift;
   print "received illegal message type 0\n";
   exit;
}

####################################################################

sub parse_disconnect
{
   my $self = shift;
   return ("");
}

####################################################################

sub parse_pingreq
{
   my $self = shift;
   return ("");
}

####################################################################

sub parse_pingresp
{
   my $self = shift;
   return ("");
}

####################################################################
#}}}
#
#{{{ HANDLERS

####################################################################

sub handle_illegal
{
   my $self = shift;
   my $packet = shift;
   print "received illegal message type $_[0]\n";
   exit;
}

####################################################################

sub handle_connack
{
   my $self = shift;
   my $packet = shift;
   my ($TNCOMP,$rc)=@_;
   if ($rc==0) 
   {
      $self->{state} = $STATES{CONNECTED};
      # clear timer out for the next use
      # $sent_time=0;
      if (defined $self->{callback_connected}) {
         &{$self->{callback_connected}};
      }
   }
   else
   {
      print $connackRC{$rc}."\n";
      exit;
   }
}

####################################################################

sub handle_publish
{
   my $self = shift;
   my $packet = shift;
   my ($subject,$MID,$content) = @_;
   print STDERR "Handling publish\n";
#   print " > Publish [$MID][$subject][$content]\n";
   if ($packet->{QoS} == 2)
   {
      $self->{QoS2}->{$MID}->{subject}=$subject;
      $self->{QoS2}->{$MID}->{content}=$content;
#      print " > QoS2 - sending pubrec\n";
      $self->send_pubrec($MID);
      return;
   }
   
   if ($packet->{QoS} == 1)
   {
      $self->{QoS1}->{$MID}->{subject}=$subject;
      $self->{QoS1}->{$MID}->{content}=$content;
#      print " > QoS1 - sending puback\n";
      $self->send_puback($MID);
   }
   if (defined $self->{callback_publish}) {
      &{$self->{callback_publish}}($subject,$content);
   }
}

####################################################################

sub handle_pubrel
{
   my $self = shift;
   my $packet = shift;
   my ($MID) = @_;

   print STDERR "Handling pubrel\n";
   
   if (exists $self->{QoS2}{$MID})
   {
      if (defined $self->{callback_publish}) {
         &{$self->{callback_publish}}($self->{QoS2}->{$MID}->{subject},$self->{QoS2}->{$MID}->{content});
      }
      delete $self->{QoS2}{$MID};
   }
   
   $self->send_pubcomp($MID);
}

####################################################################

sub handle_suback
{
   my $self = shift;
   my $packet = shift;
   my ($MID,@gQoS);
   
   ($MID,@gQoS)=@_;
   if (exists $self->{SUB_MID}{$MID})
   {
      delete $self->{SUB_MID}{$MID};
   }
   else
   {
      print "Unexpected MID on SUBACK: $MID\n";
      exit;
   }
}

####################################################################

sub handle_unsuback
{
   my $self = shift;
   my $packet = shift;
   my ($MID)=@_;
   if (exists $UNSUB_MID{$MID})
   {
      delete $UNSUB_MID{$MID};
   }
   else
   {
      print "Unexpected MID on UNSUBACK: $MID\n";
      exit;
   }
}

####################################################################

sub handle_pingreq
{
   my $self = shift;
   $self->send_pingresp;
}

####################################################################

sub handle_pingresp
{
   # we received a PINGRESP
}

####################################################################

sub handle_puback
{
   my $self = shift;
   my $packet = shift;
   print STDERR "Handle puback\n";
  
   local ($MID) = @_;
   if (exists $self->{publish_queue}->{$MID})
   {
      if ($self->{publish_queue}->{$MID}->{QoS}==1)
      {
         undef($self->{publish_queue}->{$MID}->{QoS});
         undef($self->{publish_queue}->{$MID}->{state});
         undef($self->{publish_queue}->{$MID}->{payload});
         delete $self->{publish_queue}->{$MID};
      }
      elsif ($self->{publish_queue}->{$MID}->{QoS}==2)
      {
         print "received unexpected PUBACK MID $MID to QoS2 message\n";
         exit;
      }
   }
   # if it doesn't exist, we just ignore it... that's the lightweight
   # simplicity of the QoS1 protocol flow
}


####################################################################

sub handle_pubrec
{
   my $self = shift;
   my $packet = shift;
   local ($MID)=@_;
   print STDERR "Handle pubrec\n";
   if (exists $self->{publish_queue}->{$MID} &&
      $self->{publish_queue}->{$MID}->{QoS}==2)
   {
      if ($self->{publish_queue}->{$MID}->{state}==$PUBLISHED)
      {
         if (!defined $self->send_pubrel($MID))
         {
            # TODO: Handle loss of conn
         }
         # it is correct to move on to the next state regardless of whether or not
         # we successfully sent the puback2
         $self->{publish_queue}->{$MID}->{state}=$RELEASED;
      }
      elsif ($self->{publish_queue}->{$MID}->{state}==$RELEASED)
      {
         undef($self->{publish_queue}->{$MID}->{QoS});
         undef($self->{publish_queue}->{$MID}->{state});
         undef($self->{publish_queue}->{$MID}->{payload});
         # just a duplicate puback1... ignore it
      }
      else
      {
         print "unexpected pubrec\n";
         exit;
      }
   }
   else
   {
      print "received unexpected PUBREC MID $MID\n";
      if (!exists $self->{publish_queue}->{$MID})
      {
         print "record not in outputQ\n";
      }
      exit;
   }
}

####################################################################

sub handle_pubcomp
{
   my $self = shift;
   my $packet = shift;

   local (@MID) = @_;
   print STDERR "Handle pubcomp\n";
   if (exists $self->{publish_queue}->{$MID})
   {
      if ($self->{publish_queue}->{$MID}->{QoS}==2 && $self->{publish_queue}->{$MID}->{state}==$RELEASED)
      {
         undef ($self->{publish_queue}->{$MID}->{QoS});
         undef ($self->{publish_queue}->{$MID}->{state});
         undef ($self->{publish_queue}->{$MID}->{payload});
         delete $self->{publish_queue}->{$MID};
      }
      elsif ($self->{publish_queue}->{$MID}->{QoS}==1 || $self->{publish_queue}->{$MID}->{state}==$PUBLISHED)
      {
         print "received unexpected PUBCOMP MID $MID to QoS2 message\n";
         exit;
      }
   }
   # if it doesn't exist we just ignore it... it must be a duplicate
}

####################################################################
#}}}
#
#{{{ SENDERS
sub send_pubackn
{
   my $self = shift;
   # send one of the QoS 2 acknowledgements
   # parameters are n (1-3), Message ID
   local($num,$MID) = @_;
   local ($msg);
   if ($num==1)
   {
      $msg=$PKT_TYPES{PUBACK1};
   }
   elsif ($num==2)
   {
      $msg=$PKT_TYPES{PUBACK2};
   }
   elsif ($num==3)
   {
      $msg=$PKT_TYPES{PUBACK3};
   }
   else
   {
      die "invalid PUBACKn code: $num\n";
   }
   my $fixed = &fixed($msg,0,0,0);
   # message ID
   my $variable = &toword($MID);
   my $payload = "";
   return $self->{socket}->send_packet($fixed . $variable . $payload);
}

####################################################################

sub send_pubrec
{
   my $self = shift;
   # acknowledge a publish with QoS 2
   # note that the DUP flag is not used in the protocol... we decided that
   # it is not needed. However, there is still a place for it, so we just
   # set it to 0
   # parameter is Message ID from PUBLISH
   local($MID) = @_;
   my $fixed = &fixed($PKT_TYPES{PUBREC},0,0,0);
   # message ID
   my $variable = &toword($MID);
   my $payload = "";
   return $self->{socket}->send_packet($fixed . $variable . $payload);
}


####################################################################

sub send_puback
{
   my $self = shift;
   # acknowledge a publish with QoS 1
   # note that the DUP flag is not used in the protocol... we decided that
   # it is not needed. However, there is still a place for it, so we just
   # set it to 0
   # parameter is Message ID from PUBLISH
   local($MID) = @_;
   my $fixed = &fixed($PKT_TYPES{PUBACK},0,0,0);
   # message ID
   my $variable = &toword($MID);
   my $payload = "";
   return $self->{socket}->send_packet($fixed . $variable . $payload);
}

####################################################################

sub send_pubcomp
{
   my $self = shift;
   # acknowledge a PUBREL in the QoS 2 protocol flow
   # note that the DUP flag is not used in the protocol... we decided that
   # it is not needed. However, there is still a place for it, so we just
   # set it to 0
   local($MID) = @_;
   my $fixed = &fixed($PKT_TYPES{PUBCOMP},0,0,0);
   # message ID
   my $variable = &toword($MID);
   my $payload = "";
   return $self->{socket}->send_packet($fixed . $variable . $payload);
}

####################################################################

sub send_pingreq
{
   my $self = shift;
   # request a ping response
   my $fixed = &fixed($PKT_TYPES{PINGREQ},0,0,0);
   my $variable="";
   my $payload="";
   return $self->{socket}->send_packet($fixed . $variable . $payload);
}

####################################################################

sub send_pingresp
{
   my $self = shift;
   # respond to a ping request
   my $fixed = &fixed($PKT_TYPES{PINGRESP},0,0,0);
   my $variable="";
   my $payload="";
   return $self->{socket}->send_packet($fixed . $variable . $payload);
}

####################################################################

sub send_pubrel
{
   my $self = shift;
   # acknowledge a PUBREC in the QoS 2 protocol flow
   # note that the DUP flag is not used in the protocol... we decided that
   # it is not needed. However, there is still a place for it, so we just
   # set it to 0
   # parameter is Message ID from PUBLISH
   local($MID) = @_;
   my $fixed = &fixed($PKT_TYPES{PUBREL},0,0,0);
   # message ID
   my $variable = &toword($MID);
   my $payload = "";
   return $self->{socket}->send_packet($fixed . $variable . $payload);
}

####################################################################
#}}}

####################################################################
1;
