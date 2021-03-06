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
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/conditions/condition/condition.inc';
require_once dirname(__FILE__).'/condition_entry_page_edit_fns.inc';


/**
* Condition_Entry_Page
*
* Purpose
*	This condition compares the URLs/assetids against the very first page visited in the session
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition_Entry_Page extends Condition
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
	* Evaluate this condition
	*
	* @param Asset	$asset			the asset to evaluate against
	* @param array	$condition_data	the condition details to use when evaluating
	*
	* @return boolean
	* @access public
	*/
	public static function evaluate(Asset $asset, Array $condition_data)
	{
		// Get the "entry url" from the asset
		$entry_url = isset($_SESSION['entry_url']) ? $_SESSION['entry_url'] : '';
		$entry_assetid = isset($_SESSION['entry_asset']) ? $_SESSION['entry_asset'] : '';

		// Compare assetid first
		$assetids = isset($condition_data['assetids']) ? $condition_data['assetids'] : Array();
		$matched = $entry_assetid && in_array($entry_assetid, $assetids);

		// If assetid didn't matched, try comparing URL
		if (!$matched && !empty($entry_url)) {
			$urls = isset($condition_data['url_patterns']) ? $condition_data['url_patterns'] : Array();
			if (!empty($urls)) {
				foreach($urls as $url_regex) {
					$url_regex = str_replace('/', '\/', $url_regex);
					if (@preg_match('/'.$url_regex.'/i', $entry_url)) {
						$matched = TRUE;
						break;
					}
				}//end foreach
			}//end if
		}//end if

		return $matched;

	}//end evaluate()


	/**
	* Get a list of match keywords
	*
	* @param string	$type_code	the type code of this asset
	*
	* @return array
	* @access public
	* @static
	*/
	public static function getMatchKeywords()
	{
		$match_options = Array(
							'1'	=> translate('The entry page matches any of the following URL patterns or asset ids'),
							'0' => translate('The entry page does NOT match any of the following URL patterns or asset ids'),
						 );

		return $match_options;

	}//end getMatchKeywords()


}//end class

?>
