
while (1) {

  my $output = `/home/mwhitehead/opt/housemonitor/averageCPUTimeWaitingForIO.sh`;
  if ($output > 50) {
    print STDERR "Disk IO time is running very high\n";
    system("/home/mwhitehead/opt/sendtweet/sendtweet.sh", "Disk IO time is running very high on the server");
  } else {
    print STDERR "Disk IO time is OK\n";
  }

  sleep(60);
}
