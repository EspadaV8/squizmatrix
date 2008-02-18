<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: rollback_management.php,v 1.19 2008/02/18 05:28:41 lwright Exp $
*
*/

/**
* Adds entries into rollback tables where there are no entries. This will occur
* when rollback has been enabled sometime after the system was installed.
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Revision: 1.19 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

require_once 'Console/Getopt.php';

$shortopt = 'd:p:s:q::f:';
$longopt = Array('enable-rollback', 'disable-rollback', 'reset-rollback');

$args = Console_Getopt::readPHPArgv();
array_shift($args);
$options = Console_Getopt::getopt($args, $shortopt, $longopt);

if (empty($options[0])) usage();

$PURGE_FV_DATE = '';
$ROLLBACK_DATE = '';
$SYSTEM_ROOT = '';
$ENABLE_ROLLBACK = FALSE;
$DISABLE_ROLLBACK = FALSE;
$RESET_ROLLBACK = FALSE;
$QUIET = FALSE;

foreach ($options[0] as $option) {

	switch ($option[0]) {
		case 'd':
			if (!empty($ROLLBACK_DATE)) usage();
			if (empty($option[1])) usage();
			if (!preg_match('|^\d{4}-\d{2}-\d{2}\s+\d{2}:\d{2}:\d{2}$|', $option[1])) {
				usage();
			}
			$ROLLBACK_DATE = $option[1];
		break;

		case 'p':
			if (!empty($ROLLBACK_DATE)) usage();
			if (empty($option[1])) usage();
			$matches = Array();
			if (!preg_match('|^(\d+)([hdwmy])$|', $option[1], $matches)) {
				usage();
			}

			$time_num = (int)$matches[1];
			$time_units = '';
			switch ($matches[2]) {
				case 'h' :
					$time_units = 'hour';
				break;
				case 'd' :
					$time_units = 'day';
				break;
				case 'w' :
					$time_units = 'week';
				break;
				case 'm' :
					$time_units = 'month';
				break;
				case 'y' :
					$time_units = 'year';
				break;
			}
			if ($time_num > 1) $time_units .= 's';
			$ROLLBACK_DATE = date('Y-m-d H:i:s', strtotime('-'.$time_num.' '.$time_units));
		break;

		case 'f':
			if (!empty($PURGE_FV_DATE)) usage();
			if (empty($option[1])) usage();
			$matches = Array();
			if (!preg_match('|^(\d+)([hdwmy])$|', $option[1], $matches)) {
				usage();
			}

			$time_num = (int)$matches[1];
			$time_units = '';
			switch ($matches[2]) {
				case 'h' :
					$time_units = 'hour';
				break;
				case 'd' :
					$time_units = 'day';
				break;
				case 'w' :
					$time_units = 'week';
				break;
				case 'm' :
					$time_units = 'month';
				break;
				case 'y' :
					$time_units = 'year';
				break;
			}
			if ($time_num > 1) $time_units .= 's';
			$PURGE_FV_DATE = date('Y-m-d H:i:s', strtotime('-'.$time_num.' '.$time_units));
		break;

		case 's':
			if (empty($option[1])) usage();
			if (!is_dir($option[1])) usage();
			$SYSTEM_ROOT = $option[1];
		break;

		case '--enable-rollback':
			if ($DISABLE_ROLLBACK || $RESET_ROLLBACK) {
				usage();
			}
			$ENABLE_ROLLBACK = TRUE;
		break;

		case '--disable-rollback':
			if ($ENABLE_ROLLBACK || $RESET_ROLLBACK) {
				usage();
			}
			$DISABLE_ROLLBACK = TRUE;
		break;

		case '--reset-rollback':
			if ($ENABLE_ROLLBACK || $DISABLE_ROLLBACK) {
				usage();
			}
			$RESET_ROLLBACK = TRUE;
		break;

		case 'q':
			$QUIET = TRUE;
		break;
		default:
			echo 'Invalid option - '.$option[0];
			usage();
	}//end switch

}//end foreach arguments

if ($ENABLE_ROLLBACK || $DISABLE_ROLLBACK || $RESET_ROLLBACK) {
	if (!empty($ROLLBACK_DATE) || !empty($PURGE_FV_DATE)) {
		usage();
	}
	$ROLLBACK_DATE = date('Y-m-d H:i:s');
}

if (!empty($ROLLBACK_DATE) && !empty($PURGE_FV_DATE)) {
	usage();
}

if (empty($SYSTEM_ROOT)) usage();

require_once $SYSTEM_ROOT.'/core/include/init.inc';
require SQ_DATA_PATH.'/private/db/table_columns.inc';

// get the tables from table_columns into a var
// that will not clash with other vars
$SQ_TABLE_COLUMNS = $tables;

$tables = get_rollback_table_names();

// the number of rows to limit to as to avoid an out of memory error
// MUST be greater than 1
$LIMIT_ROWS = 500;

$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
$db = MatrixDAL::getDb();

if ($PURGE_FV_DATE) {
	purge_file_versioning($PURGE_FV_DATE);
} else {

	if ($RESET_ROLLBACK) {
		// truncate toll back tables here then enable rollback
		foreach ($tables as $table) {
			truncate_rollback_entries($table);
		}
		$ENABLE_ROLLBACK = TRUE;
		echo "\nEnabling rollback...\n\n";
	}

	foreach ($tables as $table) {

		if ($ENABLE_ROLLBACK) {
			open_rollback_entries($table, $ROLLBACK_DATE);
			continue;
		}
		if ($DISABLE_ROLLBACK) {
			close_rollback_entries($table, $ROLLBACK_DATE);
			continue;
		}
		if ($ROLLBACK_DATE) {
			delete_rollback_entries($table, $ROLLBACK_DATE);
			align_rollback_entries($table, $ROLLBACK_DATE);
			continue;
		}
	}
}

$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();


/**
* Truncates rollback tables
*
* @param string	$table_name	the tablename to update
*
* @return void
* @access public
*/
function truncate_rollback_entries($table_name)
{
	global $db, $QUIET;

	$sql = 'TRUNCATE TABLE sq_rb_'.$table_name;
	try {
		$result = MatrixDAL::executeSql($sql);
	} catch (Exception $e) {
		throw new Exception('Unable to truncate table '.$table_name.' due to the following error: '.$e->getMessage());
	}//end try catch

	if (!$QUIET) {
		echo 'Rollback table sq_rb_'.$table_name." truncated.\n";
	}

}//end truncate_rollback_entries()


