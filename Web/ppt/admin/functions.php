<script type="text/javascript">
   
    $("#playlistsortable").selectable();

    loadusers();

    function loadusers()
    {
       $.get('/ppt/current-feed-owners/listfollowrequests.php', function(d){
          var $tableContent = "<table border=1><tr><td width=200><b>Screen Name</b></td><td width=200><b>ID</b></td><td width=300><b>OAuth Token</b></td><td><b>OAuth Token Secret</b></td></tr>";
          $(d).find('nextuser').each(function(){

                var $entry = $(this);
                var id = $entry.find("id").text();
                var screen_name = $entry.find('screen_name').text();
                var oauth_token = $entry.find('oauth_token').text();
                var oauth_token_secret = $entry.find('oauth_token_secret').text();

                $tableContent = $tableContent + '<tr><td>' + screen_name + '</td><td>' + id + '</td><td>' + oauth_token + '</td><td>' + oauth_token_secret + '</td></tr>';
             });
          $tableContent = $tableContent + "</table>";
          $('#playlistsortable').html($tableContent);
       });
    }

$(document).ready(function(){

  // Tabs
  $('#tabs').tabs({
    beforeActivate: function(event, ui) {
      if ( $(ui.newTab).html().indexOf("Temp &amp; Humidity") != -1 && !tempGraphsLoaded ){
        loadAverageTemperatureGraph();
        loadTemperatureGraph();
        loadHumidityGraph();
        tempGraphsLoaded = true;
      }
      else if ( $(ui.newTab).html().indexOf("Electricity") != -1 && !electricityGraphsLoaded ){
        loadAverageElectricityGraph();
        loadElectricityGraph(24);
        loadElectricityConsumptionStats();
        electricityGraphsLoaded = true;
      }
      else if ( $(ui.newTab).html().indexOf("Gas") != -1 && !gasGraphsLoaded ){
        loadGasGraph();
        loadAverageGasGraph();
        loadGasConsumptionStats();
        gasGraphsLoaded = true;
      }
      else if ( $(ui.newTab).html().indexOf("Status") != -1 && !voltageGraphsLoaded ){
        loadVoltageGraph();
        voltageGraphsLoaded = true;
      }
    }
  });

  $('#tabs').tabs('option', 'active', 0);
});

</script>

