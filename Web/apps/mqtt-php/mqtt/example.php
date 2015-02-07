<?php

require_once("mqttclient.php");

$client = new MQTTClient("rob","realtime.ngi.ibm.com",1883);
$client->connect();
$client->publish("robsmart",0,"n","test message 1");
$client->publish("robsmart",0,"n","test message 2");
$client->publish("robsmart",0,"n","test message 3");
$client->disconnect();


?>