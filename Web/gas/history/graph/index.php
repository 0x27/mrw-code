<!DOCTYPE html>
 <html lang="en" style="height: auto; width: 600px">
 <head>

   <title>Recent Gas Usage</title>
   <meta http-equiv="Content-Type" content="text/html; charset=utf-8">

   <!--  Load the jquery javascript code  -->

   <link type="text/css" href="css/redmond/jquery-ui-1.7.2.custom.css" rel="stylesheet" />
   <script src="/js/jquery-1.3.2.min.js" type="text/javascript"></script>
   <script src="/js/jquery-ui-1.7.2.custom.min.js" type="text/javascript"></script>
   <script language="javascript" type="text/javascript" src="/jquery/flot/jquery.flot.js"></script>

  <!--  jquery code to create the graph  -->
  <script id="source" language="javascript" type="text/javascript">
  function createGraph() {
     $.get('/gas/Ndays?numberofdayshistory=35', function(d){
         var gasToday = [];
         $(d).find('nextrow').each(function(){
         var $record = $(this);
         var $timestamp = $record.find('timestamp_of_day').text() * 1000;
         var $total_m3 = $record.find('total_gas_m3').text();

         gasToday.push([$timestamp, $total_m3]);
         });
         $.plot($("#thegraph"),
           [{
             data: gasToday,
             bars: {show: true, fill: true, barWidth: 24*60*60*550}
            }],
            {
             xaxis: {mode: "time", tickSize:[3, "day"]},
             series: { color: "rgba(135, 182, 217, 0.8)"},
             grid: { color: "rgba(135, 182, 217, 0.8)"}
            }
          );
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

