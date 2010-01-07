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
* $Id: test_orphan_assets.inc,v 1.2 2009/12/15 00:16:59 bpearson Exp $
*
*/

/**
* Test_Orphan_Asset
*
*	A test to run to find orphaned assets on the system
*
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix
*/
class Test_Orphan_Assets
{


	/**
	 * Return the name of this test
	 *
	 * @return string
	 * @access public
	 */
	public static function getName()
	{
		return 'Orphaned Assets Test';

	}//end getName()


	/**
	 * Return a description of this test
	 *
	 * @return string
	 * @access public
	 */
	public static function getDescription()
	{
		return 'A test to check for orphaned assets in Matrix';

	}//end getDescription()


	/**
	 * The test to run
	 *
	 * @param array	$messages	The variable to 'chuck'	all the messages
	 * @param array	$errors		The variable to 'chuck'	all the verbose messages
	 *
	 * @return boolean|int
	 * @access public
	 */
	public static function test(&$messages, &$errors)
	{
		$status = TRUE;
		$sql = 'SELECT assetid FROM sq_ast WHERE assetid NOT IN (SELECT minorid AS assetid FROM sq_ast_lnk WHERE link_type < 8);';
		$result = MatrixDAL::executeSqlAssoc($sql);
		if (!empty($result)) {
			$status = FALSE;
			$messages[] = 'There are '.count($result).' orphaned asset(s) on the system';
			foreach ($result as $asset) {
				$errors[] = 'Asset #'.$asset['assetid'].' is orphaned on the system';
			}//end foreach
		}//end if

		return $status;

	}//end test()

}//end class
?>