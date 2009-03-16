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
* $Id: session_handler_memcache_edit_fns.inc,v 1.1 2008/12/29 01:16:30 lwright Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/session_handling/session_handler/session_handler_edit_fns.inc';


/**
* Session_Handler_Memcache_Edit_Fns
*
* Purpose
*    Edit functions file for the Memcache Session Handler asset.
*    It's a bit of a waste, as session handlers don't exactly need much
*    of an interface.
*
* @author     Luke Wright <lwright@squiz.net>
* @version    $Revision: 1.1 $
* @package    MySource_Matrix
* @subpackage __core__
*/
class Session_Handler_Memcache_Edit_Fns extends Session_Handler_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


}//end class


?>