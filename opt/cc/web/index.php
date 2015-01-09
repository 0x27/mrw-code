<html><body>
<?php
$dbhost = 'localhost';
$dbuser = 'power';
$dbname = 'power';
$num = 10000;
$t_days = 7; #no of days to show temp for
$scale = 10;#report 1 in n results
$num_ts = 10; # number of timestamps

$conn = mysql_connect($dbhost, $dbuser) or die ('Error connecting to mysql');
mysql_select_db($dbname);

#Google simple encoding, uses the following letters for 0 to 61, 
#sets separated with comma, missing data with underscore
global $c;
$c = str_split("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789") ;

#get number of rows
$query  = "SELECT COUNT(*) FROM reading";
$result = mysql_query($query);
$row = mysql_fetch_row($result);
$count = $row['0'];
if($num > $count){ $num = $count;}
echo "<b>$count rows</b> - Showing last $num<br/>";

$offset = $count - $num;

#get max & min values
$query  = "SELECT MAX(watts), MIN(watts),FROM reading ";
$result = mysql_query($query);
$row = mysql_fetch_row($result);
$max_watts = (int) $row['0'];
$min_watts = (int) $row['1'];


$query  = "SELECT timestamp, watts, temp FROM reading ORDER BY id ASC LIMIT $offset, $num";
$result = mysql_query($query);

$watts = array();
$timestamp = array();
while($row = mysql_fetch_assoc($result))
{
	array_push($timestamp,$row['timestamp']);
	array_push($watts,$row['watts']);
	if($row['watts'] > $max_watts){
		$max_watts =$row['watts'];
	}
} 
echo "Watts: $max_watts > $min_watts";
$c_max = ceil($max_watts/1000)*1000;
$c_min = floor($min_watts/1000)*1000;

?>



	<h2>google chart</h2>
	<a href="http://code.google.com/apis/chart/">Google chart api</a><br>
	Todo:<ul>
		<li> average the readings - they are currently just sampled every <?php echo $scale?></li>
	</ul>
	<img src="http://chart.apis.google.com/chart?
	chs=1000x200
	&amp;chd=s:<?// chart data
			for($i=0;$i<sizeof($watts);$i++){
				if($i%$scale == 0){
						echo scale_value( $c_max, $c_min, $watts[$i]);
				}
			}
	?>
<?php//Char Type  ?>&amp;cht=lc
<?php//Axes       ?>&amp;chxt=x,y
<?php//Grid Lines ?>&amp;chg=<?php echo (int) (100/($num_ts-1)/2) #should be a fraction of 100, e.g. split into two sections, one line '50'?>,10,1,5
<?php//Label range?>&amp;chxr=1,<?php echo $c_min?>,<?php echo $c_max?><?php // label range ?> 
<?php//Labels     ?>&amp;chxl=0:|<?// timestamps
			for( $i=0; $i < $num_ts-1; $i++ ){
				$p = sizeof($timestamp) /($num_ts-1) *$i;
				$d = explode(" ",$timestamp[$p]);
				$d = str_replace(":",".",$d[1]);
				echo $d."|";
			}
			$d = explode(" ",$timestamp[sizeof($timestamp)-1]);
			$d = str_replace(":",".",$d[1]);
			echo $d."|";
	?>1:<?php //y axis
		echo "|$c_min";
		for( $i=1; $i < 10; $i++ ){
			$y= (int) (($c_max-$c_min)/10 * $i)+$c_min;
			echo "|$y";
		}
		echo "|$c_max";
		?>
	" alt="Power chart" />







	<h2>Averaged google chart</h2>
<!-- TODO: update this to use similar code to first google chart-->
	<img src="http://chart.apis.google.com/chart?
	chs=1000x200
	&amp;chd=s:<?// chart data
			$total=0;
			$count=1;
			for($i=0;$i<sizeof($watts);$i++){
				if($i%$scale == 0){
						$total=$total/$count;
						echo scale_value( $c_max, $c_min, $watts[$i]);
						$total=0;
						$count=1;
				}
				$total+=$watts[$i];
				$count++;
			}
	?>
