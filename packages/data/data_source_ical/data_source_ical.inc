<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: data_source_ical.inc,v 1.1 2009/08/16 23:05:21 bpearson Exp $
*
*/

check_ssv_licence();
require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once SQ_CORE_PACKAGE_PATH.'/data_source/data_source/data_source.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_FUDGE_PATH.'/general/datetime.inc';

/**
* Data_Source_iCal
*
* Purpose
*
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage data
*/
class Data_Source_iCal extends Data_Source
{


	/**
	* Constructor
	*
	* @param int	$assetid		assetid for the asset
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	* In the form
	*
	*   Array('[link_type]' => Array('[type_code]' => Array('card' => [cardinality], 'exclusive' => [exclusive])));
	* Where:
	*   link_type   = SQ_LINK_TYPE_1|SQ_LINK_TYPE_2|SQ_LINK_TYPE_3|SQ_LINK_NOTICE
	*   cardinality = [max number of links]|'M'
	*   exclusive   = TRUE|FALSE
	*
	* @return array
	* @access private
	*/
	function _getAllowedLinks()
	{
		// any link is allowed
		$allowed_link['asset']['card'] = 'M';
		$allowed_link['asset']['exclusive'] = FALSE;

		$links[SQ_LINK_TYPE_1] = $allowed_link;
		$links[SQ_LINK_TYPE_2] = $allowed_link;
		$links[SQ_LINK_TYPE_3] = $allowed_link;
		$links[SQ_LINK_NOTICE] = $allowed_link;

		return $links;

	}//end _getAllowedLinks()


	/**
	* Returns an array of the result of query execution
	*
	* @return array
	* @access public
	*/
	function &getResultSet()
	{
		$result = Array();
		$result = parent::getResultSet($this->name);
		if (empty($result)) {
				if (empty($result)) $result = $this->getItems();
				if (!empty($result)) {
					parent::setResultSet($result, $this->name);
				}//end if
		}//end if

		return $result;

	}//end getResultSet()


	/**
	* Function to read the iCal feed and return its results 
	*
	* @param string	$path	the path to the iCal feed
	*
	* @return array
	* @access public
	*/
	function getItems()
	{
		$result = Array();
		$file = $this->attr('path');
		if ((strpos($file, 'http://') === 0) || (strpos($file, 'ftp://') === 0)) {
			$file = str_replace(' ', '%20', $file);
		}//end if
		$ical = open_socket_connection($file, $errno, $errstr);
		$results = Array();
		$startCalendar = FALSE;
		$startEvent = FALSE;
		$index = 0;

		if ($ical) {
			while (!feof($ical)) {
				$line = fgets($ical, 4096);
				$line = trim($line);
				$matches = Array();
				$pattern = '/([^;:]*)([^:]*):(.*)/is';
				$status = preg_match($pattern, $line, $matches);
				$command = array_get_index($matches, 1, '');
				$parameter = array_get_index($matches, 3, '');

				if (!empty($command)) {
					switch ($command) {
						case 'BEGIN':
							if ($startCalendar) {
								// We are in the calendar, so allow us to begin events etc.
								if ($parameter == 'VEVENT') {
									$startEvent = TRUE;
									$results[$index] = Array();
								}//end if
							} else {
								// Not in calendar
								if ($parameter == 'VCALENDAR') {
									$startCalendar = TRUE;
								}//end if
							}//end if
						break;
						case 'END':
							if ($parameter == 'VEVENT') {
								$startEvent = FALSE;
								$index++;
							} else if ($parameter == 'VCALENDAR') {
								$startCalendar = TRUE;
							}//end if
						break;
						default:
							if ($startEvent) {
								switch ($command) {
									case 'DTEND':
									case 'DTSTAMP':
									case 'DTSTART':
									case 'CREATED':
									case 'LAST-MODIFIED':
										$date_str = $parameter;
										if (strlen($parameter) == 8) {
											$date_str .= 'T------';
										}//end if
										$value  = substr($date_str, 0, 4);		// Year
										$value .= '-'.substr($date_str, 4, 2);	// Month
										$value .= '-'.substr($date_str, 6, 2);	// Day
										$value .= 'T'.substr($date_str, 9, 2);	// Hour
										$value .= ':'.substr($date_str, 11, 2);	// Minute
										$value .= ':'.substr($date_str, 13, 2);	// Second
										if (substr($date_str, 15, 1) === 'Z') {
											$date_time = new DateTime($date_str);
											$timezone = timezone_open(SQ_CONF_TIMEZONE);
											$date_time->setTimezone($timezone);
											$value = $date_time->format('jS M Y g:ia');
										} else {
											$value = readable_datetime(iso8601_ts($value));
										}//end if
									break;
									default:
										$value = $parameter;
								}//end switch
								$results[$index][strtolower($command)] = $value;
								if ($command == 'SUMMARY') {
									$results[$index]['title'] = ellipsisize($value, 20);
								}//end if
							}//end if
					}//end switch
				}//end if
			}//end while
			fclose($ical);
		}//end if

		return $results;

	}//end getItems()


}//end class

?>