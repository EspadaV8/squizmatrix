<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: step_03.php,v 1.48 2004/11/18 04:35:47 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


/**
* Install Step 3
*
* Purpose
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
* @subpackage install
*/
ini_set('memory_limit', -1);
error_reporting(E_ALL);
$SYSTEM_ROOT = '';

// from cmd line
if ((php_sapi_name() == 'cli')) {
	if (isset($_SERVER['argv'][1])) $SYSTEM_ROOT = $_SERVER['argv'][1];
	$err_msg = "You need to supply the path to the System Root as the first argument\n";

} else {
	if (isset($_GET['SYSTEM_ROOT'])) $SYSTEM_ROOT = $_GET['SYSTEM_ROOT'];
	$err_msg = '
	<div style="background-color: red; color: white; font-weight: bold;">
		You need to supply the path to the System Root as a query string variable called SYSTEM_ROOT
	</div>
	';
}

if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	trigger_error($err_msg, E_USER_ERROR);
}

// Dont set SQ_INSTALL flag before this include because we want
// a complete load now that the database has been created
require_once $SYSTEM_ROOT.'/core/include/init.inc';
// get the list of functions used during install
require_once $SYSTEM_ROOT.'/install/install.inc';

// Firstly let's check that we are OK for the version
if(version_compare(PHP_VERSION, SQ_REQUIRED_PHP_VERSION, '<')) {
	trigger_error('<i>'.SQ_SYSTEM_LONG_NAME.'</i> requires PHP Version '.SQ_REQUIRED_PHP_VERSION.'.<br/> You may need to upgrade.<br/> Your current version is '.PHP_VERSION, E_USER_ERROR);
}

// let everyone know we are installing
$GLOBALS['SQ_INSTALL'] = true;

// call all the steps
if (!regenerate_configs()) {
	trigger_error('Config Generation Failed', E_USER_ERROR);
}

// Check if the $packageList variable has been defined at all.
if (!isset($package_list)) {
	$package_list = Array();	//'cms'=>Array('content_type_raw_html')
}

uninstall_asset_types();
uninstall_packages();
install_core($package_list);
$deferred = install_packages($package_list);


// if there were deferred packages, try to reinstall them.
if (is_array($deferred)) {
	// try and install the deferred packages again in a loop until the result
	// package is the same as the install package, at which point we know
	// the dependency has failed.
	$deferred = install_deferred($deferred);
	if (is_array($deferred)) {
		trigger_error('The following assets could not be installed due to dependency failures (see previous warnings for details): '."\n".format_deferred_packages($deferred), E_USER_ERROR);
	}
}
install_authentication_types();
generate_global_preferences();
install_event_listeners();

// need to run the install packages twice
install_packages($package_list);
cache_asset_types();

unset($GLOBALS['SQ_INSTALL']);


?>
