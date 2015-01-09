<!DOCTYPE html>
 <html lang="en" style="height: auto; width: 600px">
 <head>

   <title>Recent Soil Water Content</title>
   <meta http-equiv="Content-Type" content="text/html; charset=utf-8">

   <!--  Load the jquery javascript code  -->

   <link type="text/css" href="css/redmond/jquery-ui-1.7.2.custom.css" rel="stylesheet" />
   <script src="/js/jquery-1.3.2.min.js" type="text/javascript"></script>
   <script src="/js/jquery-ui-1.7.2.custom.min.js" type="text/javascript"></script>
   <script language="javascript" type="text/javascript" src="/jquery/flot/jquery.flot.js"></script>

  <!--  jquery code to create the graph  -->
  <script id="source" language="javascript" type="text/javascript">
  function createGraph() {
    var moistureToday = [];
       var avgMoistureToday = [];
       $.get('/moisture/history', function(d){
         $(d).find('nextrow').each(function(){
            var $record = $(this);
            var $timestamp = $record.find('timestamp').text() * 1000;
            var $moisture = $record.find('moisture').text();
            if ($moisture > 100) {
              moistureToday.push([$timestamp, $moisture]);
            }
         });
         $.get('/moisture/average', function(d){
           $(d).find('nextrow').each(function(){
              var $record = $(this);
              var $timestamp = $record.find('timestamp').text() * 1000;
              var $moisture = $record.find('moisture').text();
              avgMoistureToday.push([$timestamp, $moisture]);
           });
           $.plot($("#thegraph"), [
             { data: moistureToday,
               points: {show: false},
               lines: {show: true},
               color: "rgba(110, 180, 30, 1.4)"}
             ],
             {
               xaxis: { mode: "time" },
               yaxis: { min: 0 },
               series: { points: {show: true}, lines: {show:true}, color: "rgba(135, 182, 217, 0.8)"},
               grid: { color: "rgba(135, 182, 217, 0.8)"}
           });
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

