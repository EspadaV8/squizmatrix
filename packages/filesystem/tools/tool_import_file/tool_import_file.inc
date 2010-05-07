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
* $Id: tool_import_file.inc,v 1.2 2010/03/30 00:44:14 bpearson Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/assets/system/tool/tool.inc';

/**
* Tool_Import_File
*
* Purpose
*
*    Tool for importing a file/folder from under a file bridge
*
* @author Benjamin Pearson <bpearson@squiz.com.au>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage file
*/

class Tool_Import_File extends Tool
{


	/**
	* Constructor
	*
	* @param int  $assetid  the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Paints the tool interface.
	*
	* @param object	&$o			reference to the backend outputter of the object painting the tool
	* @param string	$type_code	the type code of the tool being painted
	*
	* @return void
	* @access public
	*/
	static function paintTool(&$o, $type_code)
	{
		$tool_info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code);
		$o->openSection($tool_info['name']);
			$o->openField(translate('file_root_node'));
				$root_nodes = Array(
								'physical_file'		=> 'I',
								'physical_folder'	=> 'I',
								'file_bridge'		=> 'I',
							 );
				asset_finder($type_code.'_importing_root_assetid', '', $root_nodes);
				$o->note(translate('file_root_node_note'));
			$o->closeField();
			$o->openField(translate('file_parent_node'));
				asset_finder($type_code.'_importing_parent_assetid', '');
			$o->closeField();
			$o->openField(translate('file_recursive'));
				check_box($type_code.'_recursive', '1');
				$o->note(translate('file_recursive_note'));
			$o->closeField();
		$o->closeSection();

	}//end paintTool()


	/**
	* Processes the user input and performs the function of the tool.
	*
	* @param object	&$o			reference to the backend outputter of the object painting the tool
	* @param string	$type_code	the type code of the tool being painted
	*
	* @return boolean
	* @access public
	*/
	static function processTool(&$o, $type_code)
	{
		$importr = 0;
		$importp = 0;
		$recursive = FALSE;

		if (isset($_REQUEST[$type_code.'_importing_root_assetid']['assetid'])) {
			$importr = $_REQUEST[$type_code.'_importing_root_assetid']['assetid'];
		}

		if (isset($_REQUEST[$type_code.'_importing_parent_assetid']['assetid'])) {
			$importp = $_REQUEST[$type_code.'_importing_parent_assetid']['assetid'];
		}

		$recursive = (array_get_index($_REQUEST, $type_code.'_recursive', FALSE)) ? TRUE : FALSE;

		$sanity_check = TRUE;
		$allowable_root_nodes = Array('physical_file', 'physical_folder', 'file_bridge');
		$root_node = $GLOBALS['SQ_SYSTEM']->am->getAsset($importr);
		if (is_null($root_node) || !in_array(strtolower($root_node->type()), $allowable_root_nodes)) {
			$sanity_check = FALSE;
			if (!is_null($root_node)) {
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($root_node, TRUE);
			}//end if
		}//end if

		if (!empty($importr) && !empty($importp) && $sanity_check) {
			$vars = Array(
						'root_assetid' 		=> Array($importr),
						'parent_assetid'	=> $importp,
						'recursive'			=> $recursive,
					);

			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$hh->queueHipo('hipo_job_import_file', $vars, '', SQ_PACKAGES_PATH.'/filesystem/hipo_jobs');
			$url = $hh->runQueuedJobs();
			if (!empty($url)) $o->setRedirect($url);
			return TRUE;
		} else {
			// Fail
			if (empty($importr) && $sanity_check) {
				trigger_localised_error('FIL0001', E_USER_ERROR);
			} else {
				trigger_localised_error('FIL0003', E_USER_ERROR);
			}//end if
			return FALSE;
		}//end if

	}//end processTool()


}//end class


?>