/**
* Closes of rollback entries to the specified date
*
* @param string	$table_name	the tablename to update
* @param string	$date		the date to close to
*
* @return void
* @access public
*/
function close_rollback_entries($table_name, $date)
{
	global $db, $QUIET;

	$sql = 'UPDATE sq_rb_'.$table_name.' SET sq_eff_to = :date WHERE sq_eff_to IS NULL';

	try {
		$query = MatrixDAL::preparePdoQuery($sql);
		$query->bindValue('date', $date);
		$result = MatrixDAL::executePdoOne($query);
	} catch (Exception $e) {
		throw new Exception('Unable to update rollback table '.$table_name.' due to the following error:'.$error->getMessage());
	}//end try catch

	$affected_rows =  $query->rowCount();

	if (!$QUIET) {
		echo $affected_rows.' ENTRIES CLOSED IN sq_rb_'.$table_name."\n";
	}

}//end close_rollback_entries()


/**
* Opens any rollback entries that have not allready been opened
*
* @param string	$table_name	the tablename to update
* @param string	$date		the date to close to
*
* @return void
* @access public
*/
function open_rollback_entries($table_name, $date)
{
	global $SQ_TABLE_COLUMNS, $db, $QUIET;

	$columns = $SQ_TABLE_COLUMNS[$table_name]['columns'];
	$sql = 'INSERT INTO sq_rb_'.$table_name.' ('.implode(', ', $columns).
		', sq_eff_from, sq_eff_to)
		SELECT '.implode(',', $columns).',:date , NULL FROM sq_'.$table_name;

	try {
		$query = MatrixDAL::preparePdoQuery($sql);
		$query->bindValue('date', $date);
		$result = MatrixDAL::executePdoOne($query);
	} catch (Exception $e) {
		throw new Exception('Unable to insert into rollback table '.$table_name.' due to the following error:'.$e->getMessage());
	}//end try catch

	$affected_rows =  $query->rowCount();

	if (!$QUIET) {
		echo $affected_rows.' ENTRIES OPENED IN sq_rb_'.$table_name."\n";
	}

}//end open_rollback_entries()


/**
* Aligns all the minimum eff_from entries in the specified rollback table so
* they all start at a specified date
*
* @param string	$table_name	the tablename to update
* @param string	$date		the date to close to
*
* @return void
* @access public
*/
function align_rollback_entries($table_name, $date)
{
	global $db, $QUIET;

	$sql = 'UPDATE sq_rb_'.$table_name.'
			SET sq_eff_from = :date1
			WHERE sq_eff_from < :date1';

	try {
		$query = MatrixDAL::preparePdoQuery($sql);
		$query->bindValue('date1', $date);
		$query->bindValue('date2', $date);
		$result = MatrixDAL::executePdoOne($query);
	} catch (Exception $e) {
		throw new Exception('Unable to update rollback table '.$table_name.' due to the following error:'.$error->getMessage());
	}//end try catch

	$affected_rows =  $query->rowCount();

	if (!$QUIET) {
		echo $affected_rows.' ENTRIES ALIGNED IN sq_rb_'.$table_name."\n";
	}

}//end align_rollback_entries()


