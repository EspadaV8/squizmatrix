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
* $Id: locking_method_edit_fns.inc,v 1.1 2009/09/21 00:17:07 lwright Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';


/**
* Locking_Method_Edit_Fns
*
* Purpose
*
*
* @author		Luke Wright <lwright@squiz.net>
* @version		$Revision: 1.1 $
* @package		MySource_Matrix_Packages
* @subpackage	__core__
*/
class Locking_Method_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	public function __construct()
	{
		parent::__construct();

	}//end __construct()


}//end class

?>