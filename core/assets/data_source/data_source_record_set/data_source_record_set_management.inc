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
* $Id: data_source_record_set_management.inc,v 1.4 2012/08/30 01:09:05 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Data_Source_Record_Set_Management
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage core
*/
class Data_Source_Record_Set_Management extends Asset_Management
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

	}//end constructor


}//end class

?>
