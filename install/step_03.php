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
* $Id: step_03.php,v 1.61 2005/04/06 23:15:18 mmcintyre Exp $
*
*/


/**
* Install Step 3
*
* Installs packages into the MySource system. You can optionally specify what
* packages and assets to run the script for in the following manner:
*
*    php step_03.php /system/root --package=packagename[-assettype,assettype,assettype]
*
* You may specify several --package= entries. If the packagename is followed by
* a hyphen, entries after the hyphen will be taken to be asset types.
*
*    php step_03.php /system/root --package=core-page,page_standard
*
* would only update the page and page_standard assets within the core package
*
*    php step_03.php /system/root --package=core --package=cms
*
* would update all the asset types for core and cms only
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.61 $
* @package MySource_Matrix
* @subpackage install
*/
ini_set('memory_limit', -1);
error_reporting(E_ALL);
$SYSTEM_ROOT = '';

$cli = true;

// from cmd line
if ((php_sapi_name() == 'cli')) {
	if (isset($_SERVER['argv'][1])) {
		$SYSTEM_ROOT = $_SERVER['argv'][1];
	}
	$err_msg = "You need to supply the path to the System Root as the first argument\n";

} else {
	$cli = false;
	if (isset($_GET['SYSTEM_ROOT'])) {
		$SYSTEM_ROOT = $_GET['SYSTEM_ROOT'];
	}
	$err_msg = '
	<div style="background-color: red; color: white; font-weight: bold;">
		You need to supply the path to the System Root as a query string variable called SYSTEM_ROOT
	</div>
	';
}

if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	trigger_error($err_msg, E_USER_ERROR);
}


// only use console stuff if we're running from the command line
if ($cli) {
	require_once 'Console/Getopt.php';

	$shortopt = '';
	$longopt = Array('package=');

	$con  = new Console_Getopt;
	$args = $con->readPHPArgv();
	array_shift($args);
	$options = $con->getopt($args, $shortopt, $longopt);

	if (is_array($options[0])) {
		$package_list = get_console_list($options[0]);
	}
}


require_once $SYSTEM_ROOT.'/core/include/init.inc';

// get the list of functions used during install
require_once $SYSTEM_ROOT.'/install/install.inc';

$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

// firstly let's check that we are OK for the version
if (version_compare(PHP_VERSION, SQ_REQUIRED_PHP_VERSION, '<')) {
	trigger_error('<i>'.SQ_SYSTEM_LONG_NAME.'</i> requires PHP Version '.SQ_REQUIRED_PHP_VERSION.'.<br/> You may need to upgrade.<br/> Your current version is '.PHP_VERSION, E_USER_ERROR);
}

// let everyone know we are installing
$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

// call all the steps
if (!regenerate_configs()) {
	trigger_error('Config Generation Failed', E_USER_ERROR);
}

// check if the $packageList variable has been defined at all
if (!isset($package_list)) {
	$package_list = Array();
}

uninstall_asset_types();
uninstall_packages();

// This has been changed so that the asset install is only done when search
// manager is being installed for the first time (to initialise the weightings
// of assets that are already installed - reconfigures do not need the second
// pass once Search Manager is installed, as it picks default weightings up
// automatically
$run_install = true;

while ($run_install) {
	$search_installed = $GLOBALS['SQ_SYSTEM']->am->installed('search_manager');

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

	if (!$search_installed && $GLOBALS['SQ_SYSTEM']->am->installed('search_manager')) {
		bam('Search manager was installed for the first time'."\n".'Need to restart asset install to initialise search weightings...');
	} else {
		$run_install = false;
	}
}//end while install should keep running

install_authentication_types();
generate_global_preferences();
install_event_listeners();
cache_asset_types();

$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

/**
* Gets a list of supplied package options from the command line arguments given
*
* Returns an array in the format needed for package_list
*
* @param array	$options	the options as retrieved from Console::getopts
*
* @return array
* @access public
*/
function get_console_list($options)
{
	$list = Array();

	foreach ($options as $option) {
		// if nothing set, skip this entry
		if (!isset($option[0]) || !isset($option[1])) continue;

		if ($option[0] != '--package') continue;

		// Now process the list
		$parts = split('-', $option[1]);

		$types = Array();
		if (count($parts) == 2 && strlen($parts[1])) {
			$types = split(',', $parts[1]);
		}

		$list[$parts[0]] = $types;
	}

	return $list;

}//end get_console_list()


?>
