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
* $Id: clean_old_shadow_links.php,v 1.2 2012/08/30 01:04:53 ewang Exp $
*
*/

/**
* Run php clean_old_shadow_links.php without any argument to see the script usage 
* 
* This script is used to clean the old shadow links whose minor shadow assets are
* no longer valid because their bridges can not find them anymore. It can be because
* the assets were removed from the datasource, e.g. An LDAP Backend User is linked
* under the System Administrators folder but is removed from the LDAP Directory.
* But there are also other cases like the connection between Matrix and the datasource
* goes down, or someone changes the bridge's configuration for testing. Hence, this
* script does not check and remove the links immediately but gives a list of old links
* for review and takes actions in the second run.
* You can use -s option to save a list of SQL statements to a file for reviewing and
* executing by an SQL client like psql.
* You can also use -p option to save the link list under PHP array format and then
* running with -d option to delete the links in the reviewed files.
* 
* 
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.2 $
* @package MySource_Matrix
*/


error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
	trigger_error("\nYou can only run this script from the command line.\n", E_USER_ERROR);
}

$options = getopt('m:s:p:d:');

if (empty($options)) {
	print_usage();
	exit;
}

if (!isset($options['m']) || !is_dir($options['m'])) {
	echo "\nYou need to supply the path to the System Root as the argument of -m option.
Run the script without argument to see its usage.\n\n";
	exit;
}

require_once $options['m'].'/core/include/init.inc';

// If -d option is used, do not check for other options
if (isset($options['d'])) {
	if (is_file($options['d'])) {
		delete_old_shadow_links($options['d']);
	} else {
		echo "\nYou need to supply the path to the PHP file as the argument of -d option.
Run the script without argument to see its usage.\n\n";
	}
	exit;
}

// Get old shadow links
$old_links = get_old_shadow_links();

// If there are no old shadow links, print a message and quit
if (empty($old_links)) {
	echo "\nThere are no old shadow links in the system.\n\n";
	exit;
}

// Print old shadow links
print_links($old_links);

// If the options -s, -p and -d are not used at all, print a message to remind the user
if (!isset($options['s']) && !isset($options['p'])) {
	echo '
You will need to run this script with -s or -p option to save the shadow links to files
and -d option to delete the shadow links stored in the output file generated by -p option.
(You have to use an SQL client like psql to execute the SQL statements generated by -s option)

';
} else {
	// Save SQL statements to file
	if (isset($options['s'])) {
		save_sql_statements($old_links, $options['s']);
	}
	
	// Save shadow links as array to a file
	if (isset($options['p'])) {
		save_linkid_array($old_links, $options['p']);
	}
}


/**
 * Delete old shadow links from an array of linkids in a PHP file
 * 
 * @param $file		The PHP file that stores an array of shadow linkids
 * @return void
 */
function delete_old_shadow_links($file)
{
	// Require the file to get the $linkids array
	require $file;
	
	// Check if $linkids array is defined
	if (!isset($linkids) || !is_array($linkids)) {
		echo "\nERROR: The file $file is not in a correct format.\n\n";
		return;
	}
	
	// Split the array into chunks of 100 elements
	$chunks = array_chunk($linkids, 100);
	
	// Declare total deleted links variable
	$total_deleted_links = 0;
	
	// Delete the old shadow links
	$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
	$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
	
	try {
		foreach ($chunks as $chunk) {
			$delete_linkids = Array();
			foreach ($chunk as $linkid) {
				$delete_linkids[] = MatrixDAL::quote($linkid);
			}
			
			$sql = 'DELETE FROM sq_shdw_ast_lnk WHERE linkid IN ('.implode(',', $delete_linkids).')';
			$deleted_rows = MatrixDAL::executeSql($sql);
			$total_deleted_links += $deleted_rows;
		}
		
	} catch (DALException $e) {
		$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		throw new Exception('Can not delete old shadow links due to the DB error: '.$e->getMessage());
	}
	
	$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
	$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
	
	echo "\n$total_deleted_links old shadow link(s) from $file were deleted successfully.\n\n";
	
}//end delete_old_shadow_links()


/**
 * Create a PHP array of old shadow linkids and store it in a file
 * 
 * @param $links	An array of old shadow links
 * @param $file		The file to store the PHP array
 * @return void
 */
function save_linkid_array($links, $file)
{
	// Create a big PHP string with comments in it
	$linkids = Array();
	foreach ($links as $link) {
		$linkids[] = "'".$link['linkid']."', /* link_type=".$link['link_type']
		             .", majorid=".$link['majorid'].", minorid=".$link['minorid']
		             ." */";
	}
	
	$php = '<?php
	$linkids = Array(
'.implode("\n", $linkids).'
	                );
?>
';
	
	// Save the PHP string to file
	if (file_put_contents($file, $php) === FALSE) {
		echo "\nERROR: Can not write PHP array to $file\n\n";
	} else {
		echo "\nThe PHP array to delete the old shadow links are saved to $file\n\n";
	}
	
}//end save_linkid_array()


