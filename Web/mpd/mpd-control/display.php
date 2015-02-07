<?php
/************************************************
 * version $Revision: 71 $
 * copyright (C) 2006 Rene de Groot
 * license http://www.gnu.org/copyleft/gpl.html GNU/GPL
 ************************************************/
 
class Display
{

  /*****************************************************************
   * Player table and rows
   */
 function getPlayerAnchor($mpd)
  {
    $output  = "<div id=\"playerrows\">\n";
    $output .= Display::getPlayerTable($mpd);
    $output .= "</div>\n";
    return $output;
  }
 function getPlayerTable($mpd)
  {
    $player = $mpd->getPlayer();
    $song = $player->getSong();
    $output = "";
    if($player->getState() == "play")
    {
      $output  .= '<script type="text/javascript">playerTimer()</script>';
    }
    $output .= "\n\n<table>\n";
    $output .= "<tr><th colspan=\"4\">";
    $output .= $player->getSongnr() . ". " . $song->toString(HTMLESCAPE);
    $output .= "</th></tr>\n";

    $output .= "<tr><td colspan=\"4\">";
    $output .= "<table><tr>";
    $link = 'class="body-clickable" onclick="player(\'previous\')"';
    $output .= "<td $link>".Icons::getplayRewind()."</td>";
    if($player->getState() == "play")
    {
      $link = 'class="body-clickable" onclick="player(\'pause\')"';
      $output .= "<td $link>".Icons::getplayPause();
      $output .= "</td>";
    }
    else
    {
      $link = 'class="body-clickable" onclick="player(\'play\')"';
      $output .= "<td $link>".Icons::getplayPlay();
      $output .= "</td>";
    }
    $link = 'class="body-clickable" onclick="player(\'next\')"';
    $output .= "<td $link>".Icons::getplayForward()."</td>";
    $link = 'class="body-clickable" onclick="player(\'stop\')"';
    $output .= "<td $link>".Icons::getplayStop()."</td>";

    $output .= "</tr></table>";
    $output .= "</td></tr>\n";

    $output .= "<tr>";
    $output .= "<td colspan=\"4\">"
             . $song->getElapsedTime()." / ".$song->getTotalTime()
             . "</td>";
    $output .= "</tr>\n";
    $output .= "<tr><td colspan=\"4\">";
    $bars = 20;
    for($i=1  ;$i<=$bars;$i++)
    {
      $output .=  '<span onclick="setVolume('.$i.' , '.$bars.')">';
      if( $player->isVolumeOn($i, $bars) )
      {
        $output .= Icons::getVolumeOn();
      }
      else
      {
        $output .= Icons::getVolumeOff();
      }
      $output .=  '</span>';
    }

    $output .= "</td></tr>";
    $output .= "</table>\n";
    return $output;
  }
  /*****************************************************************
   * Playlist table and rows
   */
  function getPlaylistAnchor($mpd)
  {
    $output  = "<div id=\"playlist\">\n";
    $output .= Display::getPlaylistTable($mpd);
    $output .= "</div>\n";
    return $output;
  }
  function getPlaylistTable($mpd)
  {
    $output = "<table>";
    $player = $mpd->getPlayer();
    $songplaying = $player->getSong();
    $playlist = $mpd->getPlaylist();
    $stringarray = $playlist->toArray();
    foreach($stringarray as $i => $song)
    {
      if($songplaying === $song)
      {
        $output .= "<tr class=\"current\">";
        $output .= '<td class="body-clickable" onclick="removeSongFromPlaylist('. $i .')">';
        $output .= Icons::getplaylistClear();
        $output .= "<td> $i </td>";
        $output .= "<td>".$song->toString(HTMLESCAPE)."</td></tr>";
      }
      else
      {
        $link = 'class="body-clickable" onclick="playnow('.$song->getPlaylistPosition().')"';
        $output .= "<tr>";
        $output .= '<td class="body-clickable" onclick="removeSongFromPlaylist('.$i.')">';
        $output .= Icons::getplaylistClear();
        $output .= '</td>';
        $output .= "<td>".$i.". </td><td $link>";
        $output .= $song->toString(HTMLESCAPE)."</td></tr>";
      }
    }
    $output .= "</table>";
    return $output;
  }

