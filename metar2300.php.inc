<?php

###############################################################################
# Product name: metar2300
# Version: 0.9
# Author: Ernest Neijenhuis, metar2300@utreg.net
###############################################################################
#
# DESCRIPTION
#
# This PHP library implements the METAR format described in the "Federal
# Meteorological Handbook No. 1 - Surface Weather Observations and Reports"
# (FCM-H1-2005) [1] for weather stations that are supported by the Open2300
# software [2,3]. Since the ws2300 series weather stations only support a
# subset of the parameters used in a METAR, only these parts of the FCM-H1-2005
# are implemented. Other parameters are not supported.
#
# It basically provides functions to create a METAR based on data stored in a
# MySQL table which is maintained by mysql2300, which is part of Open2300. The
# idea is that you have a weather station that is connected to a linux computer
# running mysql2300 every minute. The data is stored in a table, which is
# parsed by this library to create the requested METAR.
#
# This library includes the function metar2300(). It will return a string
# containing the METAR for the given time. Other functions could be used to get
# parts of the METAR, but these are not documented yet.
###############################################################################
#
# CONFIGURATION
#
# The file metar2300.conf.php.inc should reside in the same directory as
# metar2300.inc or in a directory listed in the include_path setting in your
# php.ini. For security reasons we recommend you to use the second option,
# since the configuration file includes login credentials for your MySQL
# database. You may also consider using a seperate MySQL account for metar2300
# which is only allowed to use the "SELECT" SQL statement on the weather table.
###############################################################################
#
# SYNTAX
#
# Syntax for metar2300():
# $metar = metar2300 (int $timestamp, mixed $report_modifier);
###############################################################################
#
# TODO
#
# This is the first release. There is still lots of work to do. For myself I
# would like to add the following features:
# - implement variable wind direction (examples: 180V240, VRB03KT)
# - convert rain_1h field to something like -RA, RA, +RA
###############################################################################
#
# REFERENCES
#
# [1] http://www.ofcm.noaa.gov/fmh-1/fmh1.htm
# [2] http://open2300.sourceforge.net/
# [3] http://www.lavrsen.dk/twiki/bin/view/Open2300/WebHome
###############################################################################

function metar2300_adjust_timestamp ($timestamp, $time_offset, $time_realtime) {
	if (! $time_realtime) {
		$timestamp_rounded = intval ($timestamp / 1800) * 1800;
		if (($timestamp - $timestamp_rounded) > ($time_offset * 60)) {
			$timestamp = $timestamp_rounded + ($time_offset * 60);
		} else {
			$timestamp = $timestamp_rounded + ($time_offset * 60) - 1800;
		}
	}
	return $timestamp;
}

function metar2300_timestamp_mytime ($timestamp) {
	$mytime = date ('H:i:s', $timestamp);
	return $mytime;
}

function metar2300_timestamp_mydate ($timestamp) {
	$mydate = date ('Y-m-d', $timestamp);
	return $mydate;
}

function metar2300_timestamp ($timestamp) {
	$metar_timestamp = date ('dHi', $timestamp - date ('Z', $timestamp)) . 'Z';
	return $metar_timestamp;
}

function metar2300_report_modifier ($report_modifier) {
	switch (strtoupper ($report_modifier)) {
		case 'AUTO':
			$metar_report_modifier = 'AUTO';
			break;
		case 'COR':
			$metar_report_modifier = 'COR';
			break;
		case '':
			$metar_report_modifier = '';
			break;
		default:
			$metar_report_modifier = '';
			break;
	}
	return $metar_report_modifier;
}

function metar2300_temp_dewpoint ($timestamp) {
	$rec_date = metar2300_timestamp_mydate ($timestamp);
	$min_rec_time = metar2300_timestamp_mytime ($timestamp - 1800);
	$max_rec_time = metar2300_timestamp_mytime ($timestamp);
	$query = "SELECT temp_out, dewpoint
		FROM weather
		WHERE rec_date = '$rec_date'
		AND rec_time <= '$max_rec_time'
		AND rec_time > '$min_rec_time' 
		ORDER BY rec_time DESC
		LIMIT 1;";
	$result = mysql_query ($query);
	if (mysql_num_rows ($result) == 1) {
		extract (mysql_fetch_array ($result));
		$metar_temp = sprintf ('%02d', $temp_out);
		if ($temp_out < 0) {
			$metar_temp = 'M' . $metar_temp;
		}
		$metar_dewpoint = sprintf ('%02d', $dewpoint);
		if ($dewpoint < 0) {
			$metar_dewpoint = 'M' . $metar_dewpoint;
		}
		$temp_dewpoint = "$metar_temp/$metar_dewpoint";
	} else {
		$temp_dewpoint = "";
	}
	return $temp_dewpoint;
}

