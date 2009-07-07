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
* $Id: js_api_management.inc,v 1.1 2009/07/03 04:43:49 bpearson Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Javascript API Management
*
* Purpose
*
*
* @author  Nic Hubbard <nic@zedsaid.com>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class JS_Api_Management extends Asset_Management
{


    /**
    * Constructor
    *
    */
    function __construct(Package_Manager $pm)
    {
        Asset_Management::__construct($pm);
		
		// Create a random key
		$key = rand(1000000000, 9999999999);

		$this->vars	= Array (
						'name'				=> Array(
												'added'			=> '0.0.1',
												'type'			=> 'text',
												'default'		=> '',
												'description'	=> 'The full name of the page',
												'update_fn'		=> '',
												'is_admin'		=> FALSE,
										   	   ),
						'short_name'		=> Array(
												'added'			=> '0.0.1',
												'type'			=> 'text',
												'default'		=> '',
												'description'	=> 'The short name of the page',
												'update_fn'		=> '',
												'is_admin'		=> FALSE,
										       ),
						'root_node'			=> Array(
												'added'			=> '0.0.1',
												'type'			=> 'assetid',
												'default'		=> '',
												'description'	=> 'The root node to restrict to',
												'update_fn'		=> '',
												'is_admin'		=> FALSE,
											   ),
						'api_key'			=> Array(
												'added'			=> '0.0.1',
												'type'			=> 'text',
												'default'		=> $key,
												'description'	=> 'The API Key',
												'update_fn'		=> '',
												'is_admin'		=> FALSE,
											   ),
						'get_general'		=> Array(
												'added'			=> '0.0.1',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											   ),
						'get_attributes'	=> Array(
												'added'			=> '0.0.1',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											   ),
						'set_attribute'		=> Array(
												'added'			=> '0.0.1',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											   ),
						'get_metadata'		=> Array(
												'added'			=> '0.0.1',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											   ),
						'set_metadata'		=> Array(
												'added'			=> '0.0.1',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											   ),
						'trash_asset'		=> Array(
												'added'			=> '0.0.1',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											   ),
											
					  );

	}//end constructor


}//end class
?>