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
* $Id: system_integrity_incomplete_attachments.php,v 1.1 2009/10/30 05:26:26 bpearson Exp $
*
*/

/**
* Report on the incomplete attachments that do NOT match an submission assets 
*
* Syntax: system_integrity_incomplete_attachments.php [Matrix_Root] [Action]
* 		where [ACTION] is --fix (delete the attachments) or --check (just report)
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}

$ACTION = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '';
$ACTION = ltrim($ACTION, '-');
if (empty($ACTION) || ($ACTION != 'fix' && $ACTION != 'check')) {
	echo "ERROR: No action specified\n";
	exit();
}//end if

// Define what the script will do later
$CORRECT = FALSE;
if ($ACTION == 'fix') {
	$CORRECT = TRUE;
}//end if

require_once $SYSTEM_ROOT.'/core/include/init.inc';

$form_assetids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetIds('form', FALSE);

foreach ($form_assetids as $assetid) {
	$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
	$path  = $asset->data_path;
	$path .= '/incomplete_attachments';
	$files = list_dirs($path);
	foreach ($files as $file) {
		if (strpos($file, 's') === 0) {
			// This is an incomplete submission, so check if the submission is still valid
			$incomplete_submission_assetid = substr($file, 1);
			$incomplete_submission = $GLOBALS['SQ_SYSTEM']->am->getAsset($incomplete_submission_assetid, '', TRUE);
			if (is_null($incomplete_submission)) {
				// Report only
				echo 'Form #'.$assetid.' still has some incomplete attachments for Submission #'.$incomplete_submission_assetid.'.';
				if ($CORRECT) {
					// Remove the dir, not needed
					echo ' Removing.';
					delete_directory($path.'/'.$file);
					echo ' Done.';
				}//end if
				echo "\n";
			}//end if
		}//end if
	}//end foreach
}//end foreach

?>