<?php

class Icons
{
  /*************************************
   * The playercontrol icons
   *************************************/
   function getplayPlay()
  {
    $img = Icons::toImageFile("playPlay");
    $alt = "Play song";
    return Icons::toImageTag($img, $alt);
  }
   function getplayPause()
  {
    $img = Icons::toImageFile("playPause");
    $alt = "Pause song";
    return Icons::toImageTag($img, $alt);
  }
   function getplayRewind()
  {
    $img = Icons::toImageFile("playRewind");
    $alt = "Rewind song";
    return Icons::toImageTag($img, $alt);
  }
   function getplayForward()
  {
    $img = Icons::toImageFile("playForward");
    $alt = "Forward song";
    return Icons::toImageTag($img, $alt);
  }
   function getplayStop()
  {
    $img = Icons::toImageFile("playStop");
    $alt = "Stop song";
    return Icons::toImageTag($img, $alt);
  }
  function getVolumeOff()
    {
    $img = Icons::toImageFile("volumeoff");
    $alt = "Change volume";
    return Icons::toImageTag($img, $alt);
  }
  function getVolumeOn()
  {
    $img = Icons::toImageFile("volumeon");
    $alt = "Change volume";
    return Icons::toImageTag($img, $alt);
  }

  /*************************************
   * The playlistcontrol icons
   *************************************/
   function getplaylistForward()
  {
    $img = Icons::toImageFile("playlistForward");
    $alt = "Forward in the playlist";
    return Icons::toImageTag($img, $alt);
  }
   function getplaylistCenter()
  {
    $img = Icons::toImageFile("playlistCenter");
    $alt = "Center to the current song";
    return Icons::toImageTag($img, $alt);
  }
   function getplaylistBackward()
  {
    $img = Icons::toImageFile("playlistBackward");
    $alt = "Backward in the playlist";
    return Icons::toImageTag($img, $alt);
  }
   function getplaylistShuffle()
  {
    $img = Icons::toImageFile("playlistShuffle");
    $alt = "Shuffle the playlist";
    return Icons::toImageTag($img, $alt);
  }
   function getplaylistRepeat()
  {
    $img = Icons::toImageFile("playlistRepeat");
    $alt = "Repeat the playlist";
    return Icons::toImageTag($img, $alt);
  }
   function getplaylistClear()
  {
    $img = Icons::toImageFile("playlistClear");
    $alt = "Clear the playlist";
    return Icons::toImageTag($img, $alt);
  }
  /*************************************
   * The songs/files control icons
   *************************************/
   function getfilesAdd()
  {
    $img = Icons::toImageFile("filesAdd");
    $alt = "Add to the playlist";
    return Icons::toImageTag($img, $alt);
  }
   function getfilesPlayNext()
  {
    $img = Icons::toImageFile("filesPlayNext");
    $alt = "Insert into the playlist";
    return Icons::toImageTag($img, $alt);
  }
   function getfilesRemove()
  {
    $img = Icons::toImageFile("filesRemove");
    $alt = "Remove the playlist";
    return Icons::toImageTag($img, $alt);
  }
  /*************************************
   * Internal functions to build tags
   *************************************/
   function toImageTag($img, $alt)
  {
    return '<a class="icon"><img src="'.$img.'" alt="'.$alt.'" title="'.$alt.'"></a>';
  }
   function toImageFile($img_name)
  {
    global $configuration ;
    global $icons ;
    return 'styles/'.$configuration["style"].'/'.$icons[$img_name];
  }
}
?>