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
* $Id: step_04.php,v 1.1 2005/03/30 00:40:18 lwright Exp $
*
*/


/**
* Install Step 4
*
* Compiles languages on the system
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.1 $
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


// dont set SQ_INSTALL flag before this include because we want
// a complete load now that the database has been created
require_once $SYSTEM_ROOT.'/core/include/init.inc';
// get the list of functions used during install
require_once $SYSTEM_ROOT.'/install/install.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';
require_once 'XML/Tree.php';

// firstly let's check that we are OK for the version
if (version_compare(PHP_VERSION, SQ_REQUIRED_PHP_VERSION, '<')) {
	trigger_error('<i>'.SQ_SYSTEM_LONG_NAME.'</i> requires PHP Version '.SQ_REQUIRED_PHP_VERSION.'.<br/> You may need to upgrade.<br/> Your current version is '.PHP_VERSION, E_USER_ERROR);
}

// let everyone know we are installing
// $GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
$GLOBALS['SQ_INSTALL'] = true;

$string_locales = Array();
$asset_screen_dir = SQ_DATA_PATH.'/private/asset_types/asset/localised_screens';
create_directory($asset_screen_dir);

// do it for each asset type ...
$asset_types = $GLOBALS['SQ_SYSTEM']->am->getAssetTypes();

// ... but also, give the asset type array a little base asset injection
array_unshift($asset_types, Array(
								  'type_code' => 'asset',
								  'dir'       => 'core/include/asset_edit',
								  'name'      => 'Base Asset',
								  ));

echo 'Compiling localised edit interfaces...'."\n";

foreach ($asset_types as $asset_type) {

	$type_code = $asset_type['type_code'];

	$local_screen_dir = SQ_DATA_PATH.'/private/asset_types/'.$type_code.'/localised_screens';
	$matches = Array();
	$screens = Array();

	$base_path = SQ_SYSTEM_ROOT.'/'.$asset_type['dir'].'/locale';
	$dirs_to_read = Array($base_path);

	while (!empty($dirs_to_read)) {
		$dir_read = array_shift($dirs_to_read);

		$d = @opendir($dir_read);
		if ($d) {

			// work out the locale name by taking the directory and replacing
			// the slashes with the appropriate underscore and (possibly) at sign
			$locale_name = str_replace($base_path.'/', '', $dir_read);
			if (($slash_pos = strpos($locale_name, '/')) !== false) {
				$locale_name{$slash_pos} = '_';
				if (($slash_pos = strpos($locale_name, '/')) !== false) {
					$locale_name{$slash_pos} = '@';
				}
			}

			while (false !== ($entry = readdir($d))) {
				if (($entry == '..') || ($entry == '.') || ($entry == 'CVS')) continue;

				if (is_dir($dir_read.'/'.$entry)) {
					$dirs_to_read[] = $dir_read.'/'.$entry;
				}

				if (preg_match('|lang\_((static_)?screen\_.*)\.xml|', $entry, $matches)) {
					if (!isset($screens[$locale_name])) {
						$screens[$locale_name] = Array();
					}

					$screens[$locale_name][] = Array(
													 'dir'    => $dir_read,
													 'screen' => $matches[1],
													 );
				} elseif (preg_match('|lang\_strings\.xml|', $entry, $matches)) {
					list($country,$lang,$variant) = $GLOBALS['SQ_SYSTEM']->lm->getLocaleParts($locale_name);
					if (!in_array($country, $string_locales)) {
						$string_locales[] = $country;
					}

					if (!empty($lang)) {
						if (!in_array($country.'_'.$lang, $string_locales)) {
							$string_locales[] = $country.'_'.$lang;
						}

						if (!empty($variant)) {
							if (!in_array($country.'_'.$lang.'@'.$variant, $string_locales)) {
								$string_locales[] = $country.'_'.$lang.'@'.$variant;
							}
						}
					}
				}
			}
			closedir($d);

		}
	}

	$all_screens = Array();
	$d = @opendir(SQ_SYSTEM_ROOT.'/'.$asset_type['dir']);
	if ($d) {

		while (false !== ($entry = readdir($d))) {
			if (preg_match('|edit\_interface\_((static_)?screen\_.*).xml|', $entry, $matches)) {
				$all_screens[] = $matches[1];
			}
		}
		closedir($d);

	}

	if (!empty($all_screens)) {
		echo $asset_type['type_code'].' ('.$asset_type['name'].')';
	}

	if (!empty($screens)) {
		foreach ($screens as $locale => $locale_screens) {
			foreach ($locale_screens as $screen_type) {

				if (!file_exists($local_screen_dir)) {
					create_directory($local_screen_dir);
				}

				if (strpos($screen_type['screen'], 'static_') === 0) {
					$screen_xml = build_localised_static_screen($type_code, $screen_type['screen'], $locale);
				} else {
					$screen_xml = build_localised_screen($type_code, $screen_type['screen'], $locale);
				}

				string_to_file(serialize($screen_xml), $local_screen_dir.'/'.$screen_type['screen'].'.'.$locale);
				echo '.';
			}
		}
	}

	if (!empty($all_screens)) {
		echo "\n";
	}

}//foreach asset_type

foreach ($string_locales as $locale) {
	echo 'Compiling strings for locale '.$locale."\n";
	build_locale_string_file($locale);
}

// $GLOBALS['SQ_SYSTEM']->restoreRunLevel()
unset($GLOBALS['SQ_INSTALL']);
