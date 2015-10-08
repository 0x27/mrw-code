<?php header('Content-type: text/plain'); ?>
<?php
$artistname = escapeshellarg($_GET['artistname']);
$albumname = escapeshellarg($_GET['albumname']);

if (strlen($artistname) > 0)
{
   if (strlen($albumname) > 0)
   {
      shell_exec('mpc add "'.$artistname.'/'.$albumname.'"');
   }
   else
   {
      shell_exec('mpc add "'.$artistname.'"');      
   }
}
?>

