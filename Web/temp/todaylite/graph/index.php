  <!--  The DIV to hold the temperature graph  -->

  <div id="thegraph" style="width:600px;height:300px;"></div>

  <!--  jquery code to create the graph  -->

  <script id="source" language="javascript" type="text/javascript">
  $(function () {
    $.get('/temp/todaylite', function(d){
       var tempsToday = [];
       $(d).find('nextrow').each(function(){ 
         var $record = $(this);     
         var $timestamp = $record.find('timestamp').text() * 1000;
         var $temperature = $record.find('temperature').text();
         tempsToday.push([$timestamp, $temperature]);
       });

       $.plot($("#thegraph"), [ tempsToday ], {
         xaxis: {mode: "time"},
         series: { points: {show: true}, lines: {show:true}, color: "rgba(135, 182, 217, 0.8)"},
         grid: { color: "rgba(135, 182, 217, 0.8)"} 
       });
    });
  });

  </script>

