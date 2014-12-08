<?php
/************************************************
 * version $Revision: 71 $
 * copyright (C) 2006 Rene de Groot
 * license http://www.gnu.org/copyleft/gpl.html GNU/GPL
 ************************************************/

include("mpd.php");
include("connection.php");
include("display.php");
include("player.php");
include("resources.php");
include("style.php");
include("icons.php");
include("configuration.php");
global $configuration ;
include("../styles/".$configuration["style"]."/icons.php");

header("Content-type: text/xml; charset=utf-8");
Action::handleAction();

/*
 *  Executes the actions and orders the builds the update HTML
 */
class Action
{
  function handleAction()
  {
    $mpd =& Resources::getMPD();
    $updateXML = '<?xml version="1.0" encoding="UTF-8"?>';
    $updateXML .= "<updatelist>";
    // Determine if an action needs handling
    $success = true;
    if( array_key_exists("updateplayer", $_REQUEST) )
    {
      $mpd->updatePlayer();
      $updateXML .= Action::getPlayerTableUpdate($mpd);
      $updateXML .= Action::getPlaylistTableUpdate($mpd);
    }
    elseif( array_key_exists("directory", $_REQUEST) )
    {
      $success = $mpd->changeDiretory(Action::sanitize($_REQUEST["directory"]));
      $updateXML .= Action::getDirectoryListTableUpdate($mpd);
      $updateXML .= Action::getDirectoryHeaderUpdate($mpd);
      $updateXML .= Action::getFilesListTableUpdate($mpd);
      $updateXML .= Action::getPlaylistListTableUpdate($mpd);
    }
    elseif( array_key_exists("player", $_REQUEST) )
    {
      $success = $mpd->playerAction(Action::sanitize($_REQUEST["player"]));
      $updateXML .= Action::getPlayerTableUpdate($mpd);
      $updateXML .= Action::getPlaylistTableUpdate($mpd);
    }
    elseif( array_key_exists("setVolume", $_REQUEST)
            && array_key_exists("maxVolume", $_REQUEST) )
    {
      $success = $mpd->playerSetVolume( Action::sanitize($_REQUEST["setVolume"])
                                      , Action::sanitize($_REQUEST["maxVolume"]) );
      $updateXML .= Action::getPlayerTableUpdate($mpd);
    }
    elseif( array_key_exists("playnext", $_REQUEST) )
    {
      //TODO: secure $_REQUEST["playnow"]
      $success = $mpd->playNextAction(Action::sanitize($_REQUEST["playnext"]));
      $updateXML .= Action::getPlaylistTableUpdate($mpd);
    }
    elseif( array_key_exists("addsong", $_REQUEST) )
    {
      //TODO: secure $_REQUEST["playnow"]
      $success = $mpd->playlistAddFileAction(Action::sanitize($_REQUEST["addsong"]));
      $updateXML .= Action::getPlaylistTableUpdate($mpd);
    }
    elseif( array_key_exists("adddir", $_REQUEST) )
    {
      //TODO: secure $_REQUEST["playnow"]
      $success = $mpd->playlistAddDirAction(Action::sanitize($_REQUEST["adddir"]));
      $updateXML .= Action::getPlaylistTableUpdate($mpd);
    }
    elseif( array_key_exists("playlist", $_REQUEST) )
    {
      //TODO: secure $_REQUEST["playnow"]
      $success = $mpd->playlistAction(Action::sanitize($_REQUEST["playlist"]));
      $updateXML .= Action::getPlayerTableUpdate($mpd);
      $updateXML .= Action::getPlaylistTableUpdate($mpd);
      $updateXML .= Action::getPlaylistcontrolsTableUpdate($mpd);
    }
    elseif( array_key_exists("addplaylist", $_REQUEST) )
    {
      //TODO: secure $_REQUEST["addplaylist"]
      $success = $mpd->playlistAddPlaylistAction(Action::sanitize($_REQUEST["addplaylist"]));
      $updateXML .= Action::getPlayerTableUpdate($mpd);
      $updateXML .= Action::getPlaylistTableUpdate($mpd);
    }
    elseif( array_key_exists("type", $_REQUEST)
            && array_key_exists("search", $_REQUEST) )
    {
      //TODO: secure $_REQUEST["type"], array_key_exists("search"
      $success = $mpd->search( Action::sanitize($_REQUEST["type"])
                             , Action::sanitize($_REQUEST["search"]) );
      $updateXML .= Action::getSearchTableUpdate($mpd);
    }
    elseif( array_key_exists("saveplaylist", $_REQUEST))
    {
      //TODO: secure $_REQUEST["saveplaylist"]
      $success = $mpd->savePlaylist(Action::sanitize($_REQUEST["saveplaylist"]));
      $updateXML .= Action::getPlaylistListTableUpdate($mpd);
    }
    elseif( array_key_exists("removeplaylist", $_REQUEST))
    {
      //TODO: secure $_REQUEST["removeplaylist"]
      $success = $mpd->removePlaylist(Action::sanitize($_REQUEST["removeplaylist"]));
      $updateXML .= Action::getPlaylistListTableUpdate($mpd);
    }
    elseif( array_key_exists("updateMPD", $_REQUEST))
    {
      $success = $mpd->updateMPD();
    }
    elseif( array_key_exists("playnow",$_REQUEST) )
    {
      $success = $mpd->playNow(Action::sanitize($_REQUEST["playnow"]));
      $updateXML .= Action::getPlaylistTableUpdate($mpd);
    }
    elseif( array_key_exists("removeSongFromPlaylist",$_REQUEST) )
    {
      $success = $mpd->removeSongFromPlaylist(Action::sanitize($_REQUEST["removeSongFromPlaylist"]));
      $updateXML .= Action::getPlaylistTableUpdate($mpd);
    }
    $updateXML .= Action::getErrorReport($mpd);
    $updateXML .= "</updatelist>";
    echo $updateXML;
  }
  
