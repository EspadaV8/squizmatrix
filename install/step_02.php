<?php

if (empty($_GET['SYSTEM_ROOT']) || !is_dir($_GET['SYSTEM_ROOT'])) {
	?>
		<div style="background-color: red; color: white; font-weight: bold;">
			You need to supply the path to the Resolve System as a query string variable called SYSTEM_ROOT		
		</div>
	<?php
	exit(1);
}


require_once $_GET['SYSTEM_ROOT'].'/core/include/init.inc';
$GLOBALS['SQ_SYSTEM']->am = new Asset_Manager();

/* INSTALL CORE */

// Create any necessary sequences
$GLOBALS['SQ_SYSTEM']->db->createSequence('sq_sequence_asset');
$GLOBALS['SQ_SYSTEM']->db->createSequence('sq_sequence_asset_link');
$GLOBALS['SQ_SYSTEM']->db->createSequence('sq_sequence_asset_attribute');
$GLOBALS['SQ_SYSTEM']->db->createSequence('sq_sequence_asset_url');

require_once(SQ_INCLUDE_PATH.'/package_manager.inc');
$pm = new Package_Manager('__core__');
if (!$pm->updatePackageDetails()) exit(1);

// Firstly let's create some Assets that we require to run

$root_folder = &$GLOBALS['SQ_SYSTEM']->am->getAsset(1, 'root_folder', true);
// if there is no root folder assume that this section hasn't been run
if (is_null($root_folder)) {

	$GLOBALS['SQ_SYSTEM']->am->includeAsset('root_folder');
	$root_folder = new Root_Folder();
	if (!$root_folder->create()) die();
	pre_echo('Asset Id : '.$root_folder->id);
	if ($root_folder->id != 1) {
		trigger_error('Major Problem: The new Root Folder Asset was not given assetid #1. This needs to be fixed by You, before the installation/upgrade can be completed', E_USER_ERROR);
	}

	$GLOBALS['SQ_SYSTEM']->am->includeAsset('root_user');
	$root_user = new Root_User();
	if (!$root_user->create('root@'.$_SERVER['HTTP_HOST'])) die();
	pre_echo('Root User Asset Id : '.$root_user->id);


	$GLOBALS['SQ_SYSTEM']->am->includeAsset('trash_folder');
	$trash_folder = new Trash_Folder();
	if (!$trash_folder->create()) die();
	$root_folder->createLink($trash_folder, SQ_LINK_EXCLUSIVE);
	pre_echo('Trash Asset Id : '.$trash_folder->id);

	// Now just create some useful folders 
	$users_folder = new Folder();
	if (!$users_folder->create('Users')) die();
	$root_folder->createLink($users_folder, SQ_LINK_UNITE);
	$users_folder->createLink($root_user,   SQ_LINK_UNITE);

}// end if

exit();


/* INSTALL PACKAGES */

// Right now that we have sorted all that out we can install the packages
$d = dir(SQ_PACKAGES_PATH);
while (false !== ($entry = $d->read())) {
	if ($entry == '.' || $entry == '..') continue;
	# if this is a directory, process it
	if (is_dir(SQ_PACKAGES_PATH.'/'.$entry)) {
		$pm = new Package_Manager($entry);
		if ($pm->package) {
			$result = $pm->updatePackageDetails();
		}
	}
}
$d->close();


?>
