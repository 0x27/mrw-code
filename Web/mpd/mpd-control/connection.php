<?php
/************************************************
 * version $Revision: 68 $
 * copyright (C) 2006 Rene de Groot
 * license http://www.gnu.org/copyleft/gpl.html GNU/GPL
 ************************************************/
 
Class Connection
{
  var $socket ;
  var $directory ;
  var $password ;
  var $error = array();
 
  function Connection()
  {
    $this->directory = "\"\"";
  }

  function connect()
  {
    global $configuration ;

    $this->error["code"] = "";
    $this->error["message"] = "";

    $this->socket = fsockopen( $configuration["mpd_host"], $configuration["mpd_port"], $errno, $errstr, 10);
    if( $this->socket )
    {
      while( !feof($this->socket) )
      {
        $line = $this->invalidUTF8(fgets($this->socket, 1024));
        if( Mpd::succeeded($line) )
        {
          return true;
        }
        $error = Mpd::failed($line);
        if( $error )
        {
          $this->error = $error;
          return false;
        }
      }
      $this->error["code"] = 1;
      $this->error["message"] = "Connection timed out";
      return false;
    }
    $this->error["code"] = $errno ;
    $this->error["message"] = $errstr ;
    return false;
  }

  function getErrorMessage()
  {
    global $configuration;
    $msg  = "Connecting to ".$configuration["mpd_host"].":".$configuration["mpd_port"]."\n";
    $msg .= "Debug: ".$this->error["message"]."\n";
    return $msg ;
  }

  function login($pwd)
  {
    if( isset($pwd) )
    {
      $this->password = $pwd ;
    }
    if( isset($this->password) )
    {
      return do_mpd_command( "password ".$this->password );
    }
  }

  //Sends a command to mpd and parses the results.
  function do_mpd_command($command, $use_dir = true)
  {
    $this->error["message"] .= "\n================================================\n";
    if($use_dir)
    {
      $command .= " ".$this->directory;
    }
    $command .= "\n";
    $this->error["message"] .= $command ;
    $output = array();
    fputs($this->socket, $command);
    while(!feof($this->socket))
    {
      $line = $this->invalidUTF8(fgets($this->socket, 1024));
      $this->error["message"] .= $line;
      if( Mpd::succeeded($line) )
      {
          return $output;
      }
      $error = Mpd::failed($line);
      if( $error )
      {
        $this->error = $error;
        return false;
      }
      $pair = $this->parse_mpd_pair( $line );
      array_push($output, $pair);
    }
    $this->error["code"] = 1 ;
    return false;
  }
  
  function parse_mpd_pair($in_str)
  {
    $got = trim($in_str);
    $key = trim(strtok($got, ":"));
    $val = trim(strtok("\0"));
    return array(0 => $key, 1 => $val);
  }

  function setDirectory($dir)
  {
    $this->directory = $dir;
  }
  
// Bytes and order of each char-value for utf-8
// 1: (  0 - 127) (with 32-126 as basic-latin and 10 as newline)
// 2: (192 - 223) (128 - 191)
// 3: (224 - 239) (128 - 191) (128 - 191)
// 4: (240 - 247) (128 - 191) (128 - 191) (128 - 191)
// Remove chars out of spec and also basic-latin control characters
// (due to XML parse errors they produce)
// Since these text will be displayed as text in a browser
// HTML escape occure here
  function invalidUTF8($str)
  {
      $size = 0;
      $charstart = 0;
      for ($i = 0; $i < strlen($str); $i++)
      {
        $o = ord($str{$i});
        if( $size>0 )                       // need trail byte
        {
          if( $o>127 && $o<192 )                // GOOD trail byte
          {
            $size--;
            continue;
          }
          else                                  // BAD trail byte
          {
            $str = substr_replace($str, "", $charstart, $size+1);
            $i -= $size+1;
            $size = 0;
            continue;
          }
        }
        if( $o>-1 && $o<128 )               // single byte character
        {
          $size = 0;
          if( ($o>31 && $o<127) || $o==10 ) // Latin char or newline
          {
            /* Not here, text should be text without HTML / URL or any other encoding.
            switch($o)                      // HTML encode strings, done here because to avoid more string traversals
            {
              case 34: $str = substr_replace($str, "&quot;", $i, 1);$i+=5;break;
              case 38: $str = substr_replace($str, "&amp;", $i, 1);$i+=4;break;
              case 60: $str = substr_replace($str, "&lt;", $i, 1);$i+=3;break;
              case 62: $str = substr_replace($str, "&gt;", $i, 1);$i+=3;break;
            }
            */
            continue;
            
          }
          else                                  // Weird Latin controlchar
          {
            $str = substr_replace($str, "", $i, 1);
            $i --;
            continue;
          }
        }
        if( $o>191 && $o<224 )              // lead byte for 2 byte char
        {
          $size = 1;
          $charstart = $i;
          continue;
        }
        if( $o>223 && $o<240 )              // lead byte for 3 byte char
        {
          $size = 2;
          $charstart = $i;
          continue;
        }
        if( $o>239 && $o<248 )              // lead byte for 4 byte char
        {
          $size = 3;
          $charstart = $i;
          continue;
        }
        $str = substr_replace($str, "", $i, 1); // Undefinable char
        $i --;
      }
      return $str;
  }
}
?>