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
* $Id: hipo_job_import_file.inc,v 1.2 2010/03/30 00:44:14 bpearson Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';

/**
* Highly Intensive Processing Object (HIPO) to import a file from the file bridge 
*
* Initial Running Vars :
*    root_assetid	-	the ID of the root asset of the tree that will be imported
*
* @author  Benjamin Pearson <bpearson@squiz.com.au>
* @version $Revision: 1.2 $
* @package MySource_Matrix
* @subpackage file
*/
class HIPO_Job_Import_File extends HIPO_Job
{


	/**
	* Constructor
	*
	* @param string	$code_name		a unique codename the HIPO
	*/
	function HIPO_Job_Import_File($code_name='')
	{
		$this->uses_trans = FALSE;
		$this->HIPO_Job($code_name);
		$GLOBALS['SQ_SYSTEM']->lm->includePackageStrings('file');
		$this->_hipo_vars['job_dir'] = SQ_PACKAGES_PATH.'/filesystem/hipo_jobs';

	}//end constructor


	/**
	* Paints backend of this job
	*
	* @param object	&$o		reference to the backend outputter
	* @param string	$class	calling class name
	*
	* @return void
	* @access private
	*/
	public static function paintConfig(&$o, $class, $write_access)
	{
		$o->openField(translate('file_hipo_import_file_threshold'));

		if ($write_access) {
			text_box($class.'[SQ_HIPO_IMPORT_FILE_THRESHOLD]', SQ_HIPO_IMPORT_FILE_THRESHOLD, 5);
		} else {
			echo SQ_HIPO_IMPORT_FILE_THRESHOLD;
		}
		echo ' '.translate('assets');

		$o->closeField();

	}//end paintConfig()


	/**
	* Sets some info about thresholds that this hipo requires.
	*
	* @return array
	* @access public
	*/
	public static function getConfigVars()
	{
		return Array(
				'SQ_HIPO_IMPORT_FILE_THRESHOLD'	=> Array('editable'	=> 1, 'default'	=> 5),
			   );

	}//end getConfigVars()

	
	/**
	* Returns the current Threshold.
	*
	* Returns a value that represents the total percentage that this hipo would run in
	* freestyle mode in its current state.
	*
	* @return float
	* @access public
	*/
	function getThresholdPercentageRequired()
	{
		if (SQ_HIPO_IMPORT_FILE_THRESHOLD == 0) return 0;
		if (!isset($this->_running_vars['todo_assetids'])) {
			return 0;
		}
		
		return ((count($this->_running_vars['todo_assetids']) / SQ_HIPO_IMPORT_FILE_THRESHOLD) * 100);

	}//end getThresholdPercentageRequired()
	
	
	/**
	* Returns a unique codename the HIPO can use to ensure it is not being run twice
	*
	* The Regenerate Metdata HIPO appends the ID of the schema that was updated so that any other
	* regeneration jobs for this schema do not start while it is running.
	*
	* @return string
	* @access public
	*/
	function getCodeName()
	{
		$running_dependants = '';
		if (!empty($this->_running_vars['process_dependants'])) {
			$running_dependants = $this->_running_vars['process_dependants'];
		}
		return parent::getCodeName().'-'.implode(',', $this->_running_vars['root_assetid']).$running_dependants;

	}//end getCodeName()


	/**
	* Returns the (localised) HIPO name
	*
	* @return string
	* @access public
	*/
	function getHipoName()
	{
		return translate('file_hipo_name_import_file');

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
					'name'			=> $this->getHipoName(),
					'function_call'	=> Array(
										'process_function'	=> 'processImport',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
					'allow_cancel'	=> TRUE,
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
		while (!empty($this->_running_vars['todo_assetids']) || !empty($this->_running_vars['dependant_hipo_job'])) {
			if (!$this->processImport($this->_steps[0], get_class_lower($this), TRUE)) {
				return FALSE;
			}
		}
		return TRUE;

	}//end freestyle()


	/**
	* Prepares the running vars and gets the children of the asset in question.
	*
	* @return boolean
	* @access public
	*/
	function prepare()
	{
		$allowable_file_types = Array('physical_file', 'physical_folder');
		$allowable_root_nodes = Array('physical_file', 'physical_folder', 'file_bridge');
		// Parent (where to put the files/folders) bail if none set 
		if (empty($this->_running_vars['parent_assetid']) || is_null($GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['parent_assetid']))) {
			trigger_localised_error('FIL0003', E_USER_WARNING);
			return FALSE;
		}//end if

		// No root node is set, bail out... 
		if (is_null($this->_running_vars['root_assetid']) || empty($this->_running_vars['root_assetid'])) {
			trigger_localised_error('FIL0001', E_USER_WARNING);
			return FALSE;
		}//end if
		foreach ($this->_running_vars['root_assetid'] as $assetid) {
			// so we have an assetid to start at, but make sure it is a valid assetid
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			if (is_null($asset)) {
				trigger_localised_error('FIL0001', E_USER_WARNING);
				return FALSE;
			}//end if

			if (!in_array(strtolower($asset->type()), $allowable_root_nodes)) {
				// What the heck..? We cannot import anything NOT under the file bridge
				trigger_localised_error('FIL0002', E_USER_WARNING, $assetid);
				return FALSE;
			}//end if

			if (strtolower($asset->type()) == 'physical_file') {
				$child_assets = Array(
									$asset->id	=> Array(
														Array(
															'type_code'	=> $asset->type(),
														),
												   ),
								);
			} else {
				$child_assets = $GLOBALS['SQ_SYSTEM']->am->getChildren($asset->id);
			}//end if
			$this->_running_vars['todo_assetids'] = $child_assets;
			$this->_running_vars['done_assetids'] = Array();
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset, TRUE);
			unset($asset);
		}//end foreach

