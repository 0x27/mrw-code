<!DOCTYPE html>
 <html lang="en" style="height: auto; width: 600px">
 <head>

   <title>Swap State Graph</title>
   <meta http-equiv="Content-Type" content="text/html; charset=utf-8">

   <!--  Load the jquery javascript code  -->

   <link type="text/css" href="css/redmond/jquery-ui-1.7.2.custom.css" rel="stylesheet" />
   <script src="/js/jquery-1.3.2.min.js" type="text/javascript"></script>
   <script src="/js/jquery-ui-1.7.2.custom.min.js" type="text/javascript"></script>
   <script language="javascript" type="text/javascript" src="/jquery/flot/jquery.flot.js"></script>

  <!--  jquery code to create the graph  -->
  <script id="source" language="javascript" type="text/javascript">
  function createGraph() {
     $.get('/swap/history', function(d){
          var swapHistory = [];
          var i = 0;
          $(d).find('element').each(function(){
             var $swap = $(this).text();
             swapHistory.push([i, $swap]);
             i++;
           });
          $.plot($("#thegraph"), [ swapHistory ], {
            //xaxis: {mode: "time"},
            //yaxis: {min: 0, max: 1.5},
            series: { lines: {show: true}, points: {show: false}, color: "rgba(135, 182, 217, 0.8)"},
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

