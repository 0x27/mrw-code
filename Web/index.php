<!DOCTYPE html>

 <html lang="en" style="height: auto; height: 250%; min-height: 150%">
 <head>

   <title>House Status</title>
   <meta http-equiv="Content-Type" content="text/html; charset=utf-8">

   <!--  Load the jquery javascript code  -->

   <link type="text/css" href="css/redmond/jquery-ui-1.7.2.custom.css" rel="stylesheet" />	
   <!--<script src="js/jquery-1.3.2.min.js" type="text/javascript"></script>-->
   <script src="js/jquery-1.10.2.min.js" type="text/javascript"></script>
   <!--<script src="js/jquery-ui-1.7.2.custom.min.js" type="text/javascript"></script>-->
   <script src="js/jquery-ui.min.js" type="text/javascript"></script>
   <!--<script language="javascript" type="text/javascript" src="jquery/flot/jquery.js"></script>-->
   <script language="javascript" type="text/javascript" src="jquery/flot07/jquery.flot.js"></script>
 
   <!--  Include all of the JavaScript functions  -->

   <?php include 'functions.php'; ?>

   <!--  CSS styles for the HTML DIVs on the page  -->

   <style type="text/css">
    a.test { font-weight: bold; }
    img { padding: 5px; padding-top: 12px; }
    #playlistsortable { list-style-type: none; margin: 0; padding: 0; width: 40%; }
    #playlistsortable li { margin: 0 3px 3px 3px; padding: 0.4em; padding-left: 1.5em; font-size: 0.8em; height: 16px; }
    #playlistsortable li span { position: absolute; margin-left: -1.3em; }
    #musicartists { list-style-type: none; margin: 0; padding: 15px; height: 600%}
    #musicartists li { margin: 0 3px 3px 3px; padding: 0.4em; padding-left: 1.5em; font-size: 0.8em; height: 16px; }
    #musicartists li span { position: absolute; margin-left: -1.3em; }
    div{ font-size: 1.0em; margin: 10px 0px 0px 0px }
    div.thin-progress-bar { height: 16px; width: 50%; padding: 0em; margin-left: 10px;}
    div.medium-progress-bar { height: 20px; width: 630px; padding: 0em; margin-left: 0px;}
    div.vertical-spacer { height: 30px; }
    div.vertical-spacer-medium { height: 15px; }
    #playcontrols {height: 150px; width: 150px; border: 1} 
    #playcontrols h3 { text-align: center; margin: 0; }
    #musicbrowser {height: 300px; width: 150px; border: 1} 
    #musicbrowser h3 { text-align: center; margin: 0; }
    .column { width: 170px; float: left; padding-bottom: 100px; }
	.portlet { margin: 0 1em 1em 0; }
	.portlet-header { margin: 0.3em; padding-bottom: 4px; padding-left: 0.2em; }
	.portlet-header .ui-icon { float: right; }
	.portlet-content { padding: 0.4em; }
	#albumsdialog {height: 75%; margin: 0 1em 1em 0; }
	.selectabletracks .ui-selecting { background: #FECA40; }
	.selectabletracks .ui-selected { background: #F39814; color: white; }
	.selectabletracks { list-style-type: none; margin: 0px; padding: 0; width: 30%; text-align: center; }
	.selectabletracks li { margin: 3px 3px 3px 3px; padding: 0.4em; font-size: 0.8em; height: 16px; }
        .albumoptionslist li { margin: 3px 3px 3px 3px; padding: 0.4em; }
   .ui-buttonset .ui-button {    margin-left: 15;    margin-right: 15px;}



   </style>

 </head>

 <!--  HTML Body  -->

 <body style="height: auto; height: 100%; min-height: 100%">

   <div id="tabs" style="height: 100%">
     <ul>
	<li><a href="#tabs-1">Music</a></li>
	<li><a href="#tabs-2">Electricity</a></li>
        <li><a href="#tabs-5">Gas</a></li>
        <li><a href="#tabs-3">Temp & Humidity</a></li>
        <li><a href="#tabs-6">Garden</a></li>
        <li><a href="#tabs-4">Status</a></li>
     </ul>
			<div id="tabs-1" style="position: relative; height: 100%">
                           <ul id="playlistsortable" style="position: relative; float: left">
                             <img width="30" src="ajax-loader.gif"/>
                           </ul> 
                           <div id="playcontrols" class="ui-widget-content" style="position: relative ; float: right; width: 58%">
                              <h3 class="ui-widget-header">Play Controls</h3>
                              <a href="#" onclick="javascript:play();return false;"><img id="playbutton" src="icons/music/ButtonPlay.png" width="50" border="0" style="padding-left: 12px;"/></a>
                              <a href="#" onclick="javascript:stop();return false;"><img id="stopbutton" onclick="javascript:flashStopButton();return false;" src="icons/music/ButtonStop.png" width="50" border="0"/></a>
                              <a href="#" onclick="javascript:pause();return false;"><img id="pausebutton" src="icons/music/ButtonPause.png" width="50" border="0"/></a>
                              <a href="#" onclick="javascript:back();return false;"><img id="backbutton" onclick="javascript:flashBackButton();return false;" src="icons/music/ButtonFirst.png" width="50" border="0"/></a>
                              <a href="#" onclick="javascript:forward();return false;"><img id="forwardbutton" onclick="javascript:flashForwardButton();return false;" src="icons/music/ButtonLast.png" width="50" border="0"/></a>
                              <a href="#" onclick="javascript:togglerandom();return false;"><img id="randombutton" src="icons/music/ButtonRandom.png" width="50" border="0" style="padding-left: 30px;"/></a>
                              <!--<a href="#" onclick="javascript:mute();return false;"><img src="icons/music/ButtonCancel.png" width="50" border="0"/></a>
                              <a href="#" onclick="javascript:unmute();return false;"><img src="icons/music/ButtonAdd.png" width="50" border="0"/></a>-->
                              
                                <input type="hidden" id="amount" style="border:0; color:#f6931f; font-weight:bold" />

                                <div id="slider-vertical" style="height:90px; position: relative; float: right; left: -25px; "></div>
                                
                                <input type="hidden" id="timeamount" style="border:0; color:#f6931f; font-weight:bold" />
                                
                                <div id="timeslider" style="width:275px; left: 35px; top: 5px;"></div>

                           </div>
                           <div id="musicbrowser" class="ui-widget-content" style="position: relative; float: right; width: 58%; height: auto">
                              <h3 class="ui-widget-header">Music Collection</h3>
                              <div id="music" style="height: auto">
                                 <h3><a href="#">Artists</a></h3>
                                 <div id="musicartists">
                                    <div id="musicartistscolumn0" class="column">
                                    </div>
                                    
                                    <div id="musicartistscolumn1" class="column">
                                    </div>
                                    
                                    <div id="musicartistscolumn2" class="column">
                                    </div>
                                    
                                    <div id="musicartistscolumn3" class="column">
                                    </div>

                                 </div>
                                 <!--<h3><a href="#">Albums</a></h3>
                                 <div id="musicalbums">hi</div>-->
                              </div>
                           </div>
                        </div>
			<div id="tabs-2">
                          <div><span>Electricity Usage</span><span> - </span><span id="energyamountlabel">0</span><span> watts</span></div>
                          <div class="medium-progress-bar" id="progressbarenergy"></div>

                          <div class="vertical-spacer-medium"></div>

                          <div><span>Solar Generation</span><span> - </span><span id="solaramountlabel">0</span><span> watts</span></div>
                          <div class="medium-progress-bar" id="progressbarsolar"></div>

                          <div class="vertical-spacer"></div>

                          <div><span>Electricity Consumption Today (watts)</span></div>

                          <div id="electricitygraph" style="width:600px;height:300px;">
                             <img width="30" src="ajax-loader.gif"/>
                          </div>

                          <form>
                            <div id="electricityviewoptions">
                              <input type="radio" id="radio1" name="radio" checked="checked"><label for="radio1">All Day</label>
                              <input type="radio" id="radio2" name="radio"><label for="radio2">Midnight to 2am</label>
                              <input type="radio" id="radio3" name="radio"><label for="radio3">Midnight to 6am</label>
                            </div>
                          </form>

                          <div class="vertical-spacer"></div>

                          <div><span>Solar Generation (watts)</span></div>
                          
                          <div id="solargraph" style="width:600px;height:300px;">
                             <img width="30" src="ajax-loader.gif"/>
                          </div>

                          <div class="vertical-spacer"></div>

                          <div><span>Last Few Days (kilowatt-hours used per day)</span></div>

                          <div id="averageelectricitygraph" style="width:600px;height:300px;">
                             <img width="30" src="ajax-loader.gif"/>
                          </div>

                          <div>
                             <span>Total electricity consumed this year: </span>
                             <span id="annualwattsstat" style="font-weight:bold;"><img style="font-weight:bold;" width="30" src="ajax-loader.gif"/></span>
                             <span> kilowatt-hours</span>
                          </div>

                          <div class="vertical-spacer"></div>

                          <div><span>Solar Generation This Year (watt-hours)</span></div>

                          <div id="annualsolarwattsgraph" style="width:600px;height:300px;">
                             <img width="30" src="ajax-loader.gif"/>
                          </div>

                          <div>
                             <span>Total solar power produced this year: </span>
                             <span id="annualsolarwattsstat" style="font-weight:bold;"><img style="font-weight:bold;" width="30" src="ajax-loader.gif"/></span>
                             <span> kilowatt-hours</span>
                          </div>
                        </div>
                        <div id="tabs-3">
                          <div><span>Loft Temperature</span><span> - </span><span id="temp2amountlabel">0</span><span> degrees</span></div>
                          <div class="medium-progress-bar" id="progressbartemp2"></div>

                          <div class="vertical-spacer-medium"></div>

                          <div><span>Kitchen Temperature</span><span> - </span><span id="tempamountlabel">0</span><span> degrees</span></div>
                          <div class="medium-progress-bar" id="progressbartemp"></div>

                          <div class="vertical-spacer-medium"></div>

                          <div><span>Lounge Temperature</span><span> - </span><span id="temp3amountlabel">0</span><span> degrees</span></div>
                          <div class="medium-progress-bar" id="progressbartemp3"></div>

                          <div class="vertical-spacer"></div>

                          <div><span>Hallway Temperature</span><span> - </span><span id="temp4amountlabel">0</span><span> degrees</span></div>
                          <div class="medium-progress-bar" id="progressbartemp4"></div>

                          <div class="vertical-spacer"></div>

                          <div><span>Temperature Today</span></div>

                          <div id="temperaturegraph" style="width:600px;height:300px;">
                             <img width="30" src="ajax-loader.gif"/>
                          </div>

                          <form>
                            <div id="tempviewoptions">
                              <input type="radio" id="radio4" name="radio" checked="checked"><label for="radio4">Previous Day</label>
                              <input type="radio" id="radio5" name="radio"><label for="radio5">Next Day</label>
                            </div>
                          </form>

                          <div class="vertical-spacer"></div>

                          <div><span>Last Few Months</span></div>

                          <div id="averagetemperaturegraph" style="width:600px;height:300px;">
                             <img width="30" src="ajax-loader.gif"/>
                          </div>

                          <div class="vertical-spacer"></div>

                          <div><span>Lounge Humidity</span><span> - </span><span id="humidityamountlabel">0</span><span> %</span></div>
                          <div class="medium-progress-bar" id="progressbarhumidity" name="progressbarhumidity"></div>
                          <div class="vertical-spacer"></div>

                          <div><span>Loft Humidity</span><span> - </span><span id="humidityloftamountlabel">0</span><span> %</span></div>
                          <div class="medium-progress-bar" id="progressbarhumidityloft" name="progressbarhumidityloft"></div>
                          <div class="vertical-spacer"></div>

                          <div><span>Hallway Humidity</span><span> - </span><span id="humidityhallwayamountlabel">0</span><span> %</span></div>
                          <div class="medium-progress-bar" id="progressbarhumidityhallway" name="progressbarhumidityhallway"></div>
                          <div class="vertical-spacer"></div>


                          <div><span>Humidity Today</span></div>

                          <div id="humiditygraph" style="width:600px;height:300px;">
                             <img width="30" src="ajax-loader.gif"/>
                          </div>

                          <div class="vertical-spacer"></div>

                        </div>
                        <div id="tabs-6">
                          <div><span>Soil Water Content</span></div>

                          <div class="vertical-spacer-medium"></div>

                          <div id="soilmoisturegraph" style="width:600px;height:300px;">
                             <img width="30" src="ajax-loader.gif"/>
                          </div>
                        </div>
                        <div id="tabs-4">
                           <div id="statussystemtime" name="statussystemtime">System time: </div>
                           <div id="statusuptime" name="statusuptime">Uptime: </div>
                           <div id="statususers" name="statususers">Users: </div>
                           <div id="swapstate" name="swapstate">Swap state: </div>
                           <div id="statusload1" name="statusload1">Load (1 minute): <span id="load1text"></span> 
                              <div class="thin-progress-bar" id="load1progress" name="load1progress"></div>
                           </div>
                           <div id="statusload5" name="statusload5">Load (5 minute): <span id="load5text"></span> 
                              <div class="thin-progress-bar" id="load5progress" name="load5progress"></div>
                           </div>
                           <div id="statusload15" name="statusload15">Load (15 minute): <span id="load15text"></span> 
                              <div class="thin-progress-bar" id="load15progress" name="load15progress"></div>
                           </div>
                           <div class="vertical-spacer"></div>
                           <div><span>Swap Usage (kilobytes)</span></div>

                          <div id="swapgraph" style="width:600px;height:300px;">
                             <img width="30" src="ajax-loader.gif"/>
                          </div>
                           <div class="vertical-spacer"></div>
                           <div><span>Server Controls</span></div>
                           <div>
                              <ul>
	                         <li style="font-size: 0.8em;"><a href="#" onclick="javascript:startmpd();return false;">Start Server</a></li>
	                         <li style="font-size: 0.8em;"><a href="#" onclick="javascript:stopmpd();return false;">Stop Server</a></li>
	                         <li style="font-size: 0.8em;"><a href="#" onclick="javascript:resetaudiolevels();return false;">Reset Audio Levels</a></li>
	                         <li style="font-size: 0.8em;"><a href="#" onclick="javascript:enablerouterpowerdown();return false;">Enable Router Power Down</a></li>
	                         <li style="font-size: 0.8em;"><a href="#" onclick="javascript:disablerouterpowerdown();return false;">Disable Router Power Down</a></li>
                              </ul>
                           </div>
                           <div class="vertical-spacer"></div>
                           <div><span>Wireless Sensors</span></div>
                           <div>
                              <table name="sensorstable" border="1">
                                 <tr><td>Sensor</td><td>Voltage</td><td>Last Reading</td></tr>
                                 <tr><td>Loft</td><td id="node02voltage"><img width="20" src="ajax-loader.gif"/></td><td id="node02timestamp"><img width="20" src="ajax-loader.gif"/></td></tr>
                                 <tr><td>Lounge</td><td id="node03voltage"><img width="20" src="ajax-loader.gif"/></td><td id="node03timestamp"><img width="20" src="ajax-loader.gif"/></td></tr>
                                 <tr><td>Garden</td><td id="node04voltage"><img width="20" src="ajax-loader.gif"/></td><td id="node04timestamp"><img width="20" src="ajax-loader.gif"/></td></tr>
                              </table>
                           </div>

                           <div class="vertical-spacer"></div>
                           <div><span>Wireless Sensor Voltage</span></div>

                           <div id="voltagegraph" style="width:600px;height:300px;">
                             <img width="30" src="ajax-loader.gif"/>
                           </div>

                           <div id="commentarea">
                             <label for="comment">Data label</label>
                             <input type="text" name="comment" id="comment" width="200" size="60"/>
                             <button type="button" onClick="JavaScript:return handleCommentSubmit();">Submit</button>
                           </div>

                        </div>
			<div id="tabs-5">
                          <div><span>Gas: </span><span style="font-weight:bold" id="gasamountlabel">off</span></div>
                          <div class="medium-progress-bar" id="progressbargas"></div>

                          <div class="vertical-spacer"></div>

                          <div><span>Gas Consumption Today (metres-cubed per hour)</span></div>

                          <div id="gasgraph" style="width:600px;height:300px;">
                             <img width="30" src="ajax-loader.gif"/>
                          </div>

                          <div class="vertical-spacer"></div>

                          <div><span>Last Few Days (total metres-cubed used per day)</span></div>

                          <div id="averagegasgraph" style="width:600px;height:300px;">
                             <img width="30" src="ajax-loader.gif"/>
                          </div> 

                          <div class="vertical-spacer"></div>

                          <div>
                             <span>Total Gas Consumed This Year: </span>
                             <span id="annualgasstat" style="font-weight:bold;"><img style="font-weight:bold;" width="30" src="ajax-loader.gif"/></span>
                             <span> kilowatt-hours</span>
                          </div>
		</div>

  <div id="albumsdialog" title="Albums">
  </div>

 </body>
</html> 
