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
* $Id: trigger_action_join_user_group_for_session.inc,v 1.1 2012/05/14 08:12:20 cupreti Exp $
*
*/


require_once SQ_SYSTEM_ROOT.'/core/attributes/parameter_map/parameter_map.inc';

/**
* Trigger_Action_Join_User_Group_For_Session
*
* Purpose
* 	Trigger Action to add the current user to a user group while the current session lasts
*
* @author  Maki Maruyama <mmaruyama@squiz.co.uk>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Join_User_Group_For_Session extends Trigger_Action
{
	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);
	}//end constructor

	
	/**
	* Get the HTML editing interface for this action. Returns the string instead of printing it
	*
	* @param array		$settings		settings that this condition saves in processInterface()
	* @param string		$prefix			unique prefix
	* @param boolean	$write_access	determines whether the interface is editable
	*
	* @return boolean
	* @access public
	*/
	public static function getInterface($settings, $prefix, $write_access=FALSE)
	{
		// we only allow user group type
		$type_codes = Array(
								'user_group' => 'D',
							);
		
		// buffer the user group asset chooser
		ob_start();
		$user_group_to_join = isset($settings['user_group_to_join']) ? $settings['user_group_to_join'] : '';
		if ($write_access) {
			asset_finder($prefix.'[user_group_to_join]', $user_group_to_join, $type_codes);
		} else if (!empty($user_group_to_join)) {
			$user_group_asset_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($user_group_to_join);
			if (!empty($user_group_asset_info[$user_group_to_join])) {
				echo '<b>'.$user_group_asset_info[$user_group_to_join]['name'].' (#'.$user_group_to_join.')</b>';
			}
		}
		$user_group_input = ob_get_clean();

		// buffer the text box for entering keywords that will be replaced with a user group (hopefully)
		ob_start();
			$keyword_target_groupid = isset($settings['keyword_target_groupid']) ? $settings['keyword_target_groupid'] : '';
			if ($write_access) {
				text_box($prefix.'[keyword_target_groupid]', $keyword_target_groupid, 30);
			} else {
				echo htmlspecialchars($keyword_target_groupid);
			}
		$keyword_target_groupid_input = ob_get_clean();

		// display both the asset chooser and the text box
		$output = translate('trigger_join_user_for_session', $user_group_input, $keyword_target_groupid_input);

		return $output;

	}//end getInterface()
	

	/**
	* Function that handles the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	*
	* @param array	&$settings		a container for any data the action might want to save
	* @param string	$request_data	array of data corresponding to this action as
	*								specified by the $prefix in getInterface.
	*
	* @return boolean
	* @access public
	*/
	public static function processInterface(&$settings, $request_data)
	{
		$settings['user_group_to_join']	    = isset($request_data['user_group_to_join']['assetid']) ? $request_data['user_group_to_join']['assetid'] : NULL;
		$settings['keyword_target_groupid'] = isset($request_data['keyword_target_groupid']) ? $request_data['keyword_target_groupid'] : '';

		return FALSE;

	}//end processInterface()


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return mixed array|boolean
	* @access public
	*/
	public static function execute($settings, &$state)
	{
		// the group id set in asset finder has the precedence
		$value = @$settings['user_group_to_join'];
		
		// if we don't have a group id in asset finder, see if we have something in the text box
		if (empty($value)) {
			$value = @$settings['keyword_target_groupid'];
			if (!empty($value)) {
				// replace global keywords
				replace_global_keywords($value);

				// replace asset keywords
				$keywords = retrieve_keywords_replacements($value);
				$replacements = Array();
				foreach ($keywords as $keyword) {
					$replacements[$keyword] = $state['asset']->getKeywordReplacement($keyword);
				}
				replace_keywords($value, $replacements);
			
				// we only allow an id of a User Group asset
				if (!is_numeric($value)) return FALSE;
				$user_group = $GLOBALS['SQ_SYSTEM']->am->getAsset($value);
				if (!$user_group || !$user_group instanceof User_Group) return FALSE;
			} else {
				// if we don't have any group set, there's nothing we need to do
				return FALSE;
			}
		}

		// get the current user id and double check it's a User asset
		$current_user = $GLOBALS['SQ_SYSTEM']->am->getAsset($GLOBALS['SQ_SYSTEM']->user->id);
		if (!$current_user || !$current_user instanceof User) {
			return FALSE;
		}

		// get user groups the current user is in
		$current_groups = $current_user->getUserGroups();
		
		// if user is not physically linked to the specifed group
		// add the current user to the specified user group that lasts only while the current session last
		if (!in_array($value, $_SESSION['sq_effective_user_groups'][$GLOBALS['SQ_SYSTEM']->user->id])) {
			$_SESSION['sq_effective_user_groups'][$GLOBALS['SQ_SYSTEM']->user->id][] = $value;
		}

		return Array(
				'group_joined' => $value,
			   );

	}//end execute()


}//end class

?>