  /*****************************************************************
   * Playlist browse controls table and rows
   */
  function getPlaylistbrowsecontrolsAnchor($mpd)
  {
    $output  = "<div id=\"playlistbrowsecontrols\">\n";
    $output .= Display::getPlaylistbrowsecontrolsTable($mpd);
    $output .= "</div>\n";
    return $output;
  }
  function getPlaylistbrowsecontrolsTable($mpd)
  {
    $output = "<table>";
    $output .= '<tr><td class="body-clickable" onclick="playlist(\'backward\')">';
    $output .= Icons::getplaylistBackward().'</td></tr>'."\n";
    $output .= '<tr><td class="body-clickable" onclick="playlist(\'center\')">';
    $output .= Icons::getplaylistCenter().'</td></tr>'."\n";
    $output .= '<tr><td class="body-clickable" onclick="playlist(\'forward\')">';
    $output .= Icons::getplaylistForward().'</td></tr>'."\n";
    $output .= "</table>";
    return $output;
  }

  /*****************************************************************
   * Playlist controls table and rows
   * Like Shuffle and Repeat
   */
  function getPlaylistcontrolsAnchor($mpd)
  {
    $output  = "<div id=\"playlistcontrols\">\n";
    $output .= Display::getPlaylistcontrolsTable($mpd);
    $output .= "</div>\n";
    return $output;
  }
  function getPlaylistcontrolsTable($mpd)
  {
    $player = $mpd->getPlayer();
    $repeat = "off";
    if($player->repeat){ $repeat="on";}
    $shuffle = "off";
    if($player->random){ $shuffle="on";}
    $output = "<table>";
    $output .= '<tr><td class="'.$shuffle.'" onclick="playlist(\'shuffle\')">';
    $output .= Icons::getplaylistShuffle().'</td></tr>'."\n";
    $output .= '<tr><td class="'.$repeat.'" onclick="playlist(\'repeat\')">';
    $output .= Icons::getplaylistRepeat().'</td></tr>'."\n";
    $output .= '<tr><td class="body-clickable" onclick="playlist(\'clear\')">';
    $output .= Icons::getplaylistClear().'</td></tr>'."\n";
    $output .= "</table>";
    return $output;
  }
  
  /*****************************************************************
   * Current Directory table and rows
   */
  function getDirectoryHeaderAnchor($mpd)
  {
    $output = "\n<div id=\"directoryheader\">";
    $output .= Display::getCurrentDirectoryTable($mpd);
    $output .= "\n</div>\n";
    return $output;
  }
  function getCurrentDirectoryTable($mpd)
  {
    $directory = $mpd->getDirectory() ;
    $directory = trim($directory, "\"");
    $tokenizer = strtok($directory, "/"); 
    $dirlink = "";
    $output  = '<a class="head-unclickable">Directory:</a>' ;
    $output .= '<a class="head-clickable" onclick="changeDirectory(\'/\')">Music/</a>';
    while ($tokenizer !== false)
    {
      $dirlink .= $tokenizer."/";
      $output .= '<div class="head-clickable" onclick="changeDirectory(\''.
                  rawurlencode($dirlink).'\')">'.htmlentities( $tokenizer, ENT_COMPAT, "UTF-8" ).'/</div>';
      $tokenizer = strtok("/");
    }
    return $output ;
  }

  /*****************************************************************
   * Directories table and rows
   */
  function getDirectoryListAnchor($mpd)
  {
    $output = "<div id=\"directory\">\n";
    $output .= Display::getDirectoryListTable($mpd);
    $output .= "</div>\n";
    return $output;
  }
  function getDirectoryListTable($mpd)
  {
    $output = "\n<table>\n";
    $ls = $mpd->getDirectories();
    foreach($ls as $value)
    {
      $playlink = 'class="body-clickable" onclick="addDirectory(\''.rawurlencode($value).'\')"';
      $opendirlink = 'class="body-clickable" onclick="changeDirectory(\''.rawurlencode($value).'\')"';
      $output .= "<tr><td $playlink>";
      $output .= Icons::getfilesAdd() ;
      $output .= "</td><td $opendirlink>";
      $output .= htmlentities( $value, ENT_COMPAT, "UTF-8" );
      $output .= "</td></tr>\n";
    }
    $output .= "</table>\n";
    return $output;
  }

