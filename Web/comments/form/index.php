<!DOCTYPE html>
 <html lang="en" style="height: auto; width: 800px">
 <head>

   <title>Comments</title>
   <meta http-equiv="Content-Type" content="text/html; charset=utf-8">

   <!--  Load the jquery javascript code  -->

   <link type="text/css" href="css/redmond/jquery-ui-1.7.2.custom.css" rel="stylesheet" />
   <script src="/js/jquery-1.3.2.min.js" type="text/javascript"></script>
   <script src="/js/jquery-ui-1.7.2.custom.min.js" type="text/javascript"></script>
   <script language="javascript" type="text/javascript" src="/jquery/flot/jquery.flot.js"></script>

  <!--  jquery code to create the graph  -->
  <script id="source" language="javascript" type="text/javascript">
  var lastCommentSubmitTime = 0;

  function handleCommentSubmit() {
    if (lastCommentSubmitTime == 0 || ((new Date().getTime() - lastCommentSubmitTime) > 20000)) {
      var theComment = document.getElementById('comment').value;
      $.get('/comments/index.php?comment=' + theComment, function(d){
        document.getElementById('comment').value = "";
        alert('Comment submitted');
      });
    }

    lastCommentSubmitTime = new Date().getTime();
    return false;
  }
  </script>

</head>
<body>
<div id="commentarea">
  <label for="comment">Comment to add to the database </label>
  <input type="text" name="comment" id="comment" width="200" size="60"/>
  <button type="button" onClick="JavaScript:return handleCommentSubmit();">Submit</button>
</div>
</body>
</html>

