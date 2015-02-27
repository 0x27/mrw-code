package CurrentCost;

require "nupub.pl";

sub new
{
	my $class = shift || die;
	my %params = @_;
	die "broker not set" unless $params{broker};
	die "name not set" unless $params{name};
	my $port = $params{port} || 1883;
	my $keepalive = $params{keepalive} || 120;
	my $debug = $params{debug} || 0;
    
	my $self = {
			_broker => $params{broker},
			_name   => $params{name},
			_port   => $port,
			_keepalive => $keepalive,
			_debug  => $debug,
	};
	bless $self, $class;

	return $self;
}

sub connect
{
	my $self = shift || die;

        if (&upub_connect ("currentcost_$self->{_name}",$self->{_broker},$self->{_port}))
	{
		if($self->{_debug}){
			print "connected OK\n";
		}
		$self->{_connected}=1;
		return 1;
	} else {
		print "couldnt connect\n";
		$self->{_connected}=0;
		return 0;
	}
}

sub publish_all
{
	my $self = shift || die;
	my $value = shift || die "no value to publish";

	if(! $self->{_connected}){
		$connect = $self->connect();
		if(!$connect){
			print "Not connected, not publishing\n";
			return 0;
		}
	}

	&upub_publish ("local/currentcostraw","n",$value);
}

sub disconnect
{
	my $self = shift||die;
	if($self->{_debug}){
		print "disconnecting\n";
	}
	&upub_disconnect();
}

sub DESTROY
{
	my $self = shift||die;
	$self->disconnect();	
}


1;
