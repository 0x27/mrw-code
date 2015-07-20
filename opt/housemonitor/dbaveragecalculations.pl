########################################################################

# A script which calculates average and total values and puts them into
# dedicated tables to make the data easier/quicker to consume 

########################################################################

use DBI;
use POSIX strftime;

my $db_name = 'mydata';
my $db_user = 'dbstats';
my $db_type = 'mysql';
my $db = DBI->connect("DBI:$db_type:$db_name",$db_user) or die "Couldn't connect to database: " . DBI->errstr;
my $numberOfDays = 100;
my $i = 0;
my $daysSinceCachedDataRecorded = 0;

print (localtime);
print "\nCalculating daily stats for the house\n";

while ($i < $numberOfDays) {

  my $kwhsTotal = 0.0;
  my $solarWhsTotal = 0.0;
  my $gasMCubedTotal = 0.0;
  my $tempTotal = 0.0;
  my $averageTemp = 0.0;
  my $lastTimestamp = 0;
  my $firstTimestamp = 0;
  my $lastWatts = 0;
  my $lastSolarWatts = 0;
  my $lastTemp = 0;
  my $timestampDiff = 0;
  my $nextKwhs = 0.0;
  my $nextSolarWhs = 0.0;
  my $nextTemp = 0.0;

  my $select_averages = $db->prepare_cached('SELECT *,unix_timestamp(timestamp) FROM daily_stats WHERE (timestampadd(DAY,-' . ($numberOfDays - $i) . ',curdate()) = date(timestamp))');   

  my $select_watts_handle = $db->prepare_cached('SELECT unix_timestamp(timestamp),watts,timestamp FROM electricity WHERE timestamp > timestampadd(DAY,-' . ($numberOfDays - $i) . ',curdate()) and timestamp < timestampadd(DAY,-' . ($numberOfDays - ($i + 1)) . ',curdate()) ORDER BY timestamp');

  my $select_solar_watts_handle = $db->prepare_cached('SELECT unix_timestamp(timestamp),watts,timestamp FROM solar WHERE timestamp > timestampadd(DAY,-' . ($numberOfDays - $i) . ',curdate()) and timestamp < timestampadd(DAY,-' . ($numberOfDays - ($i + 1)) . ',curdate()) ORDER BY timestamp');

  my $select_temp_handle = $db->prepare_cached('SELECT unix_timestamp(timestamp),temperature,timestamp FROM temperature WHERE timestamp > timestampadd(DAY,-' . ($numberOfDays - $i) . ',curdate()) and timestamp < timestampadd(DAY,-' . ($numberOfDays - ($i + 1)) . ',curdate()) ORDER BY timestamp');

  my $select_gas_handle = $db->prepare_cached('SELECT unix_timestamp(timestamp),count(tick),timestamp FROM gas WHERE (timestampadd(DAY,-' . ($numberOfDays - $i) . ',curdate()) = date(timestamp)) ORDER BY timestamp');

  $select_averages->execute();

  $select_averages->bind_columns(undef, \$cache_timestamp, \$cache_avgtemp, \$cache_kwhs, \$cache_gasmcubed, \$cache_solarwhs, \$cache_unixtimestamp);

  # If we don't have cached values stored, calculate them from the raw 
  # data and store the results back into the cache
  if (!$select_averages->fetch()) {

    $daysSinceCachedDataRecorded++;

    # Query the database for the raw electricity usage data
    $select_watts_handle->execute();

    $select_watts_handle->bind_columns(undef, \$unix_timestamp, \$watts, \$timestamp);

    # LOOP THROUGH RESULTS
    while($select_watts_handle->fetch()) {
      #print($timestamp . "\n");
      if ($lastTimestamp == 0) {
        # Do nothing
      } else {
        $timestampDiff = $unix_timestamp - $lastTimestamp; 
        # print "Diff between $timestamp and $lastTimestamp = $timestampDiff\n";
        # print "Last ts = $ts\n";
        # $nextKwhs = ($timestampDiff / 1000 / 60 / 60) * $lastWatts;
        $nextKwhs = ((1 / 60 / 60) * $timestampDiff) * ($lastWatts / 1000);
        $kwhsTotal = $kwhsTotal + $nextKwhs;
      }
      $lastTimestamp = $unix_timestamp;
      $lastWatts = $watts;
      #print "$timestamp, $watts, $nextKwhs\n";
    } 

    print "Total calculated kwhs on " . substr($cache_timestamp,0,10) . "(+" . $daysSinceCachedDataRecorded . ") = " . $kwhsTotal . "\n";

    $lastTimestamp = 0;

    # Query the database for the raw solar generation data
    $select_solar_watts_handle->execute();

    $select_solar_watts_handle->bind_columns(undef, \$solar_unix_timestamp, \$solarwatts, \$solartimestamp);

    # LOOP THROUGH RESULTS
    while($select_solar_watts_handle->fetch()) {
      # print($solartimestamp . "\n");
      if ($lastTimestamp == 0) {
        # Do nothing
      } else {
        $timestampDiff = $solar_unix_timestamp - $lastTimestamp; 

        #if ($solarwatts == $lastSolarWatts) {
           $nextSolarWhs = ((1 / 60 / 60) * $timestampDiff) * $lastSolarWatts;
           $solarWhsTotal = $solarWhsTotal + $nextSolarWhs;
        #}
      }
      $lastTimestamp = $solar_unix_timestamp;
      $lastSolarWatts = $solarwatts;
    } 

    print "Total generated solar watt hours on " . substr($cache_timestamp,0,10) . " = " . $solarWhsTotal . "\n";

    $lastTimestamp = 0;

    # Query the database for the raw temperature data
    $select_temp_handle->execute();

    $select_temp_handle->bind_columns(undef, \$unix_timestamp2, \$temp, \$timestamp2);

    # LOOP THROUGH RESULTS
    while($select_temp_handle->fetch()) {
      # print($timestamp2 . "\n");
      if ($lastTimestamp == 0) {
        # Do nothing
        $firstTimestamp = $unix_timestamp2;
      } else {
        $timestampDiff = $unix_timestamp2 - $lastTimestamp; 
        $nextTemp = $timestampDiff * $lastTemp;
        $tempTotal = $tempTotal + $nextTemp;
      }
      $lastTimestamp = $unix_timestamp2;
      $lastTemp = $temp;
    } 

    # Whatever the number of remaining seconds in the day, multiply
    # by the last temperature that was recorded...
    $tempTotal = $tempTotal + ($lastTemp * (86400 - ($lastTimestamp - $firstTimestamp))); 

    # Divide the total temperature by the number of seconds in a day to get the average
    $averageTemp = $tempTotal / 86400;

    print "Average calculated temperature on " . substr($cache_timestamp,0,10) . " = " . $averageTemp . "\n";

    # Query the database for the raw gas data
    $select_gas_handle->execute();

    $select_gas_handle->bind_columns(undef, \$unix_timestamp3, \$gasticks, \$timestamp3);

    # LOOP THROUGH RESULTS (there should be exactly 1)
    while($select_gas_handle->fetch()) {
      # Take the count of the gas ticks and multiple by 0.01 to get metres-cubed
      $gasMCubedTotal = $gasticks * 0.01;
    } 

    print "Total gas (metres^3) used on " . substr($timestamp3,0,10) . " = " . $gasMCubedTotal . "\n";

    if ($kwhsTotal == 0 && $averageTemp == 0 && $gasMCubedTotal == 0 && $solarWhsTotal == 0) {
      print "No data was collected today at all. Ignoring.\n";
    } else {
      print "Updating cache database to " . int($kwhsTotal * 1000) . " and " . int($averageTemp * 1000) . " and " . int($gasMCubedTotal * 1000) . " and " . int($solarWhtsTotal) . "\n";
      # Update the daily stats table so we've got this value cache
      my $query = 'INSERT INTO daily_stats(timestamp, totalwatthrs, avgtemp, totalgasmcubed, totalsolarwatthrs) values (timestampadd(DAY, ' . $daysSinceCachedDataRecorded . ', \'' . $cache_timestamp . '\'), \'' . int($kwhsTotal * 1000) . '\', \'' . int($averageTemp * 1000) . '\', \'' . int($gasMCubedTotal * 1000) . '\', \'' . int($solarWhsTotal) . '\')';
      print "Query:\n" . $query . "\n";
      my $insert_into_daily_stats_handle = $db->prepare_cached($query);
      $insert_into_daily_stats_handle->execute();
      $insert_into_daily_stats_handle->finish();
    }

    $lastTimestamp = 0;
  } else {
    print "Total cached kwhs on " . substr($cache_timestamp,0,10) . " = " . ($cache_kwhs / 1000) . ", average temp = " . ($cache_avgtemp / 1000) . ", total gas (metres-cubed) = " . ($cache_gasmcubed) . ", total solar whs = " . ($cache_solarwhs) . "\n";
    $daysSinceCachedDataRecorded = 0;
  }

  $select_watts_handle->finish();
  $select_solar_watts_handle->finish();
  $select_temp_handle->finish();
  $select_averages->finish();
  $select_gas_handle->finish();
  $i++;
}

$db->disconnect();

