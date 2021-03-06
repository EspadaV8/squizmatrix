<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: system_integrity_internal_links.php,v 1.7 2012/10/05 07:20:38 akarelia Exp $
*
*/

/**
* Go through all WYSIWYG content types are ensure all ./?a=xx links are in the correct format
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	exit();
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

$ROOT_ASSETID = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '1';
if ($ROOT_ASSETID == 1) {
	echo "\nWARNING: You are running this integrity checker on the whole system.\nThis is fine but:\n\tit may take a long time; and\n\tit will acquire locks on many of your assets (meaning you wont be able to edit content for a while)\n\n";
}

$root_user = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');

// log in as root
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	echo "Failed loggin in as root user\n";
	exit();
}

// go trough each wysiwyg in the system, lock it, validate it, unlock it
$wysiwygids = $GLOBALS['SQ_SYSTEM']->am->getChildren($ROOT_ASSETID, 'content_type_wysiwyg', false);
foreach ($wysiwygids as $wysiwygid => $type_code_data) {
	$type_code = $type_code_data[0]['type_code'];
	$wysiwyg = &$GLOBALS['SQ_SYSTEM']->am->getAsset($wysiwygid, $type_code);
	printWYSIWYGName('WYSIWYG #'.$wysiwyg->id);

	// try to lock the WYSIWYG
	if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($wysiwyg->id, 'attributes')) {
		printUpdateStatus('LOCK');
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($wysiwyg);
		continue;
	}

	$old_html = $wysiwyg->attr('html');
	$new_html = preg_replace('|http[s]?://[^\s]+(\?a=[0-9]+)|', './\\1', $old_html);
	if (!$wysiwyg->setAttrValue('html', $new_html) || !$wysiwyg->saveAttributes()) {
		printUpdateStatus('FAILED');
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($container);
		continue;
	}

	// try to unlock the WYSIWYG
	if (!$GLOBALS['SQ_SYSTEM']->am->releaseLock($wysiwyg->id, 'attributes')) {
		printUpdateStatus('!!');
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($wysiwyg);
		continue;
	}

	printUpdateStatus('OK');
	$GLOBALS['SQ_SYSTEM']->am->forgetAsset($wysiwyg);

}//end foreach


/**
* Prints the name of the WYSIWYG as a padded string
*
* Padds to 30 columns
*
* @param string	$name	the name of the container
*
* @return void
* @access public
*/
function printWYSIWYGName($name)
{
	printf ('%s%'.(30 - strlen($name)).'s', $name, '');

}//end printWYSIWYGName()


/**
* Prints the status of the container integrity check
*
* @param string	status	the status of the check
*
* @return void
* @access public
*/
function printUpdateStatus($status)
{
	echo "[ $status ]\n";

}//end printUpdateStatus()


?>
