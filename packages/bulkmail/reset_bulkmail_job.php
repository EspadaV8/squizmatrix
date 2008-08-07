#!/usr/bin/php
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
* $Id: reset_bulkmail_job.php,v 1.2 2008/08/07 04:11:46 bpearson Exp $
*
*/

/**
* Add a bulkmail job to the bulkmail queue
*
* Usage: php schedule_bulkmail_job.php /path/to/system/root/ [assetid of job]
*		This script adds a bulkmail job to the bulkmail queue.
*		Used to allow cron systems to handle bulkmail jobs.
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix
*/

// Check for environment/arguments
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$system_root = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($system_root) || !is_dir($system_root)) {
    trigger_error("You need to supply the path to the System Root as the first argument\n", E_USER_ERROR);
}

$asset_id = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '';
if (empty($asset_id)) {
    trigger_error("You need to supply the asset id of the bulkmail job as the second argument\n", E_USER_ERROR);
}

require_once $system_root.'/core/include/init.inc';

$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user);

// Check for valid asset
$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset_id);
if (is_null($asset) || (!($asset instanceof Bulkmail_Job))) {
    trigger_error("You need to provide a valid bulkmail job to process\n", E_USER_ERROR);
}//end if

// Process
$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
$vars = Array(
			'assetid'			=> $asset_id,
			'new_status'		=> SQ_STATUS_UNDER_CONSTRUCTION,
			'dependants_only'	=> TRUE
		);
if ($asset->status == SQ_STATUS_LIVE) {
	$errors = $hh->freestyleHipo('hipo_job_edit_status', $vars);
}

// Check for errors, and show if found
if (!empty($errors)) {
	echo print_r($errors, TRUE);
}

?>
