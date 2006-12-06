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
* $Id: reset_root_password.php,v 1.4 2006/12/06 05:39:51 bcaldwell Exp $
*
*/

/**
* Reset the root users password back to 'root'
*
* @author  Blair Robertson <brobertson@squiz.co.uk>
* @version $Revision: 1.4 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	exit();
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

// Get the root user
$root_user = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
// log in as root :P
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	trigger_error("Failed login in as root user\n", E_USER_ERROR);
}

// try to lock the root user
if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($root_user->id, 'attributes')) trigger_error("Couldn't get lock\n", E_USER_ERROR);

$current_run_level = $GLOBALS['SQ_SYSTEM']->getRunLevel();
$GLOBALS['SQ_SYSTEM']->setRunLevel($current_run_level - SQ_SECURITY_PASSWORD_VALIDATION);

if (!$root_user->setAttrValue('password', 'root')) trigger_error("Couldn't set password\n", E_USER_ERROR);
if (!$root_user->saveAttributes()) trigger_error("Couldn't save attributes \n", E_USER_ERROR);

$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

$GLOBALS['SQ_SYSTEM']->am->releaseLock($root_user->id, 'attributes');

echo 'Root User Password now reset to "root", please login and change.', "\n";

?>
