<?php

# This is metar2300.conf. It is a configuration file for metar2300.php.inc.

# Station identifier
# The station identifier is normally a 4-letter abbreviation. Normally the ICAO
# airport code is used for this. However, you might not have an airport at
# home. In such a case, use ZZZZ, indicating that you have no official ICAO
# code. Alternatively, IATA codes are allowed too. For example, Schiphol (which
# is the main airport of the Netherlands, near Amsterdam) has the ICAO-code
# 'EHAM' and the IATA-code 'AMS'.
$station_identifier = 'ZZZZ';

# MySQL settings
# Provide the MySQL server name or IP-address, the database, the table name
# and the login credentials.
$db_hostname = 'localhost';
$db_database = 'mydatabase';
$db_username = 'myuser';
$db_password = 'mypassword';

# Time settings
# Normally METAR's are created twice each hour, at hh:25 and hh:55. However,
# you can change these times by setting the $time_offset to the first time. For
# example, for using the timestamps hh:15 and hh:45 you set this parameter to
# 15.
$time_offset = 25;         # Allowed values: 0-29
# If you want to use the current time in the timestamp, set $time_realtime to
# true.
$time_realtime = false;    # Allowed values: true, false

?>
