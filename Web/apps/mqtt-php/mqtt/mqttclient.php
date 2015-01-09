<?php


 /**
  *
  * The MQTTClient class allows you to connect to a broker and publish messages
  * @author  Robert Smart
  *
  * @version 1.0, 04 August 2006
  * (C) Copyright IBM Corp. 2002,2006
  */




class MQTTClient
{

	private $connection;
	private $clientid;
	private $broker;
	private $port;

	/**
	 * Creates and MQTTClient object
	 * @param   clientid   the unique id that this class will connect with, you may want to generate based on browser session id etc.
	 * @param   broker   the address of the microbroker e.g. "realtime.ngi.ibm.com"
	 * @param   port   port that the broker is running on, microbroker usually runs on 1883
	 */

	function MQTTClient($clientid,$broker,$port)
	{
		$this->clientid = $clientid;
		$this->broker = $broker;
		$this->port = $port;
	}





	/**
	 * Connect to the broker, opens a socket to the broker and sends the connection string
	 * @return error no, 0 is for ok
	 */

	public function connect()
	{



		// CONNECT **********************
		// fixed header: msg type (4) dup (1) qos (2) retain (1)
	  	$fixed = chr(1*16);

		// variable header: protocol name, protocol version, connect flags, keepalive
		$variable = $this->UTF("MQIsdp").chr(3).chr(2).chr(0).chr(0);

		// payload: client ID
		$payload = $this->UTF($this->clientid);

		// add in the remaining length field and fix it together
  		$msg = $fixed . $this->remaining_length(strlen($variable)+strlen($payload)) . $variable . $payload;

		$this->connection =  fsockopen($this->broker , $this->port, $errno, $errstr, 30);
		$this->sendpacket($msg);

		return $errno;

	}



	/**
	 * Publish a message to the broker
	 * @param   topic   the topic you wish to publish on e.g. temperature/spain
	 * @param   qos     the quality of service
	 * @param   retain  y/n  indicates whether the publication is retained by the broker (i.e. a client subscribing to the topic will get the last message sent)
	 * @param   content the message string to be published
	 */

	public function publish($topic,$qos,$retain,$content)
	{


		if($retain==="y")
			$retain =1;
		else
			$retain = 0;

		// fixed header: msg type (4) dup (1) qos (2) retain (1)
		$fixed = chr(3*16 + $qos*2 + $retain);

		// variable header
		$variable = $this->UTF($topic);

		if ($qos == 1)
		  	$variable .= chr(0).chr(1);


  		$msg = $fixed . $this->remaining_length(strlen($variable)+strlen($content)) . $variable . $content;

  		// now send the publish string
		$this->sendpacket($msg);

	}





	/**
	 * Send a disconnect message to the broker and close the socket
	 */
	public function disconnect()
	{
		// fixed header: msg type (4) dup (1) qos (2) retain (1)
		$fixed = chr(14*16);
		$msg = $fixed . chr(0);

		// send the disconnect packet to the broker
		$this->sendpacket($msg);

		// close the connection
		fclose($this->connection);
	}





	/**
	 * Write data to the borker socket
	 * @param packet the data to send
	 */
	private function sendpacket($packet)
	{
		// check we're connected
		if (!$this->connection)
			echo "$errstr ($errno) The client is not connected to the broker, call connect first<br/>";
		else
			fwrite($this->connection, $packet);

	}





	/**
	 * return the UTF-8 encoded version of the parameter
	 * @param string to be encoded
	 * @return utf-8 encoded parameter
	 */
	function UTF($string)
	{
	  	$length = strlen($string);
	  	return chr($length/256).chr($length%256).$string;
	}




	/**
	 * return the remaining length field bytes for an integer input parameter
	 * @param x
	 * @return remaing length
	 */
	function remaining_length($x)
		{

		    do
		    {
		      $digit = $x % 128;
		      $x = intval($x/128);

		      // if there are more digits to encode, set the top bit of this digit
		      if ( $x > 0 )
		      {
		        $digit |= 0x80;
		      }

		      $rlf .= chr($digit);
		    } while ($x > 0);

		    return $rlf;
	}


}

?>