  function getErrorReport($mpd)
  {
    return '<error>'
         . Action::toData($mpd->connection->getErrorMessage())
         . '</error>';
  }
  function getPlayerTableUpdate($mpd)
  {
    return '<update id="playerrows">'
         .  Action::toData(Display::getPlayerTable($mpd))
         .  '</update>';
  }
  function getCurrentDirectoryRowUpdate($mpd)
  {
    return '<update id="currentdir">'
         .  Action::toData(Display::getCurrentDirectoryTable($mpd))
         .  '</update>';
  }
  function getDirectoryListTableUpdate($mpd)
  {
    return '<update id="directory">'
         .  Action::toData(Display::getDirectoryListTable($mpd))
         .  '</update>';
  }
  function getDirectoryHeaderUpdate($mpd)
  {
    return '<update id="directoryheader">'
           .  Action::toData(Display::getCurrentDirectoryTable($mpd))
           .  '</update>';
  }
  function getFilesListTableUpdate($mpd)
  {
    return '<update id="fileslist">'
         .  Action::toData(Display::getFilesListTable($mpd))
         .  '</update>';
  }
  function getPlaylistListTableUpdate($mpd)
  {
    return '<update id="playlistlist">'
         .  Action::toData(Display::getPlaylistListTable($mpd))
         .  '</update>';
  }
  function getPlaylistTableUpdate($mpd)
  {
    return '<update id="playlist">'
         .  Action::toData(Display::getPlaylistTable($mpd))
         .  '</update>';
  }
  function getPlaylistcontrolsTableUpdate($mpd)
  {
    return '<update id="playlistcontrols">'
         .  Action::toData(Display::getPlaylistcontrolsTable($mpd))
         .  '</update>';
  }
  function getSearchTableUpdate($mpd)
  {
    return '<update id="fileslist">'
         .  Action::toData(Display::getSearchResultsTable($mpd))
         .  '</update>';
  }
  // Choose between a fast CDATA or safe ESCAPE encoding of HTML
  function toData($data)
  {
    global $configuration ;
    if( $configuration["cdata_encoding"])
    {
      return Action::toCDATAData($data);
    }
    else
    {
      return Action::toEscapedData($data);
    }
  }
  // CDATA is a fast way to encode html in an xml document
  // but is not supported by all javascript engines
  function toCDATAData($data)
  {
    return "\n\n<data>".'<![CDATA['.$data.']]>'."</data>\n";
  }
  // Escaping all xml special chars in a html string makes it safe
  // to transmit in an xml document, but requires a full string traversal
  function toEscapedData($data)
  {
    return "\n\n<data>".htmlspecialchars($data, ENT_COMPAT, "UTF-8")."</data>\n";
  }
  // The input is transmitted with "'s surrounding the input on a single line
  // therefor input with newlines isnt allowed. 
  function sanitize($str)
  {
    $str = urldecode($str);
    $str = substr(str_replace("\n", "", $str), 0, 4096);
    return $str ;
  }
}
?>
