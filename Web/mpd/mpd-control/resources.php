<?php
/************************************************
 * version $Revision: 62 $
 * copyright (C) 2006 Rene de Groot
 * license http://www.gnu.org/copyleft/gpl.html GNU/GPL
 ************************************************/
 
class Resources
{
  var $mpd ;
  var $id ; 

  function Resources()
  {
    $this->mpd = new Mpd;
  }

   function startSession()
  {
    global $configuration ;
    session_name("SessionFor".$configuration["mpd_host"].$configuration["mpd_port"]);
    session_start();
  }

   function &getMPD()
  {
    Resources::startSession();
    
    if( array_key_exists("resources", $_SESSION) )
    {
      $resources =& $_SESSION["resources"];
      $resources->mpd->connection->connect() ;
    }
    else
    {
      $_SESSION["resources"] = new Resources;
      $resources =& $_SESSION["resources"];
      $resources->mpd->connection->connect() ;
      $resources->mpd->initialize();
    }
    $resources->mpd->connection->error["message"] .= "Resources id ".$resources->mpd->id."\n";
    return $resources->mpd;;
  }
}


?>
