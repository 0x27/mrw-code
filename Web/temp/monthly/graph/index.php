<!DOCTYPE html>
 <html lang="en" style="height: auto; width: 600px">
 <head>

   <title>Monthly Temperature</title>
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
     $.get('/temp/average?numberofdayshistory=20', function(d){
         var tempsToday = [];
         $(d).find('nextrow').each(function(){
         var $record = $(this);
         var $timestamp = $record.find('timestamp_of_day').text() * 1000;
         var $temperature = $record.find('average_temperature').text();
         tempsToday.push([$timestamp, $temperature]);
         });

         $.plot($("#thegraph"), [ tempsToday ], {
         xaxis: {mode: "time"},
         series: { points: {show: true}, lines: {show:true}, color: "rgba(135, 182, 217, 0.8)"},
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
  <!--  The DIV to hold the temperature graph  -->
  <div id="thegraph" style="width:600px;height:350px;"></div>
</body>
</html>

