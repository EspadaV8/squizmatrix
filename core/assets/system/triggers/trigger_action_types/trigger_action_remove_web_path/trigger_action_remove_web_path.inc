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
* $Id: trigger_action_remove_web_path.inc,v 1.4 2012/08/30 01:09:13 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';

/**
* Trigger_Action_Remove_Web_Path
*
* A trigger action that removes a webpath from an asset
*
*
* @author  Edison Wang <ewang@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Remove_Web_Path extends Trigger_Action
{


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* The settings used by this action are in the form:<BR />
	* <PRE>
	* Array(
	*       'path' => string // the web path to remove
	*		'remove_all' => 1|0 // remove all webpath?
	*		'update_children' => 1|0 update lookups for children?
	*       );
	* </PRE>
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return mixed array|boolean
	* @access public
	*/
	public static function execute($settings, &$state)
	{

		// check required settings
		if (empty($settings['path']) && empty($settings['remove_all'])) return FALSE;

		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		if (is_null($state['asset'])) return FALSE;

		// replace keywords
		$web_path = self::replaceKeywordsInWebPath($settings['path'], $state['asset']);
		$valid_paths = make_valid_web_paths(Array($web_path));
		$web_path = array_shift($valid_paths);



		// remove web path
		$web_paths_old = $web_paths_new = $state['asset']->getWebPaths();

		foreach ($web_paths_new as $key => $path){
			if($path === $web_path){
				unset ($web_paths_new[$key]);
			}
		}

		// remove all  paths, if needed
		if (!empty($settings['remove_all'])) {
			$web_paths_new = Array();
		}

		// only save the new paths if they have changed
		// (note that lookups are updated by the saveWebPaths fn)
		if ($web_paths_new !== $web_paths_old) {
			if (!$state['asset']->saveWebPaths($web_paths_new)) {
				return FALSE;
			}
		}

		//update lookups for children
		if(!empty($settings['update_children'])){
			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$vars = Array('assetids' => Array($state['asset']->id));
			$lookup_errors = $hh->freestyleHipo('hipo_job_update_lookups', $vars);
			if (!empty($lookup_errors)) {
				return FALSE;
			}
		}

		return Array(
				'assetid'	=> $state['asset']->id
			   );

	}//end execute()


	/**
	* Replaces all the keywords in a string.
	* Global keywords are also replaced.
	* All other keywords will refer to the event asset.
	* (think about moving this function to Trigger_Action)
	*
	* @param string		$web_path		a web path (can contain keywords %xxx%)
	* @param Asset		$event_asset	the event asset
	*
	* @return string
	* @access public
	*/
	public static function replaceKeywordsInWebPath($web_path, Asset $event_asset)
	{
		// replace global keywords
		replace_global_keywords($web_path);

		// replace event asset keywords
		$keywords = retrieve_keywords_replacements($web_path);
		$replacements = Array();
		foreach ($keywords as $keyword) {
			$replacements[$keyword] = $event_asset->getKeywordReplacement($keyword);
		}
		replace_keywords($web_path, $replacements);

		return $web_path;

	}//end replaceKeywordsInWebPath()


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
		ob_start();

		$web_path = array_get_index($settings, 'path', '');
		echo translate('Remove web path').': ';

		if ($write_access) {
			text_box($prefix.'[path]', $web_path, 40);
		} else {
			echo '<b>"'.$web_path.'"</b>';
		}
		echo '<br />';
		echo '<div class="sq-backend-section-note bottom-margin">'.translate('Note that you can use keywords (eg. %asset_assetid%) and global keywords (eg. %globals_user_assetid%) in the web path.').'</div>';

		check_box($prefix.'[remove_all]', '1', array_get_index($settings, 'remove_all', '0'), '', ($write_access ? '' : 'disabled="disabled"'));
		echo '<label for='.$prefix.'[remove_all]'.'>'.translate('Delete all webpaths').'</label>';

		echo '<br />';
		check_box($prefix.'[update_children]', '1', array_get_index($settings, 'update_children', '0'), '', ($write_access ? '' : 'disabled="disabled"'));
		echo '<label for='.$prefix.'[update_children]'.'>'.translate('Update lookups for children').'</label>';

		return ob_get_clean();

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
		$web_path = array_get_index($request_data, 'path', '');
		$web_path = trim($web_path);
		$remove_all =  array_get_index($request_data, 'remove_all', '');
		$update_children =  array_get_index($request_data, 'update_children', '');

		if ($web_path == '' && $remove_all == '') {
			return translate('Web Path not specified');

		}
		$settings['path'] = $web_path;
		$settings['remove_all'] = $remove_all;
		$settings['update_children'] = $update_children;

		return FALSE;

	}//end processInterface()


	/**
	* Get the list of locks that the action needs acquire before executing
	*
	* @param array	$settings	settings that this condition saves in processInterface()
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return array
	* @access public
	*/
	public static function getLocks($settings, &$state)
	{
		return Array($state['assetid'] => Array('lookups'));

	}//end getLocks()


}//end class

?>