/**
 * Create SQL statements to delete old shadow links and store them in a file.
 * 
 * @param array $links		An array of old shadow links
 * @param string $file		The file to store SQL statements
 * @return void
 */
function save_sql_statements($links, $file)
{
	// Create a big SQL string with comments in it
	$sql = '';
	foreach ($links as $link) {
		$sql .= 'DELETE FROM sq_shdw_ast_lnk WHERE linkid='.MatrixDAL::quote($link['linkid'])
		        .'; -- link_type='.$link['link_type'].', majorid='.$link['majorid']
		        .', minorid='.$link['minorid']."\n";
	}
	
	// Save the SQL string to file
	if (file_put_contents($file, $sql) === FALSE) {
		echo "\nERROR: Can not write SQL statements to $file\n\n";
	} else {
		echo "\nThe SQL statements to delete the old shadow links are saved to $file\n\n";
	}
	
}//end save_sql_statements()


/**
 * Print the list of links with 4 columns: linkid, link type, majorid, and minorid.
 * 
 * @param $links	An array of links with the format of
 * 						Array(
 * 								Array('linkid' => LINKID,
 * 								      'link_type' => TYPE,
 * 								      'majorid' => ASSETID,
 * 									  'minorid' => ASSETID,
 * 								)
 * 						)
 * @return void
 */
function print_links($links)
{
	echo "\nList of old shadow links:\n\n";
	// Create row format
	$row_format = "%7s |%5s |%8s | %-55s\n";
	$row_delimiter_format = "%'-7s-+%'-5s-+%'-8s-+%'-55s-\n";
	
	// Print the header
	printf($row_format, 'linkid', 'type', 'majorid', 'minorid');
	printf($row_delimiter_format, '', '', '', '');
	
	// Print the links
	foreach ($links as $link) {
		printf($row_format, $link['linkid'], $link['link_type'], $link['majorid'], $link['minorid']);
	}
	
	echo "(Number of links: ".count($links).")\n";
	
}//end print_links()


/**
 * Return old shadow links in the system
 * 
 * @return array	The array of old shadow links 
 */
function get_old_shadow_links()
{
	// Get all shadow links
	$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db');
	
	$sql = 'SELECT linkid, link_type, majorid, minorid FROM sq_shdw_ast_lnk ORDER BY majorid, link_type, linkid';
	$links = MatrixDAL::executeSqlAll($sql);
	
	$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
	
	// Search for old shadow links
	$am = $GLOBALS['SQ_SYSTEM']->am;
	$old_links = Array();
	foreach ($links as $link) {
		$shadow_assetid = $link['minorid'];
		$id_parts = explode(':', $shadow_assetid);
		if (isset($id_parts[1])) {
			$bridge_id = $id_parts[0];
			$bridge = $am->getAsset($bridge_id, '', FALSE);
			if (is_null($bridge) || !method_exists($bridge, 'getAsset')) {
				$old_links[] = $link;
			} else {
				//getAsset() from bridge directly so we can set the fourth param $return_null to TRUE so we don't get dummy assets.
				$shadow_asset = $bridge->getAsset($shadow_assetid, '', FALSE, TRUE);
				if (is_null($shadow_asset)) $old_links[] = $link;
			}
		} 
	}
	
	return $old_links;
	
}//end get_old_shadow_links()


/**
 * Print the usage of this script.
 * 
 * @return void
 */
function print_usage()
{
	$usage = '
This script is used to clean the old shadow links whose minor shadow assets are
no longer valid because their bridges can not find them anymore.
Usage:
          php clean_old_shadow_links.php [OPTION]
          
Options list:
	-m    Required. This option requires an argument which is the Matrix System Root.
	-s    Optional. This option requires a file path to save the SQL statements to.
	-p    Optional. This option requires a file path to save the PHP array to.
	-d    Optional. This option requires the path of a PHP file generated by
	      the -p option to delete old shadow links from the database.

Example:
    List the old shadow links:
          php scripts/clean_old_shadow_links.php -m $PWD

    Save the SQL statements to delete old shadow links to file:
          php scripts/clean_old_shadow_links.php -m $PWD -s delete_shd_links.sql

    Save the shadow links under PHP array format:
          php scripts/clean_old_shadow_links.php -m $PWD -p old_shadow_links.inc

    Delete the shadow links in the PHP file:
          php scripts/clean_old_shadow_links.php -m $PWD -d old_shadow_links.inc


';
	echo $usage;
	
}//end print_usage()


?>