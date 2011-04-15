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
* $Id: hipo_job_sync_cmis_bridge.inc,v 1.2 2011/04/07 03:50:18 mhaidar Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/hipo/hipo_job.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';

/**
* HIPO for Synchronizing CMIS Bridge files.
*
* @author  Mohamed Haidar <mhaidar@squiz.com/au>
* @version $Revision: 1.2 $
* @package MySource_Matrix
* @subpackage sharepoint
*/
class HIPO_Job_Sync_CMIS_Bridge extends HIPO_Job
{


	/**
	* Constructor
	*
	* @param string	$code_name		a unique codename the HIPO
	*/
	function HIPO_Job_Sync_CMIS_Bridge($code_name='')
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
		
		$repositoryId = '';
		if (!empty($this->_running_vars['repositoryId'])) $repositoryId = $this->_running_vars['repositoryId'];

		return parent::getCodeName().'-'.$repositoryId;

	}//end getCodeName()


	/**
	* Returns the (localised) HIPO name
	*
	* @return string
	* @access public
	*/
	function getHipoName()
	{
		return translate('snyc_cmis_bridge_hipo_name');

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
					'name'			=> 'Trash Files',
					'function_call'	=> Array(
										'process_function'	=> 'processTrashFiles',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'skip_step'		=> FALSE,
					'allow_cancel'	=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
				Array(
					'name'			=> 'Update Files',
					'function_call'	=> Array(
										'process_function'	=> 'processUpdateFiles',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'skip_step'		=> FALSE,
					'allow_cancel'	=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
				Array(
					'name'			=> 'Create Files',
					'function_call'	=> Array(
										'process_function'	=> 'processCreateFiles',
									   ),
					'running_mode'	=> 'server',
					'auto_step'		=> TRUE,
					'skip_step'		=> FALSE,
					'allow_cancel'	=> TRUE,
					'percent_done'	=> 0,
					'complete'		=> FALSE,
					'message'		=> '',
				),
			   );

	}//end getInitialStepData()


	/**
	* Prepares the running vars and gets the children of the assets in question.
	*
	* @return boolean
	* @access public
	*/
	function prepare()
	{
		if (empty($this->_running_vars['bridge_id'])) {
			trigger_localised_error('SHAR0002', E_USER_WARNING);
			return FALSE;
		}
		
		$bridge = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['bridge_id']);
		$repositoryId = $bridge->getRepositoryId();
		if (empty($repositoryId)){
			trigger_localised_error('SHAR0003', E_USER_WARNING, $bridge->attr('repository_name'));
			return FALSE;
		}
		$this->_running_vars['repositoryId'] = $repositoryId;
		
		$sync_info = $bridge->synchronizeCheck();
		if (!empty($sync_info['delete_assetids'])) {
			$this->_running_vars['todo_trash_assetids'] = $sync_info['delete_assetids'];
			$this->_running_vars['done_trash_assetids'] = Array();
		}
		
		if (!empty($sync_info['update_assetids'])) {
			$this->_running_vars['todo_update_assetids'] = $sync_info['update_assetids'];
			$this->_running_vars['done_update_assetids'] = Array();
		}
		
		if (!empty($sync_info['create_objectIds'])){
			$this->_running_vars['todo_create_objectIds'] = $sync_info['create_objectIds'];
			$this->_running_vars['done_create_objectIds'] = Array();
		}
		
		return parent::prepare();

	}//end prepare()



	/**
	* Performs the duties of this hipo, without showing any output on the frontend.
	*
	* @return boolean
	* @access public
	*/
	function freestyle()
	{
		while (!empty($this->_running_vars['todo_trash_assetids'])) {
			if (!$this->processTrashFiles($this->_steps[0], get_class($this))) {
				return FALSE;
			}
		}

		while (!empty($this->_running_vars['todo_update_assetids'])) {
			if (!$this->processUpdateFiles($this->_steps[1], get_class($this))) {
				return FALSE;
			}
		}
		
		while (!empty($this->_running_vars['todo_create_objectIds'])) {
			if (!$this->processCreateFiles($this->_steps[2], get_class($this))) {
				return FALSE;
			}
		}
		
		return TRUE;

	}//end freestyle()

	
	/**
	* Trash Files
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processTrashFiles(&$step_data, $prefix)
	{
		if (!empty($this->_running_vars['todo_trash_assetids'])) {
			$assetid = array_shift($this->_running_vars['todo_trash_assetids']);
			$step_data['message'] = "Trashing Asset (Id: #$assetid)";

			$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_SECURE);
			$GLOBALS['SQ_SYSTEM']->am->trashAsset($assetid);
			$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
			
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			$asset->updateLookups();
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			
			$this->_running_vars['done_trash_assetids'][] = $assetid;
		}

		if (empty($this->_running_vars['todo_trash_assetids'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
		} else {
			$total = count($this->_running_vars['todo_trash_assetids']) + count($this->_running_vars['done_trash_assetids']);
			$step_data['percent_done'] =(count($this->_running_vars['done_trash_assetids']) / $total) * 100;
			$step_data['complete']     = FALSE;
		}

		return TRUE;

	}//end processTrashFiles()
	
	
	/**
	* Update Files
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processUpdateFiles(&$step_data, $prefix)
	{
		if (!empty($this->_running_vars['todo_update_assetids'])) {
			$todo = array_shift($this->_running_vars['todo_update_assetids']);
			
			$bridge = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['bridge_id']);
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($todo['assetid']);
			if ($GLOBALS['SQ_SYSTEM']->am->acquireLock($asset->id, 'all')) {
				$binary = $bridge->getObjectContents($this->_running_vars['repositoryId'], $todo['objectId']);
				if (!empty($binary)) {
					$full_path = tempnam($bridge->data_path, "cmis:update_");
					file_put_contents($full_path, $binary);
					
					$file_name = $todo['contentStreamFileName'];
					$temp_info = Array('name' => $file_name, 'tmp_name' => $full_path, 'non_uploaded_file' => TRUE);
					
					require_once SQ_INCLUDE_PATH.'/backend_outputter.inc';
					$o = new Backend_Outputter();
					$edit_fns = $asset->getEditFns();
					$edit_fns->processFileUpload($asset, $o, $prefix, $temp_info);
					
					$asset->setAttrValue('title', $todo['contentStreamFileName']);
					$asset->saveAttributes();
					
					$metadata = Array();
					foreach ($bridge->attr('metadata_fields') as $property => $info){
						if ($info['field']) $metadata[$info['field']][0]['value'] = $todo[$property];
					}
					$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
					$mm->setMetadata($asset->id, $metadata);
					$mm->regenerateMetadata($asset->id);
					
					$step_data['message'] = "Updated Asset $asset->name (Id: #$asset->id)";
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
					
					unlink($full_path);
				} else {
					$step_data['message'] = translate('hipo_skipping_asset', $asset->id);
					$this->_addError(translate('snyc_cmis_bridge_hipo_can_not_update_file_no_content_stream', $todo['contentStreamFileName']), TRUE);
				}
				$GLOBALS['SQ_SYSTEM']->am->releaseLock($asset->id, 'all');
				
			} else {
				$step_data['message'] = translate('hipo_skipping_asset', $asset->id);
				$this->_addError(translate('snyc_cmis_bridge_hipo_can_not_update_file', $asset->name, $asset->id), TRUE);
			}
			
			$this->_running_vars['done_update_assetids'][] = $todo;
		}

		if (empty($this->_running_vars['todo_update_assetids'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
		} else {
			$total = count($this->_running_vars['todo_update_assetids']) + count($this->_running_vars['done_update_assetids']);
			$step_data['percent_done'] =(count($this->_running_vars['done_update_assetids']) / $total) * 100;
			$step_data['complete']     = FALSE;
		}

		return TRUE;

	}//end processUpdateFiles()
	
	
	/**
	* Create Files
	*
	* @param array	&$step_data	a reference to the array of information about the current step
	* @param string	$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function processCreateFiles(&$step_data, $prefix)
	{
		if (!empty($this->_running_vars['todo_create_objectIds'])) {
			$todo = array_shift($this->_running_vars['todo_create_objectIds']);
			$bridge = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_running_vars['bridge_id']);
			
			$binary = $bridge->getObjectContents($this->_running_vars['repositoryId'], $todo['objectId']);
			if (!empty($binary)) {
				$import_link = Array('asset' => &$bridge, 'link_type' => SQ_LINK_TYPE_1);
				
				$file_type = get_file_type($todo['contentStreamFileName']);
				switch ($file_type) {
					case 'doc' :
					case 'docx':
					case 'dot' :
						$GLOBALS['SQ_SYSTEM']->am->includeAsset('word_doc');
						$new_asset_type = 'word_doc';
					break;
					case 'pdf' :
						$GLOBALS['SQ_SYSTEM']->am->includeAsset('pdf_file');
						$new_asset_type = 'pdf_file';
					break;
					case 'xls' :
					case 'xlt' :
					case 'xlsx' :
					case 'xlsm' :
					case 'xltx' :
					case 'xltm' :
						$GLOBALS['SQ_SYSTEM']->am->includeAsset('excel_doc');
						$new_asset_type = 'excel_doc';
					break;
					case 'ppt' :
					case 'pot' :
					case 'pps' :
					case 'pptx' :
					case 'potx' :
					case 'ppsx' :
					case 'pptm' :
					case 'potm' :
					case 'ppsm' :
						$GLOBALS['SQ_SYSTEM']->am->includeAsset('powerpoint_doc');
						$new_asset_type = 'powerpoint_doc';
					break;
					case 'rtf' :
						$GLOBALS['SQ_SYSTEM']->am->includeAsset('rtf_file');
						$new_asset_type = 'rtf_file';
					break;
					case 'txt' :
						$GLOBALS['SQ_SYSTEM']->am->includeAsset('text_file');
						$new_asset_type = 'text_file';
					break;
					case 'js' :
						$GLOBALS['SQ_SYSTEM']->am->includeAsset('js_file');
						$new_asset_type = 'js_file';
					break;
					case 'css' :
						$GLOBALS['SQ_SYSTEM']->am->includeAsset('css_file');
						$new_asset_type = 'css_file';
					break;
					case 'gif' :
					case 'jpg' :
					case 'jpeg' :
					case 'png' :
						$GLOBALS['SQ_SYSTEM']->am->includeAsset('image');
						$new_asset_type = 'image';
					break;
					case 'mp3' :
						$GLOBALS['SQ_SYSTEM']->am->includeAsset('mp3_file');
						$new_asset_type = 'mp3_file';
					break;
					case 'mov' :
					case 'avi' :
					case 'wmv' :
					case 'asf' :
					case 'flv' :
					case 'mp4' :
					case 'm4v' :
					case 'mpg' :
					case 'mpeg':
						$GLOBALS['SQ_SYSTEM']->am->includeAsset('video_file');
						$new_asset_type = 'video_file';
					break;
					default :
						$GLOBALS['SQ_SYSTEM']->am->includeAsset('file');
						$new_asset_type = 'file';
					break;
				}
				
				$full_path = tempnam($bridge->data_path, "cmis:create_");
				file_put_contents($full_path, $binary);
				
				$file_name = $todo['contentStreamFileName'];
				$temp_info = Array('name' => $file_name, 'tmp_name' => $full_path, 'non_uploaded_file' => TRUE);
				$new_file = new $new_asset_type();
				$new_file->_tmp['uploading_file'] = TRUE;
				$new_file->setAttrValue('name', $file_name);
				$new_file->setAttrValue('title', $todo['contentStreamFileName']);
				$new_file->setAttrValue('allow_unrestricted', TRUE);
	
				if (!$new_file->create($import_link, $temp_info)) {
					$step_data['message'] = translate('snyc_cmis_bridge_hipo_skip_doc', $todo['contentStreamFileName']);
					$this->_addError(translate('snyc_cmis_bridge_hipo_can_not_create_file', $todo['contentStreamFileName'], $bridge->id), TRUE);
				} else {
					$GLOBALS['SQ_SYSTEM']->am->acquireLock($new_file->id, 'metadata');
					
					$metadata = Array();
					foreach ($bridge->attr('metadata_fields') as $property => $info){
						if ($info['field']) $metadata[$info['field']][0]['value'] = $todo[$property];
					}
					$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
					$mm->setMetadata($new_file->id, $metadata);
					$mm->regenerateMetadata($new_file->id);
					
					$GLOBALS['SQ_SYSTEM']->am->releaseLock($new_file->id, 'metadata');
					
					$step_data['message'] = "Created Asset $new_file->name (Id: #$new_file->id)";
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($new_file);
				}
				
				unlink($full_path);
			} else {
				$step_data['message'] = translate('snyc_cmis_bridge_hipo_skip_doc', $todo['contentStreamFileName']);
				$this->_addError(translate('snyc_cmis_bridge_hipo_can_not_create_file_no_content_stream', $todo['contentStreamFileName']), TRUE);
			}
			
			$this->_running_vars['done_create_objectIds'][] = $todo;

		}//end if

		if (empty($this->_running_vars['todo_create_objectIds'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = TRUE;
			unset($this->_running_vars['done_create_objectIds']);
		} else {
			$total = count($this->_running_vars['todo_create_objectIds']) + count($this->_running_vars['done_create_objectIds']);
			$step_data['percent_done'] = (count($this->_running_vars['done_create_objectIds']) / $total) * 100;
			$step_data['complete']     = FALSE;
		}

		return TRUE;

	}//end processCreateFiles()
	

}//end class

?>