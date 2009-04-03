<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600													   |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: This Module is not available under an open source	   |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this	   |
* | Module if you have the written consent of Squiz.				   |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: soap_api_design_lookup_service_management.inc,v 1.1 2009/04/03 00:16:23 hnguyen Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Soap_Api_Design_Lookup_Service_Management
*
* @see Asset_Management
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/

class Soap_Api_Design_Lookup_Service_Management extends Asset_Management
{


    /**
    * Constructor
    *
    */
    function __construct(Package_Manager $pm)
    {
        Asset_Management::__construct($pm);

		$this->vars	= Array (
						'function_list'	=> Array (
											'added'			=> '0.1',
											'type'			=> 'serialise',
											'default'		=> Array (),
											'description'	=> '',
											'is_admin'		=> TRUE,
										   ),
					  );

	}//end constructor


}//end class
?>