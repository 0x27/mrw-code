/************************************************
 * version $Revision: 71 $
 * copyright (C) 2006 Rene de Groot
 * license http://www.gnu.org/copyleft/gpl.html GNU/GPL
 ************************************************/

/*
 * Different action functions for the different actions
 */
var playerTimeout;

function setVolume(level , max)
{
  xmlhttpActionPost("action.php", "setVolume="+level+"&maxVolume="+max);
}

function addStream(streamUrl)
{
  xmlhttpActionPost("action.php", "addsong="+streamUrl);
}

function removeSongFromPlaylist(playlistPosition)
{
  xmlhttpActionPost("action.php", "removeSongFromPlaylist="+playlistPosition);
}
function playlist(action)
{
  if(action == "clear")
  {
    playerTimerStop();
  }
  xmlhttpActionPost("action.php", "playlist="+action);
}
function changeDirectory(dir)
{
  xmlhttpActionPost("action.php", "directory="+dir);
}
function player(action)
{
  xmlhttpActionPost("action.php", "player="+action);
  // Start or stop the timer with the start or stop of the song playing
  if(action == "play")
  {
    playerTimer();
  }
  else if(action == "pause" || action == ("stop"))
  {
    playerTimerStop();
  }
}
function playnow(playlistPosition)
{
  xmlhttpActionPost("action.php", "playnow="+playlistPosition);
}
function addDirectory(dir)
{
  xmlhttpActionPost("action.php", "adddir="+dir);
}
function addSong(song)
{
  xmlhttpActionPost("action.php", "addsong="+song);
}
function addPlaylist(playlist)
{
  xmlhttpActionPost("action.php", "addplaylist="+playlist);
}
function playNext(song)
{
  xmlhttpActionPost("action.php", "playnext="+song);
}
function search(type, search)
{
  xmlhttpActionPost("action.php", "type="+type+"&search="+search);
}
function savePlaylist(filename)
{
  xmlhttpActionPost("action.php", "saveplaylist="+filename);
}
function removePlaylist(filename)
{
  if( confirm("Remove the playlist "+filename+"?") )
  {
    xmlhttpActionPost("action.php", "removeplaylist="+filename);
  }
}
function updatePlayer()
{
  xmlhttpActionPost("action.php", "updateplayer");
}
function updateMPD()
{
  if( confirm("Start the MPD to update its datebase?") )
  {
    xmlhttpActionPost("action.php", "updateMPD");
  }
}
/*
 * Run a timer for the player interface element
 */
function playerTimer()
{
  updatePlayer()
  playerTimeout = setTimeout("playerTimer()",1500);
}
function playerTimerStop()
{
  clearTimeout(playerTimeout);
}
/*
 * Deal with updates returned by an async-action
 */
function handleActionResponse(responseXML)
{
  var debug = document.getElementById("debug");
  var updatelist = responseXML.getElementsByTagName("update");
  if(debug != null)
  {
    debug.value = "";
    debug.value += "----------------------------------------------------\n";
    debug.value += responseXML.getElementsByTagName("error").item(0).childNodes.item(1).childNodes.item(0).nodeValue;
    debug.value += "----------------------------------------------------\n";
  }
  for(var i=0;i<updatelist.length;i++)
  {
    var item = updatelist.item(i);
    var id = item.getAttribute("id");
    var replace = item.childNodes.item(1) ;     // FF + Konquerer + Opera
    if(replace == null)
    {
      replace = item.childNodes.item(0).childNodes.item(0) ;   // IE 
    }
    else 
    {
      replace = item.childNodes.item(1).childNodes.item(0) ;  // FF + Konquerer + Opera
    }
    if(debug != null)
    {
      debug.value += "----------------------------------------------------\n";
      debug.value += replace.nodeValue + "\n";
      debug.value += "----------------------------------------------------\n";
    }
    var org = document.getElementById(id);
    purgeDomOfJavaScriptFunctions(org);
    org.innerHTML = replace.nodeValue ;
  }
}

/*
 *Get the update.php script to relay ACTIONs
 */
function xmlhttpActionPost(url, action)
{
    var xmlHttpReq = false;
    if (window.XMLHttpRequest)
    {
        xmlHttpReq = new XMLHttpRequest();
    }
    else if (window.ActiveXObject)
    {
        xmlHttpReq = new ActiveXObject("Microsoft.XMLHTTP");
    }
    xmlHttpReq.open('GET', "mpd-control/"+url+"?"+action, true);
    xmlHttpReq.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
    xmlHttpReq.onreadystatechange =       
      function () {
        if (xmlHttpReq.readyState == 4) {
          handleActionResponse(xmlHttpReq.responseXML);
          xmlHttpReq = null;
        }
      };
    xmlHttpReq.send(null);
}


/*
 * Removing references to Javascript event handles in DOM
 * objects that will be removed after an update
 * Note: Needed to solve memory leaks in IE where the DOM
 * and JS are not part if the same garbage collector.
 * Reference and source: http://javascript.crockford.com/memory/leak.html
 */
function purgeDomOfJavaScriptFunctions(d) {
    var a = d.attributes, i, l, n;
    if (a) {
        var l = a.length;
        for (var i = 0; i < l; i += 1) {
            n = a[i].name;
            if (typeof d[n] === 'function') {
              var func = d[n];
              // Detaches the JavaScript event handler from the DOM object to 
              // make the DOM object  eligable for GC in IE
              d[n] = null;
            }
        }
    }
    a = d.childNodes;
    if (a) {
        l = a.length;
        for (i = 0; i < l; i += 1) {
            purgeDomOfJavaScriptFunctions(d.childNodes[i]);
        }
    }
}
