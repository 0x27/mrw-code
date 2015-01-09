<?php
$dbhost = 'localhost';
$dbuser = 'power';
$dbname = 'power';
$num = 100;
$scale = 10;#report 1 in n results
$num_ts = 10; # number of timestamps

$conn = mysql_connect($dbhost, $dbuser) or die ('Error connecting to mysql');
mysql_select_db($dbname);

#get number of rows
$query  = "SELECT COUNT(*) FROM reading";
$result = mysql_query($query);
$row = mysql_fetch_row($result);
$count = $row['0'];
if($num > $count){ $num = $count;}
$offset = $count - $num;

#get max & min values
$query  = "SELECT MAX(watts), MAX(temp), MIN(watts), MIN(temp) FROM reading ";
$result = mysql_query($query);
$row = mysql_fetch_row($result);
$max_watts = (int) $row['0'];
$max_temp = (int) $row['1'];
$min_watts = (int) $row['2'];
$min_temp = (int) $row['3'];


$query  = "SELECT timestamp, watts, temp FROM reading ORDER BY id ASC LIMIT $offset, $num";
$result = mysql_query($query);

  include("graPHPite/src/Graph.php");
    // create the graph
    $Graph =& new Image_Graph(400, 300);
    // create the plotareas
    $PlotArea =& $Graph->add(new Image_Graph_PlotArea());
    // create the dataset
//    $DataSet =& new Image_Graph_Dataset_Random(10, 2, 15, true);
		$Dataset =& new Image_Graph_Dataset_Trivial();
		$c=0;
		while($row = mysql_fetch_assoc($result))
		{
			$Dataset->addPoint($c,$row['watts']);
//		print	"Dataset->addPoint(".$c.",".$row['watts'].");<br>/";
			$c++;
		//	array_push($temp,$row['temp']);
		} 
    // create the 1st plot as smoothed area chart using the 1st dataset
    $Plot =& $PlotArea->addPlot(new Image_Graph_Plot_Line($DataSet));
    // set a line color
    $Plot->setLineColor(IMAGE_GRAPH_RED);
    // add a TrueType font
    $Arial =& $Graph->addFont(new Image_Graph_Font_TTF("arial.ttf"));
    // set the font size to 15 pixels
    $Arial->setSize(11);
    // add a title using the created font
    $Graph->add(new Image_Graph_Title("Simple Line Chart Sample", $Arial));
    // output the Graph
    $Graph->done();
?>
