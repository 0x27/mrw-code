<?php header('Content-type: application/xml'); ?>
<?php echo("<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>\n"); ?>
<?php echo("<rss version=\"2.0\">\n"); ?>
<rss>
<channel>


<title>Stats</title>
   <link>http://matthewwhitehead.bounceme.net/</link>
   <description>Voltage of wireless node 2</description>
   <item>
      <title>Voltage Value</title>
      <link>http://matthewwhitehead.bounceme.net/</link>
      <description>

Node 2: 3.6v

</description>
   </item>
   <item>
      <title>Temp Value</title>
      <link>http://matthewwhitehead.bounceme.net/</link>
      <description>Temp: 18deg</description>
   </item>
   <item>
      <title>Other Value</title>
      <link>http://matthewwhitehead.bounceme.net/</link>
      <description>Value: 19kWh</description>
   </item>

</channel>
</rss>
