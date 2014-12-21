<!DOCTYPE html>
 <html lang="en" style="height: auto; width: 600px">
 <head>

   <title>Historic Electricity Usage</title>
   <meta http-equiv="Content-Type" content="text/html; charset=utf-8">

   <!--  Load the jquery javascript code  -->

   <link type="text/css" href="css/redmond/jquery-ui-1.7.2.custom.css" rel="stylesheet" />
   <script src="/js/jquery-1.3.2.min.js" type="text/javascript"></script>
   <script src="/js/jquery-ui-1.7.2.custom.min.js" type="text/javascript"></script>
   <!--<script language="javascript" type="text/javascript" src="jquery/flot/jquery.js"></script>-->
   <script language="javascript" type="text/javascript" src="/jquery/flot/jquery.flot.js"></script>

  <!--  jquery code to create the graph  -->
  <script id="source" language="javascript" type="text/javascript">
   function createGraph() {
       $.get('/power/Ndays?numberofdayshistory=28', function(d){
         var powerToday = [];
         var lowPowerNatAvg = [];
         var mediumPowerNatAvg = [];
         var highPowerNatAvg = [];
         $(d).find('nextrow').each(function(){
         var $record = $(this);
         var $timestamp = $record.find('timestamp_of_day').text() * 1000;
         var $watts = $record.find('total_watt_hours').text() / 1000;
         powerToday.push([$timestamp, $watts]);
         lowPowerNatAvg.push([$timestamp, 5.75]);
         mediumPowerNatAvg.push([$timestamp, 9.04]);
         highPowerNatAvg.push([$timestamp, 13.97]);
       });

       $.plot($("#thegraph"),
           [{
             data: powerToday,
             bars: {show: true},
             points: {show:true},
             lines: {show:false}
            },
            {
             data: lowPowerNatAvg,
             bars: {show: false},
             points: {show: false},
             lines: {show: true},
             //label: "Low-Use National Average",
             color: "rgba(40, 230, 30, 1.4)"
            },
            {
             data: mediumPowerNatAvg,
             bars: {show: false},
             points: {show: false},
             lines: {show: true},
             //label: "National Average",
             color: "rgba(230, 130, 10, 1.8)"
            },
            {
             data: highPowerNatAvg,
             bars: {show: false},
             points: {show: false},
             lines: {show: true},
             //label: "High-Use National Average",
             color: "rgba(180, 20, 20, 1.8)"
            }],
            {
             xaxis: {mode: "time"},
             series: { points: {show: false}, lines: {show:true}, color: "rgba(135, 182, 217, 0.8)"},
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
  <!--  The DIV to hold the power graph  -->
  <div id="thegraph" style="width:600px;height:350px;"></div>
</body>
</html>