<?php//Char Type  ?>&amp;cht=lc
<?php//Axes       ?>&amp;chxt=x,y
<?php//Grid Lines ?>&amp;chg=<?php echo (int) (100/($num_ts-1)/2) #should be a fraction of 100, e.g. split into two sections, one line '50'?>,10,1,5
<?php//Label range?>&amp;chxr=1,<?php echo $c_min?>,<?php echo $c_max?><?php // label range ?> 
<?php//Labels     ?>&amp;chxl=0:|<?// timestamps
			for( $i=0; $i < $num_ts-1; $i++ ){
				$p = sizeof($timestamp) /($num_ts-1) *$i;
				$d = explode(" ",$timestamp[$p]);
				$d = str_replace(":",".",$d[1]);
				echo $d."|";
			}
			$d = explode(" ",$timestamp[sizeof($timestamp)-1]);
			$d = str_replace(":",".",$d[1]);
			echo $d."|";
	?>1:<?php //y axis
		echo "|$c_min";
		for( $i=1; $i < 10; $i++ ){
			$y= (int) (($c_max-$c_min)/10 * $i)+$c_min;
			echo "|$y";
		}
		echo "|$c_max";
		?>
	" alt="Power chart" />



	<h2>Averaged tempertature chart</h2>
<?php
$t_days = $t_days*24;
print "Last ".$t_days." readings = Last week if all readings are in<br/>";
#get temperature values, rouding to the nearest hour
$query = "SELECT CONCAT(DATE(r.timestamp),' ',HOUR(r.timestamp),':00') as datetime, ROUND(AVG(temp)) as avg_temp FROM reading r GROUP BY datetime ORDER BY r.timestamp DESC LIMIT $t_days;";
$result = mysql_query($query);

$t_timestamp = array();
$temp = array();
while($row = mysql_fetch_assoc($result))
{
	array_push($t_timestamp,$row['datetime']);
	array_push($temp,$row['avg_temp']);
} 
$t_timestamp = array_reverse($t_timestamp);
$temp = array_reverse($temp);
$max_temp = max($temp);
$min_temp = min($temp);
print "max: $max_temp, min: $min_temp<br/>";
?>
<!-- TODO: update this to use similar code to first google chart-->
	<img src="http://chart.apis.google.com/chart?
	chs=1000x200
	&amp;chd=s:<?// chart data
			$total=0;
			$count=1;
			for($i=0;$i<sizeof($temp);$i++){
				if($i%$scale == 0){
						$total=$total/$count;
						echo scale_value( $max_temp, $min_temp, $temp[$i]);
						$total=0;
						$count=1;
				}
				$total+=$temp[$i];
				$count++;
			}
	?>
<?php//Char Type  ?>&amp;cht=lc
<?php//Axes       ?>&amp;chxt=x,y
<?php//Grid Lines ?>&amp;chg=<?php echo (int) (100/($num_ts-1)/2) #should be a fraction of 100, e.g. split into two sections, one line '50'?>,10,1,5
<?php//Label range?>&amp;chxr=1,<?php echo $min_temp?>,<?php echo $max_temp?><?php // label range ?> 
<?php//Labels    ?>&amp;chxl=0:|<?// timestamps
			for( $i=0; $i < $num_ts-1; $i++ ){
				$p = sizeof($t_timestamp) /($num_ts-1) *$i;
				$d = explode(" ",$t_timestamp[$p]);
				$t = explode("-",$d[0]);
				$d = $d[1]." ".$t[2]."-".$t[1];
//				$d = str_replace(":",".",$d[1]);
				echo $d."|";
			}
			  $d = explode(" ",$t_timestamp[sizeof($t_timestamp)-1]);
				$t = explode("-",$d[0]);
				$d = $d[1]." ".$t[2]."-".$t[1];
