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
* $Id: link_management.inc,v 1.1 2009/07/28 17:35:53 ata Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Link_Management
*
* @see Asset_Management
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Link_Management extends Asset_Management
{


	/**
	* Constructor
	*
	* @param Package_Manager	$pm	The package manager relevant to this particular asset
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars = Array(
						'name'				=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'link_url'			=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'link_asset_id'		=> Array(
												'added'		=> '0.1',
												'type'		=> 'assetid',
												'default'	=> '',
											   ),
						'link_relation'		=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
					  );
		
	}//end constructor


}//end class
?>