# metar2300

PHP code which reads your WS2300 weather station and converts into a nice
METAR string.

# Description

This PHP library implements the METAR [1] format for weather stations that
are supported by the Open2300 software [2]. Since the ws2300 series weather
stations only support a subset of the parameters used in METAR, only these
parts are implemented.

It basically provides functions to create a METAR based on data stored in a
MySQL table which is filled by mysql2300, which is part of Open2300. The 
idea is that you have a weather station that is connected to a linux computer
running mysql2300 every minute. The data is stored in a table, which is
parsed by this library to create the requested METAR.

This library includes the function metar2300(). It will return a string
containing the METAR for the given time. Other functions could be used to get
parts of the METAR, but these are not documented yet.

# Configuration

The file metar2300.conf.php.inc should reside in the same directory as
metar2300.inc or in a directory listed in the include_path setting in your
php.ini. For security reasons we recommend you to use the second option,
since the configuration file includes login credentials for your MySQL
database. You may also consider using a seperate MySQL account for metar2300
which is only allowed to use the "SELECT" SQL statement on the weather table.

# Syntax

Syntax for metar2300():

```php
$metar = metar2300 (int $timestamp, mixed $report_modifier);
```

# To do

This is the first release. There is still lots of work to do. For myself I
would like to add the following features:

- implement variable wind direction (examples: 180V240, VRB03KT)
- convert rain_1h field to something like -RA, RA, +RA

# References

- [1] https://en.wikipedia.org/wiki/METAR
- [2] http://lavrsen.dk/foswiki/bin/view/Open2300/

