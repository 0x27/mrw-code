<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
 <head>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8">
    <title></title>

<link type="text/css" href="/css/redmond/jquery-ui-1.7.2.custom.css" rel="stylesheet" />
   <script src="/js/jquery-1.3.2.min.js" type="text/javascript"></script>
   <script src="/js/jquery-ui-1.7.2.custom.min.js" type="text/javascript"></script>
   <script language="javascript" type="text/javascript" src="/jquery/flot/jquery.flot.js"></script>


  <!--  jquery code to create the graph  -->

  <script id="source" language="javascript" type="text/javascript">
     var idTToday = [];
     var lfTToday = [];
     var lgTToday = [];
     var hToday = [];
     var cT = 0;

     function plotGraph() {

       // Only plot the graph if both arrays of data exist

         // Add a new data point to the end of which ever data set needs it, to make
         // sure both graph plots have a data point at the same final timestamp
         if (lfTToday[lfTToday.length - 1][0] >
             idTToday[idTToday.length - 1][0]) {

             idTToday.push([lfTToday[lfTToday.length - 1][0], idTToday[idTToday.length - 1][1]]);
             hToday.push([lfTToday[lfTToday.length - 1][0], hToday[hToday.length - 1][1]]);
         } else {
             lfTToday.push([idTToday[idTToday.length - 1][0], lfTToday[lfTToday.length - 1][1]]);
             hToday.push([idTToday[idTToday.length - 1][0], hToday[hToday.length - 1][1]]);
         }

         // Do the same for the beginning of the arrays, to make sure
         // line on the graph starts at the same timestamp.
         if (lfTToday[0][0] < idTToday[0][0]) {
             idTToday.unshift([lfTToday[0][0], idTToday[0][1]]);
             hToday.unshift([lfTToday[0][0], hToday[0][1]]);
         } else {
             lfTToday.unshift([idTToday[0][0], lfTToday[0][1]]);
             hToday.unshift([idTToday[0][0], hToday[0][1]]);
         }

         var i = 0;
         for (i = 0; i < idTToday.length; i++) {
            if (parseInt(idTToday[i][1]) > parseInt(cT)) {
               cT = parseInt(idTToday[i][1]);
            }
         }
         for (i = 0; i < lfTToday.length; i++) {
            if (parseInt(lfTToday[i][1]) > parseInt(cT)) {
               cT = parseInt(lfTToday[i][1]);
            }
         }
         for (i = 0; i < lgTToday.length; i++) {
            if (parseInt(lgTToday[i][1]) > parseInt(cT)) {
               cT = parseInt(lgTToday[i][1]);
            }
         }

         // Add 5 degrees to the current maximum so the graph is scaled sensibly (and
         // more importantly make sure the labels don't overlap the data)
         var xRange = 5 + parseInt(cT);

         $.plot($("#thegraph"), [
         { data: idTToday,
           points: {show: false},
           lines: {show: true},
           label: "K"},
         { data: lfTToday,
           points: {show: false},
           lines: {show: true},
           color: "rgba(110, 180, 30, 1.4)",
           label: "Lf"},
         { data: lgTToday,
           points: {show: false},
           lines: {show: true},
           color: "rgba(195, 110, 50, 1.4)",
           label: "Lg"},
         { data: hToday,
           points: {show: false},
           lines: {show: true},
           color: "rgba(150, 110, 30, 1.4)",
           label: "H"}
         ],
         {
           legend: { position: "ne", noColumns: 4, margin: 6 },
           xaxis: { mode: "time" },
           yaxis: { max: xRange },
           series: { points: {show: true}, lines: {show:true}, color: "rgba(135, 182, 217, 0.8)"},
           grid: { color: "rgba(135, 182, 217, 0.8)"}
         });
     }

     function loadGraph()
     {
       idTToday = [];
       lfTToday = [];
       lgTToday = [];
       hToday = [];

         $.get('/temp/today/index-noauth.php?daysold=0', function(d){
           $(d).find('nextrow').each(function(){
           var $record = $(this);
           var $timestamp = $record.find('timestamp').text() * 1000;
           var $data = $record.find('data').text();
           idTToday.push([$timestamp, $data]);
           });

           $.get('/temp2/today/index-noauth.php?daysold=0', function(e){
             $(e).find('nextrow').each(function(){
               var $record = $(this);
               var $timestamp = $record.find('timestamp').text() * 1000;
               var $data = $record.find('data').text();
               lfTToday.push([$timestamp, $data]);
             });

             $.get('/temp3/today/index-noauth.php?daysold=0', function(e){
                $(e).find('nextrow').each(function(){
                  var $record = $(this);
                  var $timestamp = $record.find('timestamp').text() * 1000;
                  var $data = $record.find('data').text();
                  lgTToday.push([$timestamp, $data]);
                });

                $.get('/temp4/today/index-noauth.php?daysold=0', function(e){
                  $(e).find('nextrow').each(function(){
                    var $record = $(this);
                    var $timestamp = $record.find('timestamp').text() * 1000;
                    var $data = $record.find('data').text();
                    hToday.push([$timestamp, $data]);
                  });
                  
                  plotGraph();
                });
             });
           });
         });
     }

  </script>

  <script language="javascript" type="text/javascript">
     loadGraph();
  </script>
</head>
<body>
  <!--  The DIV to hold the graph  -->
  <div id="thegraph" style="width:600px;height:350px;"></div>
</body>
</html>

