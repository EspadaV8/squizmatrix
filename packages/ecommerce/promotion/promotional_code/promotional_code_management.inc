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
* $Id: promotional_code_management.inc,v 1.1 2010/08/19 10:35:40 ata Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Promotional_Code_Management
*
* @see Asset_Management
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Promotional_Code_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars = Array(
						'name'			=> Array(
											'added'				=> '0.1',
											'type'				=> 'text',
											'default'			=> '',
											'description'		=> 'The full name of the promotional code',
											'is_admin'			=> FALSE,
											'is_contextable'	=> TRUE,
										   ),
						'code'			=> Array(
											'added'				=> '0.1',
											'type'				=> 'text',
											'default'			=> '',
											'description'		=> 'The promotional code string',
											'is_admin'			=> FALSE,
											'is_contextable'	=> FALSE,
										   ),
						'rules'			=> Array(
											'added'				=> '0.1',
											'type'				=> 'serialise',
											'default'			=> Array(),
											'description'		=> 'The promotional rules applied to this promotional code',
											'is_admin'			=> TRUE,
											'is_contextable'	=> FALSE,
										   ),
					  );

	}//end constructor


}//end class

?>