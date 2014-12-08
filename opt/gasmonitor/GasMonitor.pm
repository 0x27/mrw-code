package GasMonitor;

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
    
        my $db_name = 'mydata';
        my $db_user = 'cc';
        my $db_type = 'mysql';
        my $db = DBI->connect("DBI:$db_type:$db_name",$db_user) or die "Couldn't connect to database: " . DBI->errstr;

        # Since gas readings often don't come for days, we need to automatically 
        # reconnect to the DB after the default 8 hour timeout
        $db->{'AutoCommit'} = 1;
        $db->{mysql_auto_reconnect} = 1;

        # Variables used for calculating a rolling average
        my $size = 50;
        my @samples = (0) x $size; 
        my $total = 0;
        my $index = 0;
        my $average = 0;
        my $stateready = 0;
        my $statereadycount = 0;

       print STDERR "Gas monitor app waiting for ticks\n";

	my $self = {
			_broker => $params{broker},
			_name   => $params{name},
			_port   => $port,
			_keepalive => $keepalive,
			_debug  => $debug,
                        _insert_tick_handle => $db->prepare_cached('INSERT INTO gas (`tick`) VALUES (?)'),
                        _total => $total,
                        _index => $index,
                        _average => $average,
                        _size => $size,
                        _samples => $samples,
                        _stateready => $stateready,
                        _statereadycount => $statereadycount,
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
	}else{
		print "couldnt connect\n";
		$self->{_connected}=0;
		return 0;
	}
}

sub publish_gas
{
	my $self = shift || die;
	my $value = shift || die "no value to publish";

        # Check the prefix that the arduino adds to the sensor reading is present
        if ($value =~ /Gas sensor reading/) {

          # Strip the prefix 'Gas sensor reading:' from the value
          $value = substr($value, 20);

          # Trim trailing whitespace
          $value =~ s/\s+$//;

	  if(! $self->{_connected}){
		$connect = $self->connect();
		if(!$connect){
			print "Not connected, not publishing\n";
			return 0;
		}
	  }

	  if($self->{_debug}){
             print "total at start of algorithm: '$self->{_total}'\n";
             print "index: '$self->{_index}'\n";
             print "value at the current index: '@{$self->{_samples}}[$self->{_index}]'\n";
	     print "value we've read: '$value'\n";
             my $temp = $value;
             my $temp2 = @{$self->{_samples}}[$self->{_index}];
             print "value at the current index: '$temp2'\n";
	     print "value we've read: '$temp'\n";
             $self->{_total} = $self->{_total} + $temp;
	     print "Total after adding new value is '$self->{_total}'\n";
             $self->{_total} = $self->{_total} - $temp2;
	     print "Total after deducting the old value is '$self->{_total}'\n";
             @{$self->{_samples}}[$self->{_index}] = $value;
	     print "Total after adding new value to index '$self->{_index}' is '$self->{_total}'\n";
             $self->{_average} = $self->{_total} / $self->{_size};
             print "rolling average: '$self->{_average}'\n";
             print "total: '$self->{_total}'\n";

             # If we saw a tick recently, we're no longer ready to count another
             # tick until at least n samples have been taken...
             if (!$self->{_stateready}) {
                $self->{_statereadycount}++;

                # Once we've taken n new samples, we're ready to count another tick
                if ($self->{_statereadycount} == $self->{_size}) {
                   $self->{_stateready} = 1;
                   $self->{_statereadycount} = 0;
                }
             }

             $self->{_index}++;
             if ($self->{_index} == $self->{_size}) {
               $self->{_index} = 0;
             }

             if ($self->{_stateready} && (($self->{_average} - $value) > 12)) {
                my $nowtime = localtime;
                print STDERR $nowtime . "******* '$value' is below threshold - TICK *********\n";
	        &upub_publish ("local/gasmonitor/tick","n",$value);

                # Update the database
                $self->{_insert_tick_handle}->execute(1);

                #We're not in state ready at least n samples have been read again
                $self->{_stateready} = 0;
             }
	  }
        } else {
          print STDERR "Value didn't start with expected prefix string. Let's hope we can carry on OK\n";
          print STDERR "(Value was '" . $value . "')";
          #die "Value didn't start with the expected prefix";
        }
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
