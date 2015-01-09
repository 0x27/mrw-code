<!DOCTYPE html>

 <html lang="en" style="height: auto; height: 250%; min-height: 150%">
 <head>

   <title>PPT Status</title>
   <meta http-equiv="Content-Type" content="text/html; charset=utf-8">

   <!--  Load the jquery javascript code  -->

   <link type="text/css" href="../../css/redmond/jquery-ui-1.7.2.custom.css" rel="stylesheet" />	
   <script src="../../js/jquery-1.10.2.min.js" type="text/javascript"></script>
   <script src="../../js/jquery-ui.min.js" type="text/javascript"></script>
   <script language="javascript" type="text/javascript" src="../../jquery/flot07/jquery.flot.js"></script>
 
   <!--  Include all of the JavaScript functions  -->

   <?php include 'functions.php'; ?>

   <!--  CSS styles for the HTML DIVs on the page  -->

   <style type="text/css">
    a.test { font-weight: bold; }
    img { padding: 5px; padding-top: 12px; }

#playlistsortable .ui-selecting { background: #FECA40; }
#playlistsortable .ui-selected { background: #F39814; color: white; }
#playlistsortable { list-style-type: none; margin: 0; padding: 0; width: 100%; }
#playlistsortable li { margin: 3px; padding: 0.4em; font-size: 1.0em; height: 14px; }

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
	<li><a href="#tabs-1">Current Users</a></li>
	<li><a href="#tabs-2">New Follow Requests</a></li>
	<li><a href="#tabs-3">Work Items</a></li>
     </ul>
			<div id="tabs-1" style="position: relative; height: 100%">
                           <p id="playlistsortable" style="position: relative; float: left">
                             <img width="30" src="../../ajax-loader.gif"/>
                           </p> 
                        </div>
			<div id="tabs-3" style="position: relative; height: 100%">
                           <ol id="workflow" style="position: relative; float: left">
                           <li><b>Show all users in admin console</b></li>
                           <li>Show a user's followers in admin console</li>
                           <li>Allow a user to authorise PPT app to manage their feed</li>
                           <li>Check a user's follow requests every 10 minutes</li>
                           <li>Show a user's follow requests in the admin console</li>
                           <li>Send a tweet to any new follow request people</li>
                           <li>Allow a Twitter user to make a payment to PPT</li>
                           <li>Allow a PPT user to select their payment options</li>
                           <li>Show who has paid to follow a PPT user, in the admin console</li>
                           <li>Show who is about to have their PPT payment expire, in the admin console</li>
                           <li>Send a Twitter user a message when their payment is about to expire</li>
                           </ol> 
                        </div>
			<div id="tabs-2">
                          <div><span>Energy Usage</span><span> - </span><span id="energyamountlabel">0</span><span> watts</span></div>
                          <div class="medium-progress-bar" id="progressbarenergy"></div>

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

                          <div><span>Last Few Days (kilowatt-hours used per day)</span></div>

                          <div id="averageelectricitygraph" style="width:600px;height:300px;">
                             <img width="30" src="ajax-loader.gif"/>
                          </div>

                          <div class="vertical-spacer"></div>

                          <div>
                             <span>Total Electricity Consumed This Year: </span>
                             <span id="annualwattsstat" style="font-weight:bold;"><img style="font-weight:bold;" width="30" src="ajax-loader.gif"/></span>
                             <span> kilowatt-hours</span>
                          </div>
                        </div>
                        </div>

  <div id="albumsdialog" title="Albums">
  </div>

 </body>
</html> 
