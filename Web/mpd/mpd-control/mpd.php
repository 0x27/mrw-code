<?php
/************************************************
 * version $Revision: 71 $
 * copyright (C) 2006 Rene de Groot
 * license http://www.gnu.org/copyleft/gpl.html GNU/GPL
 ************************************************/

define("HTMLESCAPE",     1);
define("URLESCAPE",     2);

Class Mpd
{
  var $connection;
  var $currentFiles = array();
  var $currentDirs = array();
  var $currentSearch = array();
  var $player;
  var $playlist;
  var $playlistList;
  var $id;
  
  function Mpd()
  {
    $this->connection = new Connection;
    $this->playlist = new Playlist;
    $this->player = new Player($this->playlist);
    $this->playlistList = new PlaylistList;
  }

  function initialize()
  {
    $this->updateLists();
    $this->updatePlayList();
    $this->updatePlayer();
  }

  function &getDirectory()  { return $this->connection->directory; }
  function &getDirectories(){ return $this->currentDirs; }
  function &getFiles()      { return $this->currentFiles; }
  function &getPlayer()     { return $this->player; }
  function &getPlaylist()   { return $this->playlist; }
  function &getPlaylists()  { return $this->playlistList; }
  function &getSearch()     { return $this->currentSearch; }
  
/*****************************************
 * 
 *  Actions functions alter the state of the MPD server
 */
  function changeDiretory($dir)
  {
    $this->connection->setDirectory("\"".$dir."\"");
    return $this->updateLists() ;
  }

  function playerSetVolume($level, $max)
  {
    $mpd_level = ($level * 100) / $max  ;
    $success = is_array($this->connection->do_mpd_command("setvol $mpd_level", false));
    $this->updatePlayer();
    return $success;
  }
  function playerAction($action)
  {
    switch( $action )
    {
      case "play":
        $success = is_array($this->connection->do_mpd_command("play", false));
        break;
      case "pause":
        $success = is_array($this->connection->do_mpd_command("pause", false));
        break;
      case "next":
        $success = is_array($this->connection->do_mpd_command("next", false));
        break;
      case "previous":
        $success = is_array($this->connection->do_mpd_command("previous", false));
        break;
      case "stop":
        $success = is_array($this->connection->do_mpd_command("stop", false));
        break;
    }
    $this->updatePlayer();
    return $success;
  }
  
  function playNow($songNr)
  {
    $success = is_array($this->connection->do_mpd_command("play $songNr", false));
    if($success)
    {
      $success = $this->updatePlayList();
    }
    return $success;
  }
  function removeSongFromPlaylist($playlistPosition)
  {
    $success = is_array($this->connection->do_mpd_command("delete $playlistPosition", false));
    if($success)
    {
      $success = $this->updatePlayList();
    }
    return $success;
  }
  
  function playNextAction($filename)
  {
    $success = $this->playlistAddFileAction($filename) ;
    if($success)
    {
      $old = -1;
      $new = -1;
      $new = $this->player->getSongnr() + 1;
      $old = $this->player->getPlaylistlength();
      $success = $new >= 0 && $old >= 0 && $old > $new;
      if($success)
      {
        $success = is_array($this->connection->do_mpd_command("move $old $new", false));
        if($success)
        {
          $success = $this->updatePlayList();
        }
      }
    }
    return $success;
  }
  function playlistAddDirAction($dirname)
  {
    return $this->playlistAddFileAction($dirname);
  }
  function playlistAddFileAction($filename)
  {
    $action = 'add "'.$filename.'"';
    $success = is_array($this->connection->do_mpd_command($action, false));
    if($success)
    {
      $success = $this->updatePlayList();
    }
    return $success;
  }
  function playlistAddPlaylistAction($playlist)
  {
    $action = 'load "'.$playlist.'"';
    $success = is_array($this->connection->do_mpd_command($action, false));
    if($success)
    {
      $success = $this->updatePlayList();
    }
    return $success;
  }
  function playlistAction($action)
  {
    $success = true;
    switch( $action )
    {
      case "forward":
        $this->playlist->forward();
        break;
      case "backward":
        $this->playlist->backward();
        break;
      case "center":
        $this->player->center();
        break;
      case "shuffle":
        if($this->player->random)
        {
          $success
            = is_array($this->connection->do_mpd_command("random 0", false));
        }
        else
        {
          $success
            = is_array($this->connection->do_mpd_command("random 1", false));
        }
        break;
      case "repeat":
        if($this->player->repeat)
        {
          $success
            = is_array($this->connection->do_mpd_command("repeat 0", false));
        }
        else
        {
          $success
            = is_array($this->connection->do_mpd_command("repeat 1", false));
        }
        break;
      case "clear":
        $success = is_array($this->connection->do_mpd_command("clear", false));
        $this->player->allClear();
        $this->updatePlayList();
        break;
    }
    $this->updatePlayer();
    return $success;
  }
  function search($type, $query)
  {
    $success = true;
    $response = array();
    switch( $type )
    {
      case "title":
        $action = 'search title "'.$query.'"';
        $response = $this->connection->do_mpd_command($action, false);
        break;
      case "artist":
        $action = 'search artist "'.$query.'"';
        $response = $this->connection->do_mpd_command($action, false);
        break;
      case "filename":
        $action = 'search filename "'.$query.'"';
        $response = $this->connection->do_mpd_command($action, false);
        break;
      case "album":
        $action = 'search album "'.$query.'"';
        $response = $this->connection->do_mpd_command($action, false);
        break;
    }
    if( is_array($response) )
    {
      $this->currentSearch = array();
      $this->parseSongsToArray($response, $this->currentSearch);
    }
  }
  function savePlaylist($name)
  {
    $action = 'save "'.$name.'"';
    $response = $this->connection->do_mpd_command($action, false);
    if( is_array($response) )
    {
      $this->playlistList->add($name);
      return $true;
    }
    return false;
  }
  function removePlaylist($name)
  {
    $action = 'rm "'.$name.'"';
    $response = $this->connection->do_mpd_command($action, false);
    $this->playlistList->remove($name);
    if( is_array($response) )
    {
      return $true;
    }
    return false;
  }
  function updateMPD()
  {
    $action = 'update';
    $response = $this->connection->do_mpd_command($action, false);
  }
/*****************************************
 * 
 *  Update functions to read information from the MPD 
 */
  function updatePlayer()
  {
    $pairs = $this->connection->do_mpd_command("status", false);
    if( is_array($pairs) )
    {
      foreach($pairs as $key => $value)
      {
        switch( $value[0] )
        {
          case "volume":
            $this->player->volume = $value[1];
            break;
          case "repeat":
            $this->player->repeat = $value[1];
            break;
          case "random":
            $this->player->random = $value[1];
            break;
//           case "playlist":
//             $this->player->playlist = $value[1];
            break;
          case "playlistlength":
            $this->player->playlistlength = $value[1];
            break;
          case "xfade":
            $this->player->xfade = $value[1];
            break;
          case "state":
            $this->player->state = $value[1];
            break;
          case "song":
            $this->player->setSong($value[1]);
            break;
          case "songid":
            $this->player->songid = $value[1];
            break;
          case "time":
            $elap  = trim(strtok($value[1], ":"));
            $total = trim(strtok("\0"));
            $this->player->setTime($total, $elap);
            break;
          case "bitrate":
            $this->player->bitrate = $value[1];
            break;
          case "audio":
            $this->player->audio = $value[1];
            break;
        }
      }
      return true;
    }
    return false;
  }

  function updatePlayList()
  {
    $pairs = $this->connection->do_mpd_command("playlistinfo", false);
    $this->playlist->listClear();
    $song = null;
    if( is_array($pairs) )
    {
      foreach($pairs as $key => $value)
      {
        switch( $value[0] )
        {
          case "file":
            if(isset($song))
            {
              $this->playlist->addSong($song);
              $song = null;
            }
            $song = new Song($value[1]);
            break;
          case "Artist":
            $song->setArtist($value[1]);
            break;
          case "Album":
            $song->setAlbum($value[1]);
            break;
          case "Track":
            $song->setTrack($value[1]);
            break;
          case "Title":
            $song->setTitle($value[1]);
            break;
          case "Time":
            $song->setTime($value[1]);
            break;
          case "Pos":
            $song->setPlaylistPosition($value[1]);
            break;
        }
      }
      if(isset($song))
      {
        $this->playlist->addSong($song);
        $song = null;
      }
      return true;
    }
    return false;
  }
  function updateLists()
  {
    $pairs = $this->connection->do_mpd_command("lsinfo");
    $this->currentFiles = array();
    $this->currentDirs = array();
    $song = null;
    if( is_array($pairs) )
    {
      $this->parseSongsToArray($pairs, $this->currentFiles);
      return true;
    }
    return false;
  }
  function parseSongsToArray($pairs, &$array)
  {
    foreach($pairs as $key => $value)
    {
      switch( $value[0] )
      {
        case "file":
          if(isset($song))
          {
            array_push($array, $song);
            $song = null;
          }
          $song = new Song($value[1]);
          break;
        case "Artist":
          $song->setArtist($value[1]);
          break;
        case "Album":
          $song->setAlbum($value[1]);
          break;
        case "Track":
          $song->setTrack($value[1]);
          break;
        case "Title":
          $song->setTitle($value[1]);
          break;
        case "Time":
          $song->setTime($value[1]);
          break;
        case "directory":
          array_push($this->currentDirs, $value[1]);
          break;
        case "playlist":
          $this->playlistList->add($value[1]);
          break;
      }
    }
    if(isset($song))
    {
      array_push($array, $song);
      $song = null;
    }
  }
   function succeeded($str)
  {
    $str = ltrim($str);
    return strncmp("OK", $str, 2) == 0;
  }

   function failed($str)
  {
    $str = ltrim($str);
    if( strncmp("ACK", $str, 3) == 0 )
    {
      return array("code" => 1, "message" => $str );
    }
    return false;
  }
}


?>
