<!DOCTYPE html>
 <html lang="en" style="height: auto; width: 600px">
 <head>

   <title>Electricity Usage Today</title>
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
     $.get('/power/today?numhoursfrommidnight=24', function(d){
       var powerToday = [];
       var firstTimestamp = 0;
       var lastTimestamp = 0;
       $(d).find('nextrow').each(function(){
         var $record = $(this);
         var $timestamp = $record.find('timestamp').text() * 1000;
         var $watts = $record.find('watts').text();
         if (firstTimestamp == 0) firstTimestamp = $timestamp;
         lastTimestamp = $timestamp;
         powerToday.push([$timestamp, $watts]);
       });
      
       powerToday.push([lastTimestamp, 0]);
       powerToday.push([firstTimestamp + (22 * 60 * 60 * 1000), 0]);

       $.plot($("#thegraph"), [ powerToday ], {
         xaxis: {mode: "time"},
         series: { points: {show: false}, lines: {show:true}, color: "rgba(135, 182, 217, 0.8)"},
         grid: { color: "rgba(135, 182, 217, 0.8)"}
       });
     });
   }
  </script>

  <script language="javascript" type="text/javascript">
     createGraph();
  </script>
</head>
<body>
  <!--  The DIV to hold the power graph  -->
  <div id="thegraph" style="width:900px;height:350px;"></div>
</body>
</html>

