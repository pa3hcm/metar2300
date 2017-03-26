<?php

header ("Content-type: text/plain");
include ("metar2300.php.inc");
print (metar2300 (time(), 'AUTO') . "\n");

?>
