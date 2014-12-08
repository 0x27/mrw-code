package MQTT::Socket;

=pod

=head1 NAME

MQTT::Socket - a module to handle the socket connection of MQTT::Client.

=head1 COPYRIGHT

(C) Copyright IBM Corp. 2006, 2008

=head1 VERSION

1.0.1

=head1 AUTHOR

Nicholas O'Leary - nick_oleary@uk.ibm.com

=cut

use strict;
use IO::Socket::INET;
use IO::Select;
use MQTT::Utils;

our @ISA = qw(IO::Socket::INET);
our $VERSION   = 0.1;

sub new
{
   my ($module, $broker)=@_;
   my $self = $module->SUPER::new
   (
      PeerAddr => $broker,
      Proto    => 'tcp',
      Type     => SOCK_STREAM
      );
   
   if ($self)
   {
      $self->autoflush(1);
      return $self;
   }
   else
   {
      warn "connect error: $!\n";
      return;
   }
}

sub get_packet
{
   return packet(@_);
}
sub send_packet
{
   return packet(@_);
}
sub disconnect
{
   my $self = shift;
   $self->shutdown(2);
}

sub packet
{
   my ($self, $buffer) = @_;
   
   if (1 == @_)
   {
      my $remaining;
      my $header = $self->get(1);
      return unless defined $header;
      my $multiplier = 1;
      my $remaining = 0;
      my $digit;
      {
         $digit = $self->get(1);
         $header .= $digit;
         return unless defined $digit;
         $remaining += (unpack("C",$digit) & 127) * $multiplier;
         $multiplier *= 128;
         redo if ((unpack("C",$digit) & 128) != 0);
      }
      
      my $buffer = $self->get($remaining); 
      return unless defined $buffer; 
#      print "{\n [".unpack("C",$header)."]($remaining)\n [$buffer]\n}\n";
      return $header.$buffer;  
   }
   else
   {
      my $fixed;
      my $remaining = length($buffer) - 1;
      {
         my $size = $remaining % (1 << 7);
         $remaining = $remaining >> 7;
         $size |= (1 << 7) if $remaining;
         $fixed .= pack "C", $size;
         redo if $remaining;
      }
      substr $buffer, 1, 0, $fixed;
      return $self->put($buffer);
   }
}


sub get
{
   my ($self, $bytes) = @_;
   my $buffer;
   my $offset = 0;
   my $read = 0;
   while ($bytes > 0)
   {
      
      $read = $self->sysread($buffer, $bytes, $offset);
      $offset += $read;
      $bytes -= $read;
      
      if ($self->error)
      {
         warn sprintf "socket closed: %d '%s'\n", $!, $!;
         $self->close;
         return;
      }
   }
   return $buffer;
}

sub put
{
   my ($self, $buffer) = @_;
   
   my $offset = 0;
   while ($offset < length $buffer)
   {
      $offset += $self->syswrite($buffer,length($buffer), $offset);
      if ($self->error)
      {
         warn sprintf "socket closed: %d '%s'\n", $!, $!;
         $self->close;
         return;
      }
   }
   return 0;
}

sub is_ready
{
   my $self = shift;
   return IO::Select->new($self)->can_read(0);
}