		return parent::prepare();

	}//end prepare()


	/**
	* Import the file/folder from under the root node 
	*
	* @param array		&$step_data	a reference to the array of information about the current step
	* @param string		$prefix		prefix for form vars
	* @param boolean	$freestyle	if this function is being freestyled or not
	*
	* @return boolean
	* @access public
	*/
	function processImport(&$step_data, $prefix, $freestyle=FALSE)
	{
		// process dependant hipo_job_import_file if necessary
		if (!empty($this->_running_vars['dependant_hipo_job'])) {
			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$hipo = $hh->getJob($this->_running_vars['dependant_hipo_job']);

			// bail if we've completed this job
			if (is_null($hipo)) return FALSE;

			if ($freestyle && $hipo->freestyle()) {
				// we're freestylin' and that it, done
				unset($this->_running_vars['dependant_hipo_job']);
				$hipo->save();
			} else if ($hipo->process() && $hipo->complete()) {
				// process the sub hipo on each refresh
				unset($this->_running_vars['dependant_hipo_job']);
			}
			return TRUE;

		}//end if

		$allowable_file_types = Array('physical_file', 'physical_folder');
		if (!empty($this->_running_vars['todo_assetids'])) {
			$am =& $GLOBALS['SQ_SYSTEM']->am;

			// get next asset to import
			$array_keys = array_keys($this->_running_vars['todo_assetids']);
			$assetid = array_shift($array_keys);
			unset($this->_running_vars['todo_assetids'][$assetid]);
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			$parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['parent_assetid']);

			if (!is_null($parent) && !is_null($asset) && in_array(strtolower($asset->type()), $allowable_file_types)) {
				if (strtolower($asset->type()) == 'physical_file') {
					$step_data['message'] = translate('file_importing_file', htmlentities($asset->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET));
					$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
					$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

					// Import file
					$temp_info = Array(
									'name'				=> $asset->name,
									'tmp_name'			=> $asset->getFilePath(),
									'non_uploaded_file' => TRUE,
								 );
					$GLOBALS['SQ_SYSTEM']->am->includeAsset('file');
					$new = new File();
					$new->_tmp['uploading_file'] = TRUE;
					$new->setAttrValue('name', $asset->name);
					$new->setAttrValue('allow_unrestricted', FALSE);
					$link = Array(
								'asset'			=> &$parent,
								'link_type'		=> SQ_LINK_TYPE_1,
								'link_value'	=> '',
								'sort_order'	=> 0,
								'is_exclusive'	=> FALSE,
								'is_dependant'	=> FALSE,
							);
					$created = $new->create($link, $temp_info);

					$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				} else {
					$step_data['message'] = translate('file_importing_folder', htmlentities($asset->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET));
					$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
					$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

					// Import folder
					$GLOBALS['SQ_SYSTEM']->am->includeAsset('folder');
					$new = new Folder();
					$new->setAttrValue('name', $asset->name);
					$link = Array(
								'asset'			=> &$parent,
								'link_type'		=> SQ_LINK_TYPE_1,
								'link_value'	=> '',
								'sort_order'	=> 0,
								'is_exclusive'	=> FALSE,
								'is_dependant'	=> FALSE,
							);
					$created = $new->create($link);

					if ($created && $this->_running_vars['recursive']) {
						if ($code_name = $this->spawnDependantImportFileHipo($asset->id, $new->id)) {
							$this->_running_vars['dependant_hipo_job'] = $code_name;
						}
					}//end if

					$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				}//end if

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset, TRUE);
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($parent, TRUE);
				unset($asset);
				unset($parent);
			}//end if

			// add this assetid to the done array
			$this->_running_vars['done_assetids'][] = $assetid;
		}//end if

		if (empty($this->_running_vars['todo_assetids'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
		} else {
			$total = count($this->_running_vars['todo_assetids']) + count($this->_running_vars['done_assetids']);
			$step_data['percent_done'] =(count($this->_running_vars['done_assetids']) / $total) * 100;
			$step_data['complete']     = FALSE;
		}//end if

		return TRUE;

	}//end processImport()


	/**
	* Starts another Hipo_Job_Import_File to process files/folders of given asset
	*
	* @param string	$assetid	The new root node
	* @param int	$parentid	The new parent node
	*
	* @return string
	* @access private
	*/
	function spawnDependantImportFileHipo($assetid, $parentid)
	{
		$init_hipo = new Hipo_Job_Import_File();

		$options['auto_complete'] = TRUE;

		$running_vars = Array(
							'root_assetid'			=> Array($assetid),
							'parent_assetid'		=> $parentid,
							'process_dependants'	=> $assetid.'-'.$parentid,
							'recursive'				=> TRUE,
						);

		$init_hipo->setRunningVars($running_vars);
		foreach ($options as $k => $v) {
			$init_hipo->setOption($k, $v);
		}

		// create our sub-sub-hipo thingy, until this job is completed, we need
		// to process it in place of our todo assets
		$code_name = $init_hipo->initialise($this->source_code_name);
		if ($code_name) return $code_name;
		return FALSE;

	}//end spawnDependantImportFileHipo()


}//end class

?>