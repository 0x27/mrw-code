<?php
shell_exec('mpc seek '.escapeshellarg($_GET['timepercent']).'%');
?>
