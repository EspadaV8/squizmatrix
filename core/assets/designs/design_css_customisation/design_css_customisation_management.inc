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
* $Id: design_css_customisation_management.inc,v 1.8 2012/08/30 01:09:06 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Css_Customisation_Management
*
* @see Asset_Management
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Css_Customisation_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

	}//end constructor
	
	
	/**
	* Upgrade
	*
	* @param string	$current_version	the version of the asset that is currenly installed
	*
	* @return boolean
	* @access public
	*/
	function _upgrade($current_version)
	{
		// first do things as normal
		$res = parent::_upgrade($current_version);

		if (version_compare($current_version, '0.2', '<')) {
			pre_echo('UPGRADING DESIGN CSS CUSTOMISATION FROM v < 0.2');

			// get all design assets
			$design_ids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('design_css_customisation', TRUE, TRUE);
			foreach ($design_ids as $design_id => $type) {
				$design = $GLOBALS['SQ_SYSTEM']->am->getAsset($design_id, $type['type_code']);
				$design->setAttrValue('static_url_versioning', FALSE);
				$design->saveAttributes();
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($design);
				unset($design);
			}
			
		}//end if
		return TRUE;

	}//end _upgrade()
	

}//end class
?>
