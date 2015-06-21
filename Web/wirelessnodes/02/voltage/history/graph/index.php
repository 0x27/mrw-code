<!DOCTYPE html>
 <html lang="en" style="height: auto; width: 600px">
 <head>

   <title>Wireless Sensor Voltage</title>
   <meta http-equiv="Content-Type" content="text/html; charset=utf-8">

   <!--  Load the jquery javascript code  -->

   <link type="text/css" href="/css/redmond/jquery-ui-1.7.2.custom.css" rel="stylesheet" />
   <script src="/js/jquery-1.3.2.min.js" type="text/javascript"></script>
   <script src="/js/jquery-ui-1.7.2.custom.min.js" type="text/javascript"></script>
   <script language="javascript" type="text/javascript" src="/jquery/flot/jquery.flot.js"></script>

  <!--  jquery code to create the graph  -->
  <script id="source" language="javascript" type="text/javascript">
  var sensorVoltages = [];
  function createGraph() {
     $.get('/wirelessnodes/02/voltage/Ndays?numberofdayshistory=950', function(d){
          sensorVoltages[0] = [];
          $(d).find('nextrow').each(function(){
             var $record = $(this);
             var $timestamp = $record.find('timestamp').text() * 1000;
             var $voltage = $record.find('voltage').text();
             sensorVoltages[0].push([$timestamp, $voltage]);
           });

           /*$.get('/wirelessnodes/03/voltage/Ndays?numberofdayshistory=150', function(d){
              sensorVoltages[1] = [];
              $(d).find('nextrow').each(function(){
                 var $record = $(this);
                 var $timestamp = $record.find('timestamp').text() * 1000;
                 var $voltage = $record.find('voltage').text();
                 sensorVoltages[1].push([$timestamp, $voltage]);
              });

              $.get('/wirelessnodes/04/voltage/Ndays?numberofdayshistory=150', function(d){
                 sensorVoltages[2] = [];
                 $(d).find('nextrow').each(function(){
                    var $record = $(this);
                    var $timestamp = $record.find('timestamp').text() * 1000;
                    var $voltage = $record.find('voltage').text();
                    sensorVoltages[2].push([$timestamp, $voltage]);
                 });*/

           $.plot($("#thegraph"), [
           { data: sensorVoltages[0],
              points: {show: false},
              lines: {show: true},
              label: "Loft"},
            { data: sensorVoltages[1],
              points: {show: false},
              lines: {show: true},
              label: "Lg",
              color: "rgba(195, 110, 50, 1.4)"},
            { data: sensorVoltages[2],
              points: {show: false},
              lines: {show: true},
              label: "G",
              color: "rgba(110, 180, 30, 1.4)"}
            ],
          {
            legend: { position: "ne", noColumns: 3, margin: 7 },
            xaxis: { mode: "time" },
            yaxis: {min: 2.6, max: 4.6},
            series: { points: {show: false}, lines: {show:true}, color: "rgba(135, 182, 217, 0.8)"},
            grid: { color: "rgba(135, 182, 217, 0.8)"}
          });
        // });
       //});
       });

  }
  </script>

  <script language="javascript" type="text/javascript">
     createGraph();
  </script>
</head>
<body>
  <!--  The DIV to hold the temperature graph  -->
  <div id="thegraph" style="width:600px;height:350px;"></div>
</body>
</html>

