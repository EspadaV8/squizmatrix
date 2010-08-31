<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: hipo_job_import_assets_from_xml.inc,v 1.1 2010/08/26 06:49:01 akarelia Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';
require_once SQ_LIB_PATH.'/import_export/import.inc';


/**
* HIPO for importing a large HTML file into Matrix assets
*
* Initial Running Vars :
*			'file_info' 			- Full path to the file to import
*			'create_under_assetid'	- Root asset ID to import to
*
*
* @author  Ash Karelia <akarelia@squiz.com.au>
* @version $Revision: 1.1 $
* @package MySource_Matrix
* @subpackage import_tools/hipo_jobs
*
*/
class HIPO_Job_Import_Assets_From_Xml extends HIPO_Job
{


	public $import_action_outputs = Array();

	/**
	* Constructor
	*
	* @param string	$code_name		a unique codename the HIPO
	*/
	function HIPO_Job_Import_Assets_From_Xml($code_name='')
	{
		$this->uses_trans = FALSE;
		$this->HIPO_Job($code_name);

	}//end constructor


	/**
	* Returns a unique id for this job
	*
	* @return string
	* @access public
	*/
	function getCodeName()
	{
		return 'HIPO_Job_Import_Assets_From_Xml-'.md5($this->_running_vars['create_under_assetid'].$this->_running_vars['file_info']);

	}//end getCodeName()


	/**
	* Returns the (localised) HIPO name
	*
	* @return string
	* @access public
	*/
	function getHipoName()
	{
		return translate('tool_import_assets_from_xml_hipo_name');

	}//end getHipoName()


	/**
	* Returns the steps in this hipo, possibly with localised step names
	* and messages
	*
	* @return string
	* @access public
	*/
	function getInitialStepData()
	{
		return Array(
				Array(
					'name'			=> translate('tool_import_assets_from_xml_hipo_name'),
					'function_call'	=> Array(
										'process_function'	=> 'processImportAsset',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'allow_cancel'	=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
			  );

	}//end getInitialStepData()


	/**
	* Performs the duties of this hipo, without showing any output on the frontend.
	*
	* @return boolean
	* @access public
	*/
	function freestyle()
	{
		$step_data['complete'] = FALSE;
		while(!$step_data['complete']) {
			if($this->processImportAsset($step_data, '')) {
				return FALSE;
			}
		}

		return TRUE;

	}//end freestyle()


	/**
	* Prepares the running vars
	*
	* @return boolean
	* @access public
	*/
	function prepare()
	{
		$file_info = $this->_running_vars['file_info'];
		if (empty($file_info)) {
			trigger_localised_error('HIPO0095', E_USER_NOTICE);
			return FALSE;
		}

		$root_node_id 	= $this->_running_vars['create_under_assetid'];
		// if value of root node is 0 then we are good. It just means we are not selecting root node
		// but if we have a value, then make sure it is a proper asset
		if($root_node_id != 0) {
			$root_node 		= $GLOBALS['SQ_SYSTEM']->am->getAsset($root_node_id);
			if (is_null($root_node)) {
				trigger_localised_error('HIPO0096', E_USER_NOTICE);
				return FALSE;
			}
		}

		return parent::prepare();

	}//end prepare()


	/**
	* Import Bulkmail Users from a CSV file
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processImportAsset(&$step_data, $prefix)
	{
		$import_actions = get_import_actions($this->_running_vars['file_info']);
		// if we want to explictily target the root node under which the structure is imported , do it
		if(isset($this->_running_vars['create_under_assetid']) && $this->_running_vars['create_under_assetid'] != 0) {
			$import_actions['actions'][0]['action'][0]['parentid'][0] = $this->_running_vars['create_under_assetid'];
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_OPEN);
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$import_action_outputs = Array();
		// Loop through the Actions from the XML File
		foreach ($import_actions['actions'][0]['action'] as $action) {
			// Execute the action
			if (!execute_import_action($action, $import_action_outputs)) {
				trigger_error('Action ID "'.$action['action_id'][0].'" could not be executed', E_USER_WARNING);
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		// update progress
		$step_data['percent_done'] = 100;
		$step_data['complete']     = TRUE;

		// done with everything ? delete the file
		unlink($this->_running_vars['file_info']);
		return TRUE;

	}//end processImport()


}//end class

?>