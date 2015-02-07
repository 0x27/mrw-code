<!-- Top row the controls and songs in the playlist -->
<div class="top">
  <div class="element-head"> Player </div>
  <div class="element-body">
    <!-- Controls -->
    <div class="player">
      <?php echo Display::getPlayerAnchor($mpd);?>
    </div>
    <!-- Playlist browse controls -->
    <div class="browse">
      <?php echo Display::getPlaylistbrowsecontrolsAnchor($mpd);?>
    </div>
    <!-- Playlist -->
    <div class="playlist">
      <?php echo Display::getPlaylistAnchor($mpd);?>
    </div>
    <!-- Playlist Controls -->
    <div class="controls">
      <?php echo Display::getPlaylistcontrolsAnchor($mpd);?>
    </div>
  </div>
  
</div>
  
<!-- Bottom row with the directories, playlists, search and songs -->

<!-- Left column -->
<div class="left">
  
  <!-- Directories -->
  <div class="element-head">
      <?php echo Display::getDirectoryHeaderAnchor($mpd);?> 
  </div>
  <div class="element-body">
      <?php echo Display::getDirectoryListAnchor($mpd);?>
  </div>

  <!-- Saved Playlists -->
  <div class="element-head"> Playlists </div>
  <div class="element-body">
      <?php echo Display::getPlaylistListAnchor($mpd);?>

  </div>
</div>

<!-- Right column -->
<div class="right">

    <!-- Search -->
    <div class="element-head">
      <div class="left"> Search </div>
      <?php echo Display::getUpdateAnchor($mpd); ?> 
    </div>
    <div class="element-body">
        <?php echo Display::getSearchAnchor($mpd);?>
    </div>

    <!-- File list -->
    <div class="element-head"> Songs </div>
    <div class="element-body">
        <?php echo Display::getFilesListAnchor($mpd);?>
    </div>
</div>