//			$d = str_replace(":",".",$d[1]);
			echo $d."|";
	?>1:<?php //y axis
		echo "|$min_temp";
		for( $i=1; $i < 10; $i++ ){
			$y= (int) (($max_temp-$min_temp)/10 * $i)+$min_temp;
			echo "|$y";
		}
		echo "|$max_temp";
		?>
	" alt="Power chart" />



<h2>graPHPite</h2>
<a href="http://sourceforge.net/projects/graphpite/">sourceforge graPHPite page</a>
<br/>
Sample<br/>
<img src="graPHPite.php"/>





<h2>phplot</h2>
<a href="http://phplot.sourceforge.net/">phplot</a>
<img src="phplot.php"/>
<?php // recalculate max/min watts for detailed graph
	$max_watts=0;
	$min_watts=10000;
	for($i=sizeof($watts)-100;$i<sizeof($watts);$i++){
		if($watts[$i] > $max_watts){
			$max_watts = $watts[$i];
		}
		if($watts[$i] < $min_watts){
			$min_watts = $watts[$i];
		}
	}
$c_max = ceil($max_watts/100)*100;
$c_min = floor($min_watts/100)*100;
?>


<h2>php-gnuplot</h2>

	<h2>google chart last 30 min</h2>
<!-- TODO: update this to use similar code to first google chart-->
<?php echo "Watts: $max_watts > $min_watts" ?><br/>
	<img src="http://chart.apis.google.com/chart?
	chs=800x200
	&amp;chd=s:<?// chart data
			for($i=sizeof($watts)-300;$i<sizeof($watts);$i++){
				echo scale_value($c_max, $c_min, $watts[$i]);
//				echo $watts[$i].",";
			}
	?>

<?php//Char Type  ?>&amp;cht=lc
<?php//Axes       ?>&amp;chxt=x,r,x,r
<?php//Grid Lines ?>&amp;chg=<?php echo (int) (100/($num_ts-1)/2) #should be a fraction of 100, e.g. split into two sections, one line '50'?>,10,1,5
<?php//Label range?>&amp;chxr=1,<?php echo $c_min?>,<?php echo $c_max?><?php // label range ?> 
<?php//Labels     ?>&amp;chxl=0:|<?// timestamps
			for( $i=0; $i < $num_ts-1; $i++ ){
        $p = (int) 300/($num_ts-1) *$i + (sizeof($timestamp)-300);
				$d = explode(" ",$timestamp[$p]);
				$d = str_replace(":",".",$d[1]);
				echo $d."|";
			}
			$d = explode(" ",$timestamp[sizeof($timestamp)-1]);
			$d = str_replace(":",".",$d[1]);
			echo $d."|";
	?>1:<?php //y axis
		echo "|$c_min";
		for( $i=1; $i < 10; $i++ ){
			$y= (int) (($c_max-$c_min)/10 * $i)+$c_min;
			echo "|$y";
		}
		echo "|$c_max";
		?>|2:||time||3:||watts|
	" alt="Power chart" />

<?php
echo "<h2>Raw Data (last 130)</h2>";
for($i=sizeof($watts)-300;$i<sizeof($watts);$i++){
//for($i=0;$i<sizeof($timestamp);$i++){
    echo "$timestamp[$i]} watts : $watts[$i] temp : $temp[$i] <br>";
}

mysql_close($conn);

//print "-".scale_value(1000,450,500)."<br>";
//print "-".scale_value(200,10,150)."<br>";

?>
<br>
Page loaded in
<?php $load = microtime();
print (number_format($load,2));
?>sec
</body></html>



<?
function scale_value ($max, $min, $value){
	global $c;
//	$c = str_split("0123456789");
	if($max==0){
		return "_";
	}
	$i = (int) ( ($value - $min ) / ($max - $min) * sizeof($c) );
	return $c[$i];
}
