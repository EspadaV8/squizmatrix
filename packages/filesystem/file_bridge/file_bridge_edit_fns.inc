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
* $Id: file_bridge_edit_fns.inc,v 1.1.1.1 2010/03/29 23:32:29 bpearson Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';

/**
* File_Bridge_Edit_Fns
*
* Purpose
*
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.1.1.1 $
* @package MySource_Matrix_Packages
* @subpackage file
*/
class File_Bridge_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function File_Bridge_Edit_Fns()
	{
		parent::__construct();

	}//end constructor


	/**
	* Prints the interface for editing filename
	*
	* @param object	&$asset	the file asset
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintFilename(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		if ($wa) {
			echo $asset->attr('name');
		} else {
			echo $asset->attr('name');
		}
		return $wa;

	}//end paintFilename()


	/**
	* Processes the interface for editing filename
	*
	* @param object	&$asset	the file asset
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processFilename(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');

		return $wa;

	}//end processFilename()


}//end class

?>