function metar2300_pressure ($timestamp) {
	$rec_date = metar2300_timestamp_mydate ($timestamp);
	$min_rec_time = metar2300_timestamp_mytime ($timestamp - 1800);
	$max_rec_time = metar2300_timestamp_mytime ($timestamp);
	$query = "SELECT rel_pressure
		FROM weather
		WHERE rec_date = '$rec_date'
		AND rec_time <= '$max_rec_time'
		AND rec_time > '$min_rec_time' 
		ORDER BY rec_time DESC
		LIMIT 1;";
	$result = mysql_query ($query);
	if (mysql_num_rows ($result) == 1) {
		extract (mysql_fetch_array ($result));
		$pressure = 'Q' . round ($rel_pressure, 0);
	} else {
		$pressure = "";
	}
	return $pressure;
}

function metar2300_wind ($timestamp) {
	$rec_date = metar2300_timestamp_mydate ($timestamp);
	$min_rec_time = metar2300_timestamp_mytime ($timestamp - 1800);
	$max_rec_time = metar2300_timestamp_mytime ($timestamp);
	$query = "SELECT windspeed, wind_angle, rec_time
		FROM weather
		WHERE rec_date = '$rec_date'
		AND rec_time <= '$max_rec_time'
		AND rec_time > '$min_rec_time' 
		ORDER BY rec_time DESC
		LIMIT 1;";
	$result = mysql_query ($query);
	if (mysql_num_rows ($result) == 1) {
		extract (mysql_fetch_array ($result));
		$wind_angle = sprintf ('%03d', round ($wind_angle, -1));
		$windspeed = round ($windspeed * 1.9438445, 0);
		if ($windspeed < 10) {
			$windspeed = sprintf ('%02d', $windspeed);
		}
		# Calm wind
		if ($windspeed <= 0) {
			$wind_angle = '000';
		}
		# Gusts
		$query2 = "SELECT MIN(windspeed) AS min_windspeed, MAX(windspeed) AS max_windspeed
			FROM weather
			WHERE rec_date = '$rec_date'
			AND rec_time <= '$rec_time'
			AND rec_time > (SEC_TO_TIME(TIME_TO_SEC('$rec_time') - 600));";
		$result2 = mysql_query ($query2);
		if (mysql_num_rows ($result2) == 1) {
			extract (mysql_fetch_array ($result2));
			$max_windspeed = round ($max_windspeed * 1.9438445, 0);
			$min_windspeed = round ($min_windspeed * 1.9438445, 0);
			if ($max_windspeed >= ($min_windspeed + 10)) {
				if ($min_windspeed < 10) {
					$min_windspeed = sprintf ('%02d', $min_windspeed);
				}
				if ($max_windspeed < 10) {
					$max_windspeed = sprintf ('%02d', $max_windspeed);
				}
				$windspeed = $min_windspeed . 'G' . $max_windspeed;
			}
		}
		$wind = $wind_angle . $windspeed . 'KT';
	} else {
		$wind = "";
	}
	return $wind;
}

function metar2300 ($timestamp, $report_modifier) {
	require_once ('metar2300.conf.php.inc');
	$db_link = mysql_connect ($db_hostname, $db_username, $db_password);
	if (! $db_link) {
		print "Error connecting database\n";
		return "ERROR";
	} else {
		$db = mysql_select_db ($db_database, $db_link);
		$timestamp = metar2300_adjust_timestamp ($timestamp, $time_offset, $time_realtime);
		$metar_timestamp = metar2300_timestamp ($timestamp);
		$metar = "METAR $station_identifier $metar_timestamp";
		$metar_report_modifier = metar2300_report_modifier ($report_modifier);
		if (! empty ($metar_report_modifier)) {
			$metar .= " $metar_report_modifier";
		}
		$metar_wind = metar2300_wind ($timestamp);
		if (! empty ($metar_wind)) {
			$metar .= " $metar_wind";
		}
		$metar_temp_dewpoint = metar2300_temp_dewpoint ($timestamp);
		if (! empty ($metar_temp_dewpoint)) {
			$metar .= " $metar_temp_dewpoint";
		}
		$metar_pressure = metar2300_pressure ($timestamp);
		if (! empty ($metar_pressure)) {
			$metar .= " $metar_pressure";
		}
		return $metar;
	}
}

?>
