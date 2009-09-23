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
* $Id: trigger_action_call_rest_resource_edit_fns.inc,v 1.1 2009/09/23 07:13:30 akarelia Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action_edit_fns.inc';

/**
* Trigger_Action_Call_Rest_Resource_Edit_Fns
*
* @author  Maki Maruyama <mmaruyama@squiz.co.uk>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage rest
*/
class Trigger_Action_Call_Rest_Resource_Edit_Fns extends Trigger_Action_Edit_Fns
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