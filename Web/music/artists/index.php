<?php header('Content-type: application/xml'); ?>
<?php echo("<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n"); ?>
<result>
<?php
$artistname = $_GET['name'];

if (strlen($artistname) == 0)
{
   $music=shell_exec('mpc ls');

   $parts=explode("\n", $music);
   $size=count($parts);
   $i=0;

   while ($i < $size-1)
   {
   ?>
   <item>
   <artist>
   <?php
      echo trim(htmlspecialchars($parts[$i]));
   ?>
   </artist>
   </item>
   <?php
   $i++;
   }
}
else
{

  #$albums=shell_exec('mpc ls "'.$artistname.'"'); # Old vulnerable code
  $albums=shell_exec('mpc ls "'.escapeshellarg($artistname)'"');

  $parts=explode("\n", $albums);
   $size=count($parts);
   $i=0;

   while ($i < $size-1)
   {
      ?>
      <item>
      <album>
      <?php
         $nextalbumname = explode("/", $parts[$i]);
         echo trim(htmlspecialchars($nextalbumname[1]));
      ?>
      </album>
      </item>
      <?php
         $i++;
   }
}
?>
</result>