  /*****************************************************************
   * Playlistlist table and rows
   */
  function getPlaylistListAnchor($mpd)
  {
    $output  = "<div id=\"playlistlist\">\n";
    $output .= Display::getPlaylistListTable($mpd);
    $output .= "</div>\n";
    return $output;
  }
  function getPlaylistListTable($mpd)
  {
    $playlistList = $mpd->getPlaylists();
    $ls = $playlistList->getList();
    $output = "\n<table>\n";
    $output .= '<tr><td colspan="2">';
    $output .= 'Current playlist ';
    $output .= '<input type="text" id="newplaylist"> ';
    $output .= '<button onclick="savePlaylist(getElementById(\'newplaylist\').value)"> Save</button>';
    $output .= '</td></tr>';
    foreach($ls as $playlist)
    {
      $output .= '<tr>';
      $output .= '<td class="body-clickable" style="width: 10%"';
      $output .= 'onclick="removePlaylist(\''.$playlist.'\')">';
      $output .= Icons::getfilesRemove()."</td>\n";
      $output .= '<td class="body-clickable" onclick="addPlaylist(\''.$playlist.'\')">';
      $output .= $playlist."</td></tr>\n";
    }
    $output .= "</table>\n";
    return $output;
  }
  
  /*****************************************************************
   * Search table and rows
   */
  function getSearchAnchor($mpd)
  {
    $output  = "<div id=\"search\">\n";
    $output .= Display::getSearchTable($mpd);
    $output .= "</div>\n";
    return $output;
  }
  function getSearchTable($mpd)
  {
    $output = "\nFor ";
    $output .= '<select id="typefield">';
    $output .= '<option value="title">title';
    $output .= '<option value="artist">artist';
    $output .= '<option value="album">album';
    $output .= '<option value="file">filename';
    $output .= '</select> with the keyword ';
    $output .= '<input type="text" id="searchfield"> ';
    $output .= '<button onclick="search(getElementById(\'typefield\').value,
                getElementById(\'searchfield\').value)">Search</button>';
    $output .= "\n";
    return $output;
  }
  function getSearchResultsTable($mpd)
  {
    $output = "<table>";
    $output .= Display::getSearchRows($mpd);
    $output .= "</table>\n";
    return $output;
  }
  function getSearchRows($mpd)
  {
    $ls = $mpd->getSearch();
    foreach($ls as $song)
    {
      $nextlink = 'class="body-clickable" onclick="playNext(\''.$song->getFilename(URLESCAPE).'\')"';
      $addlink = 'class="body-clickable" onclick="addSong(\''.$song->getFilename(URLESCAPE).'\')"';
      $output .= "<tr>";
      $output .= "<td $addlink>".Icons::getfilesAdd()."</td>\n";
      $output .= "<td $nextlink>";
      $output .= Icons::getfilesPlayNext().$song->toString(HTMLESCAPE)."</td>";
      $output .= "</tr>\n";
    }
    return $output;
  }
  /*****************************************************************
   * Update button
   */
  function getUpdateAnchor($mpd)
  {
    $output = '<div class="button"><button onclick="updateMPD()">Update MPD</button></div>';
    return $output ;
  }
  /*****************************************************************
   * Files/Songs table and rows
   */
  function getFilesListAnchor($mpd)
  {
    $output  = "<div id=\"fileslist\">\n";
    $output .= Display::getFilesListTable($mpd);
    $output .= "</div>\n";
    return $output;
  }
  function getFilesListTable($mpd)
  {
    $output = "\n<table>\n";
    $ls = $mpd->getFiles();
    foreach($ls as $song)
    {
      $nextlink = 'class="body-clickable" onclick="playNext(\''.$song->getFilename(URLESCAPE).'\')"';
      $addlink = 'class="body-clickable" onclick="addSong(\''.$song->getFilename(URLESCAPE).'\')"';
      $output .= "<tr>";
      $output .= "<td $addlink>".Icons::getfilesAdd()."</td>\n";
      $output .= "<td $nextlink>";
      $output .= Icons::getfilesPlayNext().$song->toString(HTMLESCAPE)."</td>";
      $output .= "</tr>\n";
    }
    $output .= '<tr><td/><td>';
    $output .= 'Stream URL: ';
    $output .= '<input type="text" id="addStream"> ';
    $output .= '<button onclick="addStream(getElementById(\'addStream\').value)"> add</button>';
    $output .= '</td></tr>';
    $output .= "</table>\n";
    return $output;
  }
}
?>