<?php

class Style
{
  function getCSS()
  {
    global $configuration ;
    return "styles/".$configuration["style"]."/stylesheet.css";
  }
}
?>