/**
* Deletes the rollback entries that started before the specified date
*
* @param string	$table_name	the tablename to update
* @param string	$date		the date to close to
*
* @return void
* @access public
*/
function delete_rollback_entries($table_name, $date)
{
	global $db, $QUIET;

	$sql = 'DELETE FROM sq_rb_'.$table_name.'
		WHERE sq_eff_to <= :date';

	try {
		$query = MatrixDAL::preparePdoQuery($sql);
		$query->bindValue('date', $date);
		$result = MatrixDAL::executePdoOne($query);
	} catch (Exception $e) {
		throw new Exception('Unable to delete rollback table '.$table_name.' due to the following error:'.$error->getMessage());
	}//end try catch

	$affected_rows =  $query->rowCount();

	if (!$QUIET) {
		echo $affected_rows.' ENTRIES DELETED IN sq_rb_'.$table_name."\n";
	}

}//end delete_rollback_entries()


/**
* Returns the table names of the database tables that require rollback
*
* @return array
* @access public
*/
function get_rollback_table_names()
{
	global $SYSTEM_ROOT;
	$table_names = Array();

	$packages_installed = $GLOBALS['SQ_SYSTEM']->getInstalledPackages();

	if (empty($packages_installed)) return Array();

	foreach ($packages_installed as $package_array) {
		if ($package_array['code_name'] == '__core__') {
			$table_file = SQ_CORE_PACKAGE_PATH.'/tables.xml';
		} else {
			$table_file = SQ_PACKAGES_PATH.'/'.$package_array['code_name'].'/tables.xml';
		}

		if (!file_exists($table_file)) continue;

		try {
			$root = new SimpleXMLElement($table_file, LIBXML_NOCDATA, TRUE);
		} catch (Exception $e) {
			throw new Exception('Unable to parse table file : '.$table_file .' due to the following error: '.$e->getMessage());
		}//end try catch

		foreach ($root->children() as $child) {
			$first_child_name = $child->getName();
			break;
		}//end foreach
		if ($root->getName() != 'schema' || $first_child_name != 'tables') {
			trigger_error('Invalid table schema for file "'.$table_file.'"', E_USER_ERROR);
		}

		$table_root = $child;

		foreach ($table_root->children() as $table_child) {
			if ((string) $table_child->attributes()->require_rollback) {
				$table_name = (string) $table_child->attributes()->name;
				array_push($table_names, $table_name);
			}//end if
		}//end foreach
	}

	return $table_names;

}//end get_rollback_table_names()


/**
* Purge file versioning database entries and files before a certain date
*
* @param string	$date	the date to purge to
*
* @return void
* @access public
*/
function purge_file_versioning($date)
{
	global $db, $QUIET;

	$history_table = 'sq_file_vers_history';
	$file_table = 'sq_file_vers_file';

	// Get all the file versioning entries
	$SQL = 'SELECT * FROM '.$history_table.' h JOIN '.$file_table.' f ON h.fileid = f.fileid WHERE to_date <= :date';

	try {
		$query = MatrixDAL::preparePdoQuery($sql);
		$query->bindValue('date', $date);
		$result = MatrixDAL::executePdoAssoc($query);
	} catch (Exception $e) {
		throw new Exception('Unable to select from rollback table '.$table_name.' due to the following error:'.$error->getMessage());
	}//end try catch

	$count = 0;

	// Delete the files - if it isn't there then don't worry because it means the
	// DB entry shouldn't have been there in the first place
	foreach ($result as $row) {
		$ffv_file = SQ_SYSTEM_ROOT.'/data/file_repository/'.$row['path'].'/'.$row['filename'].',ffv'.$row['version'];
		unlink($ffv_file);
		$count++;
	}

	// Now delete the entries from the table
	$SQL = 'DELETE FROM '.$history_table.' WHERE to_date <= :date';

	try {
		$query = MatrixDAL::preparePdoQuery($SQL);
		$query->bindValue('date', $date);
		$result = MatrixDAL::executePdoOne($query);
	} catch (Exception $e) {
		throw new Exception('Unable to delete from rollback table '.$table_name.' due to the following error:'.$error->getMessage());
	}//end try catch

	$affected_rows = $query->rowCount();

	if (!$QUIET) {
		echo $affected_rows.' FILE VERSIONING FILES AND ENTRIES DELETED'."\n";
	}

}//end purge_file_versioning()


/**
* Prints the usage for this script and exits
*
* @return void
* @access public
*/
function usage()
{
	echo "\nUSAGE: rollback_management.php -s <system_root> [-d <date>] [-p <period>] [--enable-rollback] [--disable-rollback] [--reset-rollback] [-q --quiet]\n".
		"--enable-rollback  Enables rollback in MySource Matrix\n".
		"--disable-rollback Disables rollback in MySource Matrix\n".
		"--reset-rollback Removes all rollback information and enables rollback in MySource Matrix\n".
		"-q No output will be sent\n".
		"-d The date to set rollback entries to in the format YYYY-MM-DD HH:MM:SS\n".
		"-p The period to purge rollback entries before\n".
		"-f The period to purge file versioning entries and files before\n".
		"(For -p and -f, the period is in the format nx where n is the number of units and x is one of:\n".
		" h - hours\t\n d - days\t\n w - weeks\t\n m - months\t\n y - years\n".
		"\nNOTE: only one of [-d -p -f --enable-rollback --disable-rollback --reset-rollback] option is allowed to be specified\n";
	exit();

}//end usage()


?>
