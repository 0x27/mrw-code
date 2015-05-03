<script type="text/javascript">
 
     var lastCommentSubmitTime = 0;

     function handleCommentSubmit() {
       if (lastCommentSubmitTime == 0 || ((new Date().getTime() - lastCommentSubmitTime) > 20000)) {
         var theComment = document.getElementById('comment').value;
         $.get('/comments/index.php?comment=' + theComment, function(d){
           document.getElementById('comment').value = "";
           alert('Comment submitted');
         });
       } 

       lastCommentSubmitTime = new Date().getTime();
       return false;
     }
   
     setInterval ( "loadmusiclist()", 15000 );

     var indoorTempsToday = [];
     var loftTempsToday = [];
     var loungeTempsToday = [];
     var hallwayTempsToday = [];
     var gardenTempsToday = [];
 
     var sensorVoltages = [];

     var currentTemperature = 0;
     var maxTemperatureFromAllSensors = 0;

     // Specify 1 for yesterday's data, 2 for
     // the day before that, etc.
     var tempGraphDaysOffset = 0;

     function plotTempGraph() {

       if (true) {

         // Add a new data point to the end of which ever data set needs it, to make 
         // sure both graph plots have a data point at the same final timestamp
         if (loftTempsToday[loftTempsToday.length - 1][0] >
             indoorTempsToday[indoorTempsToday.length - 1][0]) {
         
             indoorTempsToday.push([loftTempsToday[loftTempsToday.length - 1][0], indoorTempsToday[indoorTempsToday.length - 1][1]]);
         } else {
             loftTempsToday.push([indoorTempsToday[indoorTempsToday.length - 1][0], loftTempsToday[loftTempsToday.length - 1][1]]);
         } 

         // Do the same for the beginning of the arrays, to make sure
         // line on the graph starts at the same timestamp.
         if (loftTempsToday[0][0] < indoorTempsToday[0][0]) {
             indoorTempsToday.unshift([loftTempsToday[0][0], indoorTempsToday[0][1]]);
         } else {
             loftTempsToday.unshift([indoorTempsToday[0][0], loftTempsToday[0][1]]);
         } 

         var i = 0;
         for (i = 0; i < indoorTempsToday.length; i++) {
            if (parseInt(indoorTempsToday[i][1]) > parseInt(currentTemperature)) { 
               currentTemperature = parseInt(indoorTempsToday[i][1]);
            }
            if (parseInt(indoorTempsToday[i][1]) > parseInt(maxTemperatureFromAllSensors)) { 
               maxTemperatureFromAllSensors = parseInt(indoorTempsToday[i][1]);
            }
         }
         for (i = 0; i < loftTempsToday.length; i++) {
            if (parseInt(loftTempsToday[i][1]) > parseInt(currentTemperature)) {
               currentTemperature = parseInt(loftTempsToday[i][1]);
            }
            if (parseInt(loftTempsToday[i][1]) > parseInt(maxTemperatureFromAllSensors)) {
               maxTemperatureFromAllSensors = parseInt(loftTempsToday[i][1]);
            }
         }
         for (i = 0; i < loungeTempsToday.length; i++) {
            if (parseInt(loungeTempsToday[i][1]) > parseInt(currentTemperature)) {
               currentTemperature = parseInt(loungeTempsToday[i][1]);
            }
            if (parseInt(loungeTempsToday[i][1]) > parseInt(maxTemperatureFromAllSensors)) {
               maxTemperatureFromAllSensors = parseInt(loungeTempsToday[i][1]);
            }
         }
         for (i = 0; i < hallwayTempsToday.length; i++) {
            if (parseInt(hallwayTempsToday[i][1]) > parseInt(currentTemperature)) {
               currentTemperature = parseInt(hallwayTempsToday[i][1]);
            }
            if (parseInt(hallwayTempsToday[i][1]) > parseInt(maxTemperatureFromAllSensors)) {
               maxTemperatureFromAllSensors = parseInt(hallwayTempsToday[i][1]);
            }
         }
         for (i = 0; i < gardenTempsToday.length; i++) {
            if (parseInt(gardenTempsToday[i][1]) > parseInt(currentTemperature)) {
               currentTemperature = parseInt(gardenTempsToday[i][1]);
            }
            if (parseInt(gardenTempsToday[i][1]) > parseInt(maxTemperatureFromAllSensors)) {
               maxTemperatureFromAllSensors = parseInt(gardenTempsToday[i][1]);
            }
         }

         // Add 5 degrees to the current maximum so the graph is scaled sensibly (and
         // more importantly make sure the labels don't overlap the data)
         var xRange = 5 + parseInt(maxTemperatureFromAllSensors);

         $.plot($("#temperaturegraph"), [
         { data: indoorTempsToday,
           points: {show: false}, 
           lines: {show: true},
           label: "Kitchen"},
         { data: loftTempsToday,
           points: {show: false}, 
           lines: {show: true}, 
           color: "rgba(110, 180, 30, 1.4)",
           label: "Loft"},
         { data: loungeTempsToday,
           points: {show: false}, 
           lines: {show: true}, 
           color: "rgba(195, 110, 50, 1.4)",
           label: "Lounge"},
         { data: hallwayTempsToday,
           points: {show: false}, 
           lines: {show: true}, 
           color: "rgba(215, 150, 70, 1.4)",
           label: "Hallway"},
         { data: gardenTempsToday,
           points: {show: false}, 
           lines: {show: true}, 
           color: "rgba(205, 60, 120, 1.4)",
           label: "Garden"}
         ], 
         {
           legend: { position: "ne", noColumns: 5, margin: 4 },
           xaxis: { mode: "time" },
           yaxis: { max: xRange },
           series: { points: {show: true}, lines: {show:true}, color: "rgba(135, 182, 217, 0.8)"},
           grid: { color: "rgba(135, 182, 217, 0.8)"} 
         });
       }
     }

     function loadTemperatureGraph() 
     {
       indoorTempsToday = [];
       loftTempsToday = [];
       loungeTempsToday = [];
       hallwayTempsToday = [];
       gardenTempsToday = [];

       // Only load the graphs once per load of the website
       //if (indoorTempsToday.length == 0 && 
       //    loftTempsToday.length == 0 &&
       //    loungeTempsToday.length == 0) {

         $.get('/temp/today?daysold=' + tempGraphDaysOffset, function(d){
           $(d).find('nextrow').each(function(){ 
           var $record = $(this);     
           var $timestamp = $record.find('timestamp').text() * 1000;
           var $temperature = $record.find('temperature').text();
           indoorTempsToday.push([$timestamp, $temperature]);
           });
         
           $.get('/temp2/today?daysold=' + tempGraphDaysOffset, function(e){
             $(e).find('nextrow').each(function(){ 
               var $record = $(this);     
               var $timestamp = $record.find('timestamp').text() * 1000;
               var $temperature = $record.find('temperature').text();
               loftTempsToday.push([$timestamp, $temperature]);
             });
            
             $.get('/temp3/today?daysold=' + tempGraphDaysOffset, function(e){
                $(e).find('nextrow').each(function(){ 
                  var $record = $(this);     
                  var $timestamp = $record.find('timestamp').text() * 1000;
                  var $temperature = $record.find('temperature').text();
                  loungeTempsToday.push([$timestamp, $temperature]);
                });

                $.get('/temp4/today?daysold=' + tempGraphDaysOffset, function(e){
                   $(e).find('nextrow').each(function(){ 
                      var $record = $(this);     
                      var $timestamp = $record.find('timestamp').text() * 1000;
                      var $temperature = $record.find('temperature').text();
                      hallwayTempsToday.push([$timestamp, $temperature]);
                   });
                
                   $.get('/temp5/today?daysold=' + tempGraphDaysOffset, function(e){
                      $(e).find('nextrow').each(function(){ 
                         var $record = $(this);     
                         var $timestamp = $record.find('timestamp').text() * 1000;
                         var $temperature = $record.find('temperature').text();
                         gardenTempsToday.push([$timestamp, $temperature]);
                      });

                      plotTempGraph();
                   });
                });
             });
           });
         });
       //}
     }

     function loadAverageTemperatureGraph() 
     {
       $.get('/temp/average?numberofdayshistory=120', function(d){
         var tempsToday = [];
         $(d).find('nextrow').each(function(){ 
         var $record = $(this);     
         var $timestamp = $record.find('timestamp_of_day').text() * 1000;
         var $temperature = $record.find('average_temperature').text();
         tempsToday.push([$timestamp, $temperature]);
         });

         $.plot($("#averagetemperaturegraph"), [ tempsToday ], {
         xaxis: {mode: "time"},
         series: { points: {show: false}, lines: {show:true}, color: "rgba(135, 182, 217, 0.8)"},
         grid: { color: "rgba(135, 182, 217, 0.8)"} 
         });

         loadHumidityGraph();
       });
     }

     function loadElectricityGraph(numOfHours) 
     {
       $.get('/power/today?numhoursfrommidnight=' + numOfHours, function(d){
          var powerToday = [];
          $(d).find('nextrow').each(function(){ 
             var $record = $(this);     
             var $timestamp = $record.find('timestamp').text() * 1000;
             var $watts = $record.find('watts').text();
             powerToday.push([$timestamp, $watts]);
           });
          $.plot($("#electricitygraph"), [ powerToday ], {
            xaxis: {mode: "time"},
            series: { points: {show: false}, lines: {show:true}, color: "rgba(135, 182, 217, 0.8)"},
            grid: { color: "rgba(135, 182, 217, 0.8)"} 
          });

          loadSolarGraph(-1);
          //setInterval ( "loadSolarGraph(2)", 120000 );
       });
     }

     function loadSolarGraph(numOfDays) 
     {
       $.get('/power/solar/Ndays?numberofdayshistory=' + numOfDays, function(d){
          var solarToday = [];
          $(d).find('nextrow').each(function(){ 
             var $record = $(this);     
             var $timestamp = $record.find('timestamp').text() * 1000;
             var $watts = $record.find('watts').text();
             solarToday.push([$timestamp, $watts]);
           });
          $.plot($("#solargraph"), [ solarToday ], {
            xaxis: {mode: "time"},
            series: { points: {show: false}, lines: {show:true}, color: "rgba(135, 182, 217, 0.8)"},
            grid: { color: "rgba(135, 182, 217, 0.8)", clickable: true}, 
            clickable: true,
            zoom: { interactive: true },
            pan: { interactive: true }
          });

          loadAnnualSolarGraph();
       });
     }

     function loadAnnualSolarGraph() 
     {
       $.get('/power/solar/history', function(d){
         var solarAnnual = [];
         $(d).find('nextrow').each(function(){
         var $record = $(this);
         var $timestamp = $record.find('next_timestamp').text() * 1000;
         var $watts = $record.find('next_solar_watt_hours').text();
         solarAnnual.push([$timestamp, $watts]);
         });

         $.plot($("#annualsolarwattsgraph"),
           [{
             data: solarAnnual,
             bars: {show: true},
             points: {show:false},
             lines: {show:false},
             clickable: true,
             zoom: {
               interactive: true
             },
             pan: {
               interactive: true
             }
            }],
            {
             xaxis: {mode: "time"},
             series: { points: {show: false}, lines: {show:true}, color: "rgba(40, 230, 30, 0.8)"},
             grid: { color: "rgba(135, 182, 217, 0.8)", clickable: true},
             clickable: true,
             zoom: { interactive: true },
             pan: { interactive: true }

            }
          );

          loadElectricityConsumptionStats();
       });
     }

     function loadVoltageGraph() 
     {
       $.get('/wirelessnodes/02/voltage/Ndays?numberofdayshistory=120', function(d){
          sensorVoltages[0] = [];
          $(d).find('nextrow').each(function(){ 
             var $record = $(this);     
             var $timestamp = $record.find('timestamp').text() * 1000;
             var $voltage = $record.find('voltage').text();
             sensorVoltages[0].push([$timestamp, $voltage]);
           });

           $.get('/wirelessnodes/03/voltage/Ndays?numberofdayshistory=120', function(d){
           sensorVoltages[1] = [];
           $(d).find('nextrow').each(function(){
             var $record = $(this);
             var $timestamp = $record.find('timestamp').text() * 1000;
             var $voltage = $record.find('voltage').text();
             sensorVoltages[1].push([$timestamp, $voltage]);
           });

           $.get('/wirelessnodes/04/voltage/Ndays?numberofdayshistory=120', function(d){
             sensorVoltages[2] = [];
             $(d).find('nextrow').each(function(){
               var $record = $(this);
               var $timestamp = $record.find('timestamp').text() * 1000;
               var $voltage = $record.find('voltage').text();
               sensorVoltages[2].push([$timestamp, $voltage]);
             });
             plotVoltageGraph();
           });
         });
       });
     }

     function plotVoltageGraph() {
       if (true) {
          $.plot($("#voltagegraph"), [ 
            { data: sensorVoltages[0],
              points: {show: false},
              lines: {show: true},
              label: "Loft "},
            { data: sensorVoltages[1],
              points: {show: false},
              lines: {show: true},
              label: "Lounge ",
              color: "rgba(195, 110, 50, 1.4)"},
            { data: sensorVoltages[2],
              points: {show: false},
              lines: {show: true},
              label: "Garden ",
              color: "rgba(110, 180, 30, 1.4)"}
            ],
          {
            legend: { position: "ne", noColumns: 3, margin: 7 },
            xaxis: { mode: "time" },
            yaxis: {min: 2.6, max: 4.6},
            series: { points: {show: false}, lines: {show:true}, color: "rgba(135, 182, 217, 0.8)"},
            grid: { color: "rgba(135, 182, 217, 0.8)"} 
          });

          loadSwapStateGraph();
       } 
     }

     function loadHumidityGraph() 
     {
       var humidityToday = [];

       $.get('/humidity3/today', function(d){

          humidityToday[0] = [];

          $(d).find('nextrow').each(function(){ 
             var $record = $(this);     
             var $timestamp = $record.find('timestamp').text() * 1000;
             var $humidity = $record.find('humidity').text();
             humidityToday[0].push([$timestamp, $humidity]);
          });

          $.get('/humidity2/today', function(d){
            humidityToday[1] = [];

            $(d).find('nextrow').each(function(){ 
              var $record = $(this);     
              var $timestamp = $record.find('timestamp').text() * 1000;
              var $humidity = $record.find('humidity').text();
              humidityToday[1].push([$timestamp, $humidity]);
            });
          
            $.get('/humidity4/today', function(d){
               humidityToday[2] = [];

               $(d).find('nextrow').each(function(){ 
                  var $record = $(this);     
                  var $timestamp = $record.find('timestamp').text() * 1000;
                  var $humidity = $record.find('humidity').text();
                  humidityToday[2].push([$timestamp, $humidity]);
               });

               $.get('/humidity5/today', function(d){
                  humidityToday[3] = [];

                  $(d).find('nextrow').each(function(){ 
                     var $record = $(this);     
                     var $timestamp = $record.find('timestamp').text() * 1000;
                     var $humidity = $record.find('humidity').text();
                     humidityToday[3].push([$timestamp, $humidity]);
                  });

               $.plot($("#humiditygraph"), [
                  { data: humidityToday[1],
                    points: {show: false}, 
                    lines: {show: true},
                    color: "rgba(110, 180, 30, 1.4)",
                    label: "Loft"},
                  { data: humidityToday[0],
                    points: {show: false}, 
                    lines: {show: true}, 
                    color: "rgba(195, 110, 50, 1.4)",
                    label: "Lounge"},
                  { data: humidityToday[2],
                    points: {show: false}, 
                    lines: {show: true}, 
                    color: "rgba(215, 150, 70, 1.4)",
                    label: "Hallway"},
                  { data: humidityToday[3],
                    points: {show: false}, 
                    lines: {show: true}, 
                    color: "rgba(205, 60, 120, 1.4)",
                    label: "Garden"}
               ], 
               {
                  legend: { position: "ne", noColumns: 4, margin: 4 },
                  xaxis: { mode: "time" },
                  yaxis: { max: 100 },
                  series: { points: {show: true}, lines: {show:true}, color: "rgba(135, 182, 217, 0.8)"},
                  grid: { color: "rgba(135, 182, 217, 0.8)"} 
               });

               loadTemperatureGraph();
               });
            });
         });
       });
     }

     function loadElectricityConsumptionStats() 
     {
       $.get('/power/annual', function(d){
          var $year;
          var $kilowatts;
          $(d).find('nextrow').each(function(){ 
             var $record = $(this);     
             $year = $record.find('year').text();
             $kilowatts = $record.find('total_watt_hours').text() / 1000;
           });
         //document.getElementById('annualwattsstat').innerHTML = $kilowatts;
         
           $.get('/power/solar/annual', function(d){
              var $year;
              var $solarkilowatts;
              $(d).find('nextrow').each(function(){ 
                 var $record = $(this);     
                 $year = $record.find('year').text();
                 $solarkilowatts = $record.find('total_solar_watt_hours').text() / 1000;
              });
              document.getElementById('annualsolarwattsstat').innerHTML = $solarkilowatts;
              document.getElementById('annualwattsstat').innerHTML = $kilowatts;
           });
       });
     }

     function loadAverageElectricityGraph()
     {
       $.get('/power/Ndays?numberofdayshistory=18', function(d){
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

         $.plot($("#averageelectricitygraph"), 
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

          loadElectricityGraph(24);
       });
     }

     function loadGasConsumptionStats() 
     {
       $.get('/gas/annual', function(d){
          var $year;
          var $gasmcubed;
          $(d).find('nextrow').each(function(){ 
             var $record = $(this);     
             $year = $record.find('year').text();
             $gasmcubed = $record.find('total_gasmcubed').text();
           });
         document.getElementById('annualgasstat').innerHTML = Math.round(($gasmcubed * 1.02264 * 40) / 3.6);
       });
     }

     function loadAverageGasGraph()
     {
       $.get('/gas/Ndays?numberofdayshistory=20', function(d){
         var gasToday = [];
         $(d).find('nextrow').each(function(){
         var $record = $(this);
         var $timestamp = $record.find('timestamp_of_day').text() * 1000;
         var $total_m3 = $record.find('total_gas_m3').text();

         gasToday.push([$timestamp, $total_m3]);
         });

         $.plot($("#averagegasgraph"),
           [{
             data: gasToday,
             bars: {show: true, fill: true, barWidth: 24*60*60*550}
            }],
            {
             xaxis: {mode: "time", tickSize:[1, "day"]},
             series: { color: "rgba(135, 182, 217, 0.8)"},
             grid: { color: "rgba(135, 182, 217, 0.8)"}
            }
          );

          loadGasConsumptionStats();
       });
     }

     function loadGasGraph() 
     {
       $.get('/gas/today', function(d){
          var gasToday = [];
          $(d).find('nextrow').each(function(){ 
             var $record = $(this);     
             var $timestamp = $record.find('hour').text() * 1000;
             var $watts = $record.find('total_ticks').text() * 0.01;
             gasToday.push([$timestamp, $watts]);
           });
          $.plot($("#gasgraph"), [ gasToday ], {
            xaxis: {mode: "time"},
            yaxis: {min: 0, max: 1.5},
            series: { bars: {show: true}, points: {show: true}, color: "rgba(135, 182, 217, 0.8)"},
            grid: { color: "rgba(135, 182, 217, 0.8)"} 
          });

          loadAverageGasGraph();
       });
     }

     function loadSoilMoistureGraph() 
     {
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
           $.plot($("#soilmoisturegraph"), [
             { data: moistureToday,
               points: {show: false}, 
               lines: {show: true}},
             { data: avgMoistureToday,
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

     function loadSwapStateGraph() 
     {
       $.get('/swap/history', function(d){
          var swapHistory = [];
          var i = 0;
          $(d).find('element').each(function(){ 
             var $swap = $(this).text();     
             swapHistory.push([i, $swap]);
             i++;
           });
          $.plot($("#swapgraph"), [ swapHistory ], {
            series: { lines: {show: true}, points: {show: false}, color: "rgba(135, 182, 217, 0.8)"},
            grid: { color: "rgba(135, 182, 217, 0.8)"} 
          });
       });
     }

     function addEntireAlbum(artist, album)
     {
        var address = '/music/add/?artistname=' + artist + '&albumname=' + album;
        
        $.get(address, function(d){play();});
        $("#albumsdialog").dialog('close');
     }
     
     function replaceEntireAlbum(artist, album)
     {
        var address = '/music/add/?artistname=' + artist + '&albumname=' + album;
        
        clear();        
        $.get(address, function(d){play();});
        $("#albumsdialog").dialog('close');
     }
     
     function crop()
     {
        $.get("music/crop/");
     }
     
     function clear()
     {
        $.get("music/clear/");
     }
     
     function flashStopButton()
     {
        $('#stopbutton').attr({'src': 'icons/music/ButtonStopSet.png'});
        setTimeout('resetButtons()', 600);
     }
     
     function flashBackButton()
     {
        $('#backbutton').attr({'src': 'icons/music/ButtonFirstSet.png'});
        setTimeout('resetButtons()', 600);
     }
     
     function flashForwardButton()
     {
        $('#forwardbutton').attr({'src': 'icons/music/ButtonLastSet.png'});
        setTimeout('resetButtons()', 600);
     }

     function resetButtons()
     {
        $('#backbutton').attr({'src': 'icons/music/ButtonFirst.png'});
        $('#forwardbutton').attr({'src': 'icons/music/ButtonLast.png'});
        $('#stopbutton').attr({'src': 'icons/music/ButtonStop.png'});
        $('#shufflebutton').attr({'src': 'icons/music/ButtonShuffle.png'});
     }
     
     function updateButtons()
     {
       if (paused)
       {
          $('#pausebutton').attr({'src': 'icons/music/ButtonPauseSet.png'});
          $('#playbutton').attr({'src': 'icons/music/ButtonPlay.png'});
       }
       else if (playing)
       {
          $('#pausebutton').attr({'src': 'icons/music/ButtonPause.png'});
          $('#playbutton').attr({'src': 'icons/music/ButtonPlaySet.png'});
       }
       else
       {
          $('#pausebutton').attr({'src': 'icons/music/ButtonPause.png'});
          $('#playbutton').attr({'src': 'icons/music/ButtonPlay.png'});
       }

       if (random)
       {
          $('#randombutton').attr({'src': 'icons/music/ButtonRandomSet.png'});
       }
       else
       {
          $('#randombutton').attr({'src': 'icons/music/ButtonRandom.png'});
       }
     }

     function togglerandom()
     {
        $.get("music/random/");
        loadmusiclist();
     }
   
     function forward()
     {
        $.get("music/forward/");
        loadmusiclist();
     }
     
     function back()
     {
        $.get("music/back/");
        loadmusiclist();
     }
     
     function play()
     {
        $.get("music/play/");
        loadmusiclist();
     }
     
     function stop()
     {
        $.get("music/stop/");
        loadmusiclist();
     }
     
     function mute()
     {
        $.get("music/mute/");
        loadmusiclist();
     }
     
     function unmute()
     {
        $.get("music/unmute/");
        loadmusiclist();
     }
     
     function pause()
     {
        $.get("music/pause/");
     }

     function startmpd()
     {
        $.get("mpd/start/");
     }

     function stopmpd()
     {
        $.get("mpd/stop/");
     }

     function resetaudiolevels()
     {
        $.get("apps/systemaudio/reset");
     }

     function enablerouterpowerdown()
     {
        $.get("power/control/enablerouterpowerdown");
     }

     function disablerouterpowerdown()
     {
        $.get("power/control/disablerouterpowerdown");
     }
     
     var currentlyPlayingTrack = "-1";
     
     var playing = false;
     var paused = false;
     var random = false;

     // Flags so we only load the graphs once (F5 
     // must be used to refresh the graphs)
     var electricityGraphsLoaded = false;
     var tempGraphsLoaded = false;
     var gasGraphsLoaded = false;
     var gardenGraphsLoaded = false;
     var voltageGraphsLoaded = false;
 
     $(document).ready(function(){

        // Tabs
        $('#tabs').tabs({
	  beforeActivate: function(event, ui) { 
            if ( $(ui.newTab).html().indexOf("Temp &amp; Humidity") != -1 && !tempGraphsLoaded ){ 
              loadAverageTemperatureGraph();
              tempGraphsLoaded = true;
            }
            else if ( $(ui.newTab).html().indexOf("Electricity") != -1 && !electricityGraphsLoaded ){ 
              loadAverageElectricityGraph();
              electricityGraphsLoaded = true;
            }
            else if ( $(ui.newTab).html().indexOf("Gas") != -1 && !gasGraphsLoaded ){ 
              loadGasGraph();
              gasGraphsLoaded = true;
            }
            //else if ( $(ui.newTab).html().indexOf("Garden") != -1 && !gardenGraphsLoaded ){ 
            //  loadSoilMoistureGraph();
             // gardenGraphsLoaded = true;
            //}
            else if ( $(ui.newTab).html().indexOf("Status") != -1 && !voltageGraphsLoaded ){ 
              loadVoltageGraph();
              voltageGraphsLoaded = true;
            }
          }
	});
        $('#tabs').tabs('option', 'active', 0);
        
        // Accordion
        $("#music").accordion({
                        collapsible: true,active: false
                });

        // Progressbar
        $("#progressbarenergy").progressbar({
           value: 0 
        });

        // Progressbar
        $("#progressbarsolar").progressbar({
           value: 0 
        });

        $("#electricityviewoptions").buttonset();
        $("#radio1").click(function() {
          $("#electricitygraph").html("Loading...");
          loadElectricityGraph(24);
        });
        $("#radio2").click(function() {
          $("#electricitygraph").html("Loading...");
          loadElectricityGraph(1);
        });
        $("#radio3").click(function() {
          $("#electricitygraph").html("Loading...");
          loadElectricityGraph(5);
        });
        $("#radio1").prop("checked", true);
        
        $("#progressbargas").progressbar({
           value: 0 
        });

        $("#tempviewoptions").buttonset();
        $("#radio4").click(function() {
          $("#temperaturegraph").html("Loading...");
          tempGraphDaysOffset++;
          loadTemperatureGraph();
        });
        $("#radio5").click(function() {
          if (tempGraphDaysOffset > 0) {
            $("#temperaturegraph").html("Loading...");
            tempGraphDaysOffset--;
            loadTemperatureGraph();
          }
        });

        // Progress bar for kitchen temperature
        $("#progressbartemp").progressbar({
           value: 0 
        });

        // Progress bar for loft temperature
        $("#progressbartemp2").progressbar({
           value: 0 
        });

        // Progress bar for lounge temperature
        $("#progressbartemp3").progressbar({
           value: 0 
        });

        // Progress bar for lounge temperature
        $("#progressbartemp4").progressbar({
           value: 0 
        });

        // Progress bar for humidity
        $("#progressbarhumidity").progressbar({
           value: 10 
        });

        // Progress bar for humidity
        $("#progressbarhumidityloft").progressbar({
           value: 0 
        });

        // Progress bar for humidity
        $("#progressbarhumidityhallway").progressbar({
           value: 0 
        });

        // Progressbar
        $("#load1progress").progressbar({
           value: 0
        });
        $("#load5progress").progressbar({
           value: 0
        });
        $("#load15progress").progressbar({
           value: 0
        });
        
            //$(".column").sortable({
                //      connectWith: '.column'
        //      });

                

        //      $(".column").disableSelection();      
        

        $("#playlistsortable").disableSelection();
        $("#playlistsortable").sortable({
                        cancel: '.ui-state-disabled'
                });
 
       $("#albumsdialog").dialog({
                        bgiframe: true,
                        autoOpen: false,
                        height: 500,
                        width: 500,
                        modal: true,
                        buttons: {
                                Cancel: function() {
                                        $(this).dialog('close');
                                }
                        },
                        close: function() {
                        }
                });
       $("a").addClass("test");
       $('#jlink').click(function(event){
         event.preventDefault();
         $(this).hide("slow");
       });
       
       $("#slider-vertical").slider({
                        orientation: "vertical",
                        range: "min",
                        min: 0,
                        max: 100,
                        value: 0,
                        slide: function(event, ui) {
                                $("#amount").val(ui.value);
                                $.get("music/volume/?volumeamount=" + ui.value);
                        }
                });
                $("#amount").val($("#slider-vertical").slider("value"));
                
                $("#timeslider").slider({
                        value: 0,
                        slide: function(event, ui) {
                                $("#timeamount").val(ui.value);
                                $.get("music/seek/?timepercent=" + ui.value);
                        }
                });
       
       loadmusiclist();
       
       loadartists();
     });

     var powerLoading = false;
     function loadpower()
     {
        if (!powerLoading) {
          powerLoading = true;
          // Change e.g. 0.28 kw into a percentage of 3000 watts
          $.get('/power', function(d, status){
            $('#progressbarenergy').progressbar('option', 'value', ((d * 1000) / 2000 * 100));
            $('#energyamountlabel').html(d * 1000);

            // Change the watts into a percentage of 200 watts
            $.get('/power/solar', function(d, status){
              if (status === 'success') {
                 $('#progressbarsolar').progressbar('option', 'value', ((d  / 200) * 100));
                 $('#solaramountlabel').html(d);
                 powerLoading = false;
                 loadhousedata();
              }
              powerLoading = false;
            }, "text");
          }, "text");
        }
     }

     var loadedTemperatures = false;
     function loadtemp()
     {
       if (!loadedTemperatures) {
          loadedTemperatures = true;
       
          $.get('/temp', function(d, status){
             if (status !== 'success') loadedTemperatures = false;
             $('#progressbartemp').progressbar('option', 'value', (d / 50 * 100));
             $('#tempamountlabel').html(d);
             if (d > currentTemperature)  {
                currentTemperature = d;
             }

             $.get('/temp2', function(d, status){
                if (status !== 'success') loadedTemperatures = false;
                $('#progressbartemp2').progressbar('option', 'value', (d / 50 * 100));
                $('#temp2amountlabel').html(d);

                $.get('/temp3', function(d, status){
                   if (status !== 'success') loadedTemperatures = false;
                   $('#progressbartemp3').progressbar('option', 'value', (d / 50 * 100));
                   $('#temp3amountlabel').html(d);

                   $.get('/temp4', function(d, status){
                      if (status !== 'success') loadedTemperatures = false;
                      $('#progressbartemp4').progressbar('option', 'value', (d / 50 * 100));
                      $('#temp4amountlabel').html(d);

                      loadhumidity();
                   }, "text");
                }, "text");
             }, "text");
          }, "text");
       }
     }

     function loadgas()
     {
        $.get('/gas', function(d){
          var gasOnAmount = 1;
          if (d == "On") {
             gasOnAmount = 100;
          }
          $('#progressbargas').progressbar('option', 'value', gasOnAmount);
          $('#gasamountlabel').html(d);

          loadswapdata();

       }, "text");
     }

     var loadedHumidity = false;
     function loadhumidity()
     {
        if (!loadedHumidity) {
           loadedHumidity = true;

           $.get('/humidity2', function(d, status){
              if (status !== 'success') loadedHumidity = false;
              var humidity = d;
              $('#progressbarhumidity').progressbar('option', 'value', parseInt(d));
              $('#humidityamountlabel').html(d);
        
              $.get('/humidity3', function(d, status){
                 if (status !== 'success') loadedHumidity = false;
                 var humidity = d;
                 $('#progressbarhumidityloft').progressbar('option', 'value', parseInt(d));
                 $('#humidityloftamountlabel').html(d);
          
                 $.get('/humidity4', function(d, status){
                    if (status !== 'success') loadedHumidity = false;
                    var humidity = d;
                    $('#progressbarhumidityhallway').progressbar('option', 'value', parseInt(d));
                    $('#humidityhallwayamountlabel').html(d);

                    loadsensorstatus();
                 }, "text");
              }, "text");
           }, "text");
        }
     }

     function loadsensorstatus()
     {
        $.get('/house/sensors/02/voltage', function(d){
          document.getElementById('node02voltage').innerHTML = (parseInt(d) / 10) + "v";
          
          $.get('/house/sensors/02/timestamp', function(d){
             document.getElementById('node02timestamp').innerHTML = d;

             $.get('/house/sensors/03/voltage', function(d){
                document.getElementById('node03voltage').innerHTML = (parseInt(d) / 10) + "v";

                $.get('/house/sensors/03/timestamp', function(d){
                   document.getElementById('node03timestamp').innerHTML = d;

                   $.get('/house/sensors/04/voltage', function(d){
                      document.getElementById('node04voltage').innerHTML = (parseInt(d) / 10) + "v";

                      $.get('/house/sensors/04/timestamp', function(d){
                         document.getElementById('node04timestamp').innerHTML = d;
                      }, "text");
                   }, "text");
                }, "text");
             }, "text");
          }, "text");
        }, "text");
     }
     
     function loadhousedata()
     {
        $.get('/house', function(d){
          //$('#statustest').html(d);
          var currentTagTokens = d.split( " " );

          // Uptime returns slightly different things depending on the time of day(!!!) so offset with this
          var offset = 0;
          if (currentTagTokens.length == 16)
          {
             offset = 1;
          } else if (currentTagTokens.length == 13)
          {
             offset = -2;
          }

          var load1=currentTagTokens[12 + offset].substring(0, currentTagTokens[12 + offset].length - 1);
          var load5=currentTagTokens[13 + offset].substring(0, currentTagTokens[13 + offset].length - 1);
          var load15=currentTagTokens[14 + offset];

          $('#statussystemtime').html('System time: ' + currentTagTokens[1]);
          $('#statusuptime').html('Uptime: ' + currentTagTokens[3] + ' ' + currentTagTokens[4].substring(0, currentTagTokens[4].length - 1));
          $('#statususers').html('Users: ' + currentTagTokens[7 + offset]);
          $('#load1progress').progressbar('option', 'value', (load1 / 5) * 100); 
          $('#load5progress').progressbar('option', 'value', (load5 / 5) * 100); 
          $('#load15progress').progressbar('option', 'value', (load15 / 5) * 100);
          $('#load1text').html(load1);
          $('#load5text').html(load5);
          $('#load15text').html(load15);

          loadgas();

       }, "text");
     }

     function loadswapdata()
     {
        $.get('/swap', function(d){
          $('#swapstate').html('Swap state: ' + d + 'k');
        }, "text");
     }
     
     function loadartists()
     {
        $.get('/music/artists', function(d){
          //$('#musicartists').html("");
          
          var i = 0;
          var artist = "";
          
          $(d).find('item').each(function(){  
   
                var $book = $(this);     
                artist = $book.find('artist').text();  

                var nextPortlet = '<div id="portlet" class="portlet">';
                nextPortlet = nextPortlet + '<div class="portlet-header" artistname="' + artist + '">' + artist + '</div>';
                nextPortlet = nextPortlet + '<div class="portlet-content">something about ' + artist + ' </div>';
                nextPortlet = nextPortlet + '</div>';
                $('#musicartistscolumn' + i).append($(nextPortlet));
                i = i + 1;      
                i = i % 4;      
           });
           
           $(".portlet").addClass("ui-widget ui-widget-content ui-helper-clearfix ui-corner-all");
                        $(".portlet-header")
                                .addClass("ui-widget-header ui-corner-all")
                                .prepend('<span class="ui-icon ui-icon-plusthick"></span>');
                $(".portlet-content").hide();

                    $(".portlet-header").click(function() {
                           $(this).toggleClass("ui-icon-minusthick");
                           //$(this).parents(".portlet:first").find(".portlet-content").toggle();
                           loadartistalbums($(this).attr("artistname"));
                   });
         if ($('#tempamountlabel').html().indexOf("0") == 0 ||
             $('#temp2amountlabel').html().indexOf("0") == 0 ||
             $('#temp3amountlabel').html().indexOf("0") == 0) {
            loadtemp();
         }
       });
     }
     
     function loadartistalbums(artist)
     {        
        var address = '/music/artists/?name=' + jQuery.trim(artist);
     
        $.get(address, function(d){
          
          var i = 0;
          var html = '<div>';
          
          $(d).find('item').each(function(){
                var $book = $(this);     
                var album = $book.find('album').text(); 
                album = jQuery.trim(album);
                artist = jQuery.trim(artist);
                html = html + '<div id="albumportlet" class="portlet">';
                html = html + '<div class="albumportlet-header" albumname="' + album + '">' + album + '</div>';
                //html = html + '<div class="albumportlet-content">';
                html = html + '<ol class="selectabletracks">';
                html = html + '<li class="ui-widget-content addalbum-button" albumname="' + album + '" artistname="' + artist + '">Add All</li>';
                html = html + '<li class="ui-widget-content replacealbum-button" albumname="' + album + '" artistname="' + artist + '">Replace All</li>';
                html = html + '</ol>';
                //html = html + '</div>';
                html = html + '</div>';
           });
           
           html = html + '</div>';
           
           $('#albumsdialog').html(html);
           $('#albumsdialog').attr({'title': 'Albums'});
           
           $(".albumportlet").addClass("ui-widget ui-widget-content ui-helper-clearfix ui-corner-all");
                        $(".albumportlet-header")
                                .addClass("ui-widget-header ui-corner-all")
                                .prepend('<span class="ui-icon ui-icon-plusthick"></span>');
                $(".selectabletracks").hide();
                
                $(".albumportlet-header").click(function() {
                           $(this).toggleClass("ui-icon-minusthick");
                           $(this).next().toggle();
                   });
                   
                   $(".addalbum-button").click(function() {
                      //alert("Adding album " + $(this).attr("albumname") + " by " + $(this).attr("artistname"));
                      addEntireAlbum($(this).attr("artistname"), $(this).attr("albumname"));
                   });
                   
                   $(".replacealbum-button").click(function() {
                      replaceEntireAlbum($(this).attr("artistname"), $(this).attr("albumname"));
                   });
                   
                   $(".selectabletracks").selectable();
                
           $('#albumsdialog').dialog('open');  
       });
       
       return;
     }
     
     function loadmusiclist(){ 
       currentlyPlayingTrack = 0;
       $.get('/music/playing', function(d){
       
          // Reset values - these will be overwritten when we get data back from the server
          //$('#playbutton').attr({'src': 'icons/music/ButtonPlay.png'});
          
          paused = false;
          playing = false;
          random = false;
          
          $(d).find('state').each(function(){
             
             if ($(this).text() == "playing")
             {
                playing = true;
                paused = false;
             }
             else if ($(this).text() == "paused")
             {
                playing = true;
                paused = true;
             }
          });
          
          $(d).find('position').each(function(){
             currentlyPlayingTrack = $(this).text();
             
             // Something is playing - therefore we can set the Play button to be set
             //$('#playbutton').attr({'src': 'icons/music/ButtonPlaySet.png'});
          });
          
          $(d).find('volume').each(function(){
             $("#amount").val($(this).text());
             $("#slider-vertical").slider('option', 'value', $(this).text());
          });
          
          $(d).find('timepercent').each(function(){
             $("#timeamount").val($(this).text());
             $("#timeslider").slider('option', 'value', $(this).text());
          });

          $(d).find('random').each(function(){
             if (jQuery.trim($(this).text()) == "on")
             {
                random = true;
             }
             else
             {
                random = false;
             }
          });

          $.get('/music', function(d){  
          
             $('#playlistsortable').html("");
   
             $(d).find('item').each(function(){  
   
                var $book = $(this);   
                var title = $book.find("track").text();  
                var artist = $book.find('artist').text();                
                var pos = $book.find('position').text();

                if (jQuery.trim(pos) == jQuery.trim(currentlyPlayingTrack))
                {
                   var nextli = '<li class="ui-state-highlight ui-state-disabled"><span class="ui-icon ui-icon-arrowthick-2-n-s"></span>' + pos + '. ' + artist + ' - ' + title + '</li>';
                   $('#playlistsortable').append($(nextli));
                }
                else
                {
                   var nextli = '<li class="ui-state-default"><span class="ui-icon ui-icon-arrowthick-2-n-s"></span>' + pos + '. ' + artist + ' - ' + title + '</li>';
                   $('#playlistsortable').append($(nextli));
                }
             });
          });
          
          updateButtons();
 
          loadpower();

          // Only does anything if one of the calls in loadtemp() failed and needs redoing
          loadtemp();

          // Only does anything if one of the calls in loadhumidity() failed and needs redoing
          loadhumidity();
       });
    }

</script>

