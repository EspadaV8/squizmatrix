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
* $Id: package_manager_saml.inc,v 1.2 2012/08/30 00:59:03 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/package_manager.inc';

/**
* Package_Manager_SAML
*
*
* @author  Edison Wang <ewang@squiz.com.au>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage saml
*/
class Package_Manager_SAML extends Package_Manager
{

	/**
	* Constructor
	*
	*/
	function Package_Manager_SAML()
	{
		$this->_full_path = SQ_PACKAGES_PATH.'/saml';
		$this->Package_Manager();

	}//end constructor



}//end class

?>
