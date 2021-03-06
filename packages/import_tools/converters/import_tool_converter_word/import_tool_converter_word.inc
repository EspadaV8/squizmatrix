<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: import_tool_converter_word.inc,v 1.8 2012/08/30 00:58:50 ewang Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/import_tools/converters/import_tool_converter/import_tool_converter.inc';

/**
* Import_Tool_Converter_Word
*require_once SQ_PACKAGES_PATH.'/import_tools/converters/import_tool_converter/import_tool_converter.inc';
* Purpose
*
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage import_tools
*/
class Import_Tool_Converter_Word extends Import_Tool_Converter
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Import_Tool_Converter_Word($assetid=0)
	{
		$this->Import_Tool_Converter($assetid);

	}//end constructor


	/**
	* Perform any pre-validation required during the creation of this asset
	*
	* Import_Tool_Converter_Words ensure a name has been set before they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	function _preCreateCheck(&$link)
	{
		if (!parent::_preCreateCheck($link)) return FALSE;

		$name = trim($this->attr('name'));
		if ($name == '') {
			trigger_localised_error('CORE0083', sprintf(translate('Unable to create a %s with a blank name'), $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name')), E_USER_WARNING);
			return FALSE;
		}

		return TRUE;

	}//end _preCreateCheck()


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Import_Tool_Converter_Words set an initial web path when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	function _createAdditional(&$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;

		// set a web path if we are linking this folder somewhere
		// NOTE: the main time we wont want to do this is if we are creating
		// the root folder as it is not linked anywhere
		if (!empty($link)) {
			// set an initial web path
			$initial_path = strtolower($this->attr('name'));
			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
			$valid_paths = make_valid_web_paths(Array($initial_path));
			$good_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($link['asset'], $valid_paths, $this->id, TRUE);
			if (!$this->saveWebPaths($good_paths)) return FALSE;
		}
		return TRUE;

	}//end _createAdditional()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	function _getName($short_name=FALSE)
	{
		return $this->attr('name');

	}//end _getName()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{

		return Array(
				SQ_LINK_TYPE_1	=> Array(
									'file'	=> Array(
												'card'		=> 'M',
												'exclusive'	=> FALSE,
											   ),
								   ),
				SQ_LINK_TYPE_2	=> Array(
									'file'	=> Array(
												'card'		=> 'M',
												'exclusive'	=> FALSE,
											   ),
								   ),
			   );

	}//end _getAllowedLinks()


}//end class
?>
