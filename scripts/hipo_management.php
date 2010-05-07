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
* $Id: hipo_management.php,v 1.1 2010/03/29 23:25:06 bpearson Exp $
*
*/

/**
* Hipo Management 
*
* @author  Benjamin Pearson <bpearson@squiz.com.au>
* @version $Revision: 1.1 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
ini_set('memory_limit', '-1');
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	trigger_error("You need to supply the path to the System Root as the first argument\n", E_USER_ERROR);
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';
require_once SQ_FUDGE_PATH.'/general/datetime.inc';

// ask for the root password for the system
echo 'Enter the root password for "'.SQ_CONF_SYSTEM_NAME.'": ';
$root_password = rtrim(fgets(STDIN, 4094));

$am = $GLOBALS['SQ_SYSTEM']->am;
$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();

$root_user = $am->getSystemAsset('root_user');
if (!$root_user->comparePassword($root_password)) {
	trigger_error("The root password entered was incorrect\n", E_USER_ERROR);
	exit;
}//end if

$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user);

$source_jobs = Array();
$now = time();

$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db3');
$db = MatrixDAL::getDb();
$sql = 'SELECT source_code_name, code_name, job_type
		FROM sq_hipo_job';
try {
	$query = MatrixDAL::preparePdoQuery($sql);
	$results = MatrixDAL::executePdoAssoc($query);
} catch (Exception $e) {
	throw new Exception('Unable to get HIPO jobs due to database error: '.$e->getMessage());
}
$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

// Filter out dependants
foreach ($results as $result) {
	$source_name = array_get_index($result, 'source_code_name', '');
	$job_name = array_get_index($result, 'code_name', '');

	if (!empty($job_name) && $job_name == $source_name) {
		$source_jobs[] = $result;
	}//end if
}//end foreach

if (empty($source_jobs)) {
	echo translate('hipo_currently_no_jobs')."\n";
	exit;
}//end if

foreach ($source_jobs as $index => $job) {
	$source_code_name = array_get_index($job, 'source_code_name', '');
	$source_job_type = array_get_index($job, 'job_type', '');
	if (empty($source_code_name)) continue;
	$source_job = $hh->getJob($source_code_name);
	if (is_null($source_job)) continue;
	echo ($index+1).': ';
	echo ucwords(str_replace('_', ' ', $source_job_type));
	echo '( '.$source_job->percentDone().'% )';
	echo "\tLast Updated: ".easy_time_total($source_job->last_updated - $now, TRUE)."\n";
	unset($source_job);
}//end foreach

echo 'Enter the number of the job to change: (Press q to quit)';
$choice = rtrim(fgets(STDIN, 4094));

if (strtolower($choice) == 'q') exit;
$actual_choice = ($choice-1);
if (!isset($source_jobs[$actual_choice])) {
	echo "Incorrect entry\n";
	exit;
}//end if

echo "Options\n\tr - resume\n\tk - kill\n\tq - quit\nChoice:";
$action = rtrim(fgets(STDIN, 4094));
$action = strtolower($action);

$action = (string) $action;
switch ($action) {
	case 'r':
		// Recover/resume
		$source_code_name = array_get_index($source_jobs[$actual_choice], 'source_code_name', '');
		$source_job_type  = array_get_index($source_jobs[$actual_choice], 'job_type', '');
		if (empty($source_code_name)) exit;
		$source_job = $hh->getJob($source_code_name);
		if (is_null($source_job)) exit;
		
		echo 'Resuming HIPO Job ';
		echo ucwords(str_replace('_', ' ', $source_job_type))."\t";

		// Do itttttt
		$status = $source_job->process();
		if ($status) {
			echo '[  OK  ]';
		} else {
			echo '[  !!  ]';
		}//end if
		echo "\n";
	break;

	case 'k':
		// Kill kill kill
		$source_code_name = array_get_index($source_jobs[$actual_choice], 'source_code_name', '');
		$source_job_type  = array_get_index($source_jobs[$actual_choice], 'job_type', '');
		if (empty($source_code_name)) exit;
		$source_job = $hh->getJob($source_code_name);
		if (is_null($source_job)) exit;

		echo 'Aborting HIPO Job ';
		echo ucwords(str_replace('_', ' ', $source_job_type))."\n";
		$source_job->abort();
	break;

	case 'q':
	default:
		exit;
}//end switch

?>