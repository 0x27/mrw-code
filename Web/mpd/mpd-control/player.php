<?php
/************************************************
 * version $Revision: 71 $
 * copyright (C) 2006 Rene de Groot
 * license http://www.gnu.org/copyleft/gpl.html GNU/GPL
 ************************************************/


 
/***********************************************
 * A class to hold and the playcontrols and status
 */
class Player
{
  var $volume = 1;
  var $repeat;
  var $random;
  var $playlist;
  var $playlistlength;
  var $xfade;
  var $state;
  var $songnr;
  var $songid;
  var $bitrate;
  var $audio;
  var $song;
  var $maxvolume = 100 ;

  function Player(&$playlist)
  {
    $this->playlist =& $playlist;
    $this->song = new Song("");
    $this->songnr = -1 ;
  }
  function allClear()
  {
    $this->song = new Song("");
    $this->songnr = -1 ;
    $this->playlist->allClear();
  }
  function center()
  {
    $this->playlist->setStart($this->songnr) ;
  }
  function getState() { return $this->state;  }
  function &getSong()  { return $this->song;   }
  function getSongnr(){ return $this->songnr; }
  function getPlaylistlength(){ return $this->playlistlength; }
  function setTime($tot, $ela)
  {
    $this->song->setTime($tot, $ela);
  }
  function setSong($i)
  {
    $this->songnr = $i;
    $this->song =& $this->playlist->getSong($i);
  }
  function isVolumeOn($i, $bars)
  {
    if($i / $bars <= $this->volume / $this->maxvolume)
    {
      return true;
    }
    return false;
  }
}
/***********************************************
 * A class to hold the current playlist
 */
class Playlist
{
  var $songs = array();
  var $length = 5 ;
  var $start = 0 ;

  function allClear()
  {
    $this->songs = array();
    $this->start = 0 ;
  }
  function listClear()
  {
    $this->songs = array();
  }
  function addSong($song)
  {
    $this->songs[$song->playlistPosition] =& $song;
  }
  function forward()
  {
    if($this->start + $this->length < sizeof($this->songs))
    {
      $this->start += $this->length;
    }
  }
  function backward()
  {
    if($this->start - $this->length >= 0)
    {
      $this->start -= $this->length;
    }
  }
  function &getSong($i)
  {
    $song =& $this->songs[$i];
    if(is_a($song, 'Song'))
    {
      return $song;
    }
    else
    {
      $song = new Song("");
      return $song;
    }
  }
  
  function setStart($index)
  {
    $this->start = max($index - fmod($index, $this->length), 0);
  }
  function getEnd()
  {
    return min(sizeof($this->songs), $this->start+$this->length);
  }

  function toArray()
  {
    $output = array();
    $end = $this->getEnd();
    for( $i=$this->start
       ; $i<$end
       ; $i++)
    {
      $output[$i] = $this->songs[$i];
    }
    return $output;
  }
}
/***********************************************
 * A class to hold the information of a song
 * (either playlist or file)
 */
class Song
{
  var $filename;
  var $artist;
  var $album;
  var $track;
  var $title;
  var $elapsedTime;
  var $totalTime;
  var $playlistPosition;

  function Song($f)
  {
    $this->filename = $f;
  }

  function setPlaylistPosition($p){ $this->playlistPosition = $p; }
  function getPlaylistPosition()  { return $this->playlistPosition; }
  function setArtist ($a)   { $this->artist = $a; }
  function setAlbum($a)     { $this->album = $a ; }
  function setTitle ($t)    { $this->title = $t;  }
  function setTrack($t)     { $this->track = $t;  }
  function getElapsedTime() { return Song::ppTime($this->elapsedTime); }
  function getTotalTime()   { return Song::ppTime($this->totalTime) ; }
  function setTime($b, $a = 0)
  {
    $this->elapsedTime = $a;
    $this->totalTime = $b;
  }

  function getFilename($escaping=HTMLESCAPE)
  {
    $output = $this->filename;
    switch($escaping)
    {
      case(HTMLESCAPE):
        $output = htmlentities( $output, ENT_COMPAT, "UTF-8" ) ;
        break;
      case(URLESCAPE):
        $output = rawurlencode($output);
        break;
    }
    return $output;
  }

  function toString( $escaping=HTMLESCAPE)
  {
    $output = "";
    if(isset($this->artist) && isset($this->title) )
    {
      $output = $this->artist." - ".$this->title;
    }
    else
    {
      $file = substr(strrchr($this->filename, "/"), 1);
      $file = substr($file, 0, strrpos($file, ".") );
      $output =  $file;
    }
    switch($escaping)
    {
      case(HTMLESCAPE):
        $output = htmlentities( $output, ENT_COMPAT, "UTF-8" ) ;
        break;
      case(URLESCAPE):
        $output = rawurlencode($output);
        break;
    }
    return $output;
  }

  function ppTime($secs)
  {
    $output = "" ;
    $hours = floor($secs / (60*60));
    $secs -= $hours*(3600);
    $minutes = floor($secs / 60);
    $secs -= $minutes*60;
    if($hours > 0)
    {
      $output - $hours.":";
    }
    $output .= $minutes.":";
    $secs < 10
      ? $output .= "0" . $secs
      : $output .= $secs ;
    return $output ;
  }
}
  /**************
   * Class to hold and cache the set of playlists in the
   * root of the MPD songs library
   */
  class PlaylistList
  {
    var $currentPlaylists;

    function PlaylistList()
    {
       $this->currentPlaylists = array();
    }

    function add($playlistName)
    {
      if( !in_array($playlistName, $this->currentPlaylists ) )
      {
        array_push($this->currentPlaylists, $playlistName);
      }
    }

    function remove($playlistName)
    {
      $key = array_search($playlistName, $this->currentPlaylists);
      unset($this->currentPlaylists[$key]);
    }

    function &getList()
    {
      return $this->currentPlaylists;
    }
  }
?>