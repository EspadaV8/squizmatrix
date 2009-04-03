<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600													   |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: This Module is not available under an open source	   |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this	   |
* | Module if you have the written consent of Squiz.				   |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: soap_api_workflow_service.inc,v 1.1 2009/04/03 05:57:16 hnguyen Exp $
*
*/

check_ssv_licence();
require_once SQ_PACKAGES_PATH.'/web_services/api/soap_api/soap_api.inc';

/**
* Soap_Api_Workflow_Service
*
* Purpose
*
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class Soap_Api_Workflow_Service extends Soap_Api
{


    /**
    * Constructor
    *
    * @param int    $assetid    the asset id to be loaded
    *
    */
    function __construct($assetid=0)
    {
        parent::__construct($assetid);

    }//end constructor


    /**
    * Returns a function list of the asset let the children handle this
    *
    * @return string
    * @access private
    * @see Asset::_getName()
    */
    public function getFunctionList()
    {
        return Array(
				'StartWorkflow'				=> '1',
				'CancelWorkflow'			=> '1',
				'CompleteWorkflow'			=> '1',
				'SetWorkflowSchema'			=> '1',
				'ApproveAssetInWorkflow'	=> '1',
			   );

    }//end getFunctionList()


	/**
	* Description: This operation will start the workflow for the asset in query
	*
    * @param string  $request  The request information
    * <pre>
    * Array (
	*		'AssetID'	 		=> [The ID of the asset in query],
    *        )
    * </pre>
	*
	* @return void
	* @access public
	*/
	function StartWorkflow($request)
	{
		$request_info = (Array) $request;
		// We gotta assume that when someone call this function, they know what status the asset is in, and what status it is going to.
		// We are going to do the changes, but there will be no confirmation for the caller to be warned what is going to happen.

		$assetid	= array_get_index($request_info, 'AssetID', '');
		if (!empty($assetid)) {
			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
			$asset			= $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			$asset_status	= $asset->status;
			if (!in_array($asset_status, Array (SQ_STATUS_UNDER_CONSTRUCTION, SQ_STATUS_LIVE, SQ_STATUS_EDITING))) {
				throw new SoapFault('Server', 'Can Not Start Workflow When Asset Is In '.get_status_description($asset_status).' Status');
			}//end if

			$wm					= $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
			$current_schemas	= $wm->getSchemas($assetid, TRUE);
			if (empty($current_schemas)) {
				throw new SoapFault('Server', 'There Is No Workflow Schemas Applied On This Asset');
			}//end if

			// We are starting workflow by change the status of the asset
			$running_vars	= Array (
								'assetid'			=> $assetid,
								'new_status'		=> self::getNextStatusStartWorkflow($asset_status),
								'dependants_only'	=> TRUE,
							  );

			$hh		= $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$errors	= $hh->freestyleHipo('hipo_job_edit_status', $running_vars);
			if (empty($errors)) {
				return Array (
						'StartWorkflowResult'	=> TRUE,
					   );
			} else {
				throw new SoapFault('Server', 'Unable To Start Workflow');
			}//end else

		} else {
			throw new SoapFault('Server', 'Asset ID Is Not Valid. Please Provide A Valid Asset ID');
		}//end else

	}//end StartWorkflow()


	/**
	 * This function return the next status for an asset to go to when workflow is starting
	 *
	 * @param $status string	The current status of the asset
	 * @return int
	 */
	private static function getNextStatusStartWorkflow($status)
	{
		$next_statuses	= Array (
							SQ_STATUS_UNDER_CONSTRUCTION	=> SQ_STATUS_PENDING_APPROVAL,
							SQ_STATUS_LIVE					=> SQ_STATUS_LIVE_APPROVAL,
							SQ_STATUS_EDITING				=> SQ_STATUS_EDITING_APPROVAL,
						  );
		if (isset($next_statuses)) {
			return ($next_statuses[$status]);
		} else {
			throw new SoapFault('Server', 'There are no status to advance to from status '.$status);
		}//end else

	}//end getNextStatusStartWorkflow()


	/**
	 * This function return the next status for an asset to go to when workflow is cancelled
	 *
	 * @param $status string	The current status of the asset
	 * @return int
	 */
	private static function getNextStatusCancelWorkflow($status)
	{
		$next_statuses	= Array (
							SQ_STATUS_PENDING_APPROVAL		=> SQ_STATUS_UNDER_CONSTRUCTION,
							SQ_STATUS_APPROVED				=> SQ_STATUS_UNDER_CONSTRUCTION,
							SQ_STATUS_LIVE_APPROVAL			=> SQ_STATUS_EDITING,
							SQ_STATUS_EDITING				=> SQ_STATUS_LIVE,
							SQ_STATUS_EDITING_APPROVAL		=> SQ_STATUS_EDITING
						  );
		if (isset($next_statuses)) {
			return ($next_statuses[$status]);
		} else {
			throw new SoapFault('Server', 'There are no status to advance to from status '.$status);
		}//end else

	}//end getNextStatusStartWorkflow()


	/**
	 * This function return the next status for an asset to go to when workflow is completed
	 *
	 * @param $status string	The current status of the asset
	 * @return int
	 */
	private static function getNextStatusCompleteWorkflow($status)
	{
		$next_statuses	= Array (
							SQ_STATUS_APPROVED			=> SQ_STATUS_LIVE,
							SQ_STATUS_LIVE_APPROVAL		=> SQ_STATUS_LIVE,
							SQ_STATUS_EDITING_APPROVED	=> SQ_STATUS_LIVE,
						  );
		if (isset($next_statuses)) {
			return ($next_statuses[$status]);
		} else {
			throw new SoapFault('Server', 'There are no status to advance to from status '.$status);
		}//end else

	}//end getNextStatusStartWorkflow()


	/**
	 * This function return the next status for an asset to go to when workflow is approved
	 *
	 * @param $status string	The current status of the asset
	 * @return int
	 */
	private static function getNextStatusApproveAssetInWorkflow($status)
	{
		$next_statuses	= Array (
							SQ_STATUS_EDITING			=> SQ_STATUS_EDITING_APPROVED,
							SQ_STATUS_UNDER_CONSTRUCTION=> SQ_STATUS_APPROVED,
							SQ_STATUS_PENDING_APPROVAL	=> SQ_STATUS_PENDING_APPROVAL,
							SQ_STATUS_PENDING_APPROVAL	=> SQ_STATUS_PENDING_APPROVED,
							SQ_STATUS_LIVE_APPROVAL		=> SQ_STATUS_LIVE_APPROVAL,
						  );
		if (isset($next_statuses)) {
			return ($next_statuses[$status]);
		} else {
			throw new SoapFault('Server', 'There are no status to advance to from status '.$status);
		}//end else

	}//end getNextStatusApproveAssetInWorkflow()

	/**
	* Description: This operation will cancel the current running workflow of an asset.
	* If the asset is in LIVE APPROVAL status, it will turn into SAFE EDIT status.
	*
    * @param string  $request  The request information
    * <pre>
    * Array (
	*		'AssetID'	 		=> [The ID of the asset in query],
    *        )
    * </pre>
	*
	* @return void
	* @access public
	*/
	function CancelWorkflow($request)
	{
		$request_info	= (Array) $request;
		$assetid		= array_get_index($request_info, 'AssetID', '');
		if (!empty($assetid)) {
			$asset	= $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
			$asset			= $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			$asset_status	= $asset->status;
			if (!in_array($asset_status, Array (SQ_STATUS_PENDING_APPROVAL, SQ_STATUS_LIVE_APPROVAL, SQ_STATUS_EDITING, SQ_STATUS_APPROVED, SQ_STATUS_EDITING_APPROVAL))) {
				throw new SoapFault('Server', 'Can Not Cancel Workflow When Asset Is In '.get_status_description($asset_status).' Status');
			}//end if

			$wm					= $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
			$current_schemas	= $wm->getSchemas($assetid, TRUE, ($asset_status == SQ_STATUS_EDITING) ? FALSE : TRUE);
			if (empty($current_schemas)) {
				throw new SoapFault('Server', 'There is no workflow schemas applied on this asset');
			}//end if

			// We are starting workflow by changing the status of the asset
			$running_vars	= Array (
								'assetid'			=> $assetid,
								'new_status'		=> self::getNextStatusCancelWorkflow($asset_status),
								'dependants_only'	=> TRUE,
							  );

			$hh		= $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$errors	= $hh->freestyleHipo('hipo_job_edit_status', $running_vars);
			if (empty($errors)) {
				return Array (
						'CancelWorkflowResult'	=> TRUE,
					   );
			} else {
				throw new SoapFault('Server', 'Unable To Cancel Workflow');
			}//end else

		} else {
			throw new SoapFault('Server', 'Asset ID is not valid. Please provide a valid Asset ID');
		}//end else

	}//end CancelWorkflow()


	/**
	* Description: This operation will complete the current running workflow of an asset
	*
	* @param array $asset_info the info array to find children of a specific asset
    * <pre>
    * Array (
	*		'AssetID'	 		=> [The ID of the asset in query],
    *        )
    * </pre>
	*
	* @return void
	* @access public
	*/
	function CompleteWorkflow($request)
	{
		$request_info	= (Array) $request;
		$assetid		= array_get_index($request_info, 'AssetID', '');

		if (!empty($assetid)) {
			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
			$asset			= $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			$asset_status	= $asset->status;
			if (!in_array($asset_status, Array (SQ_STATUS_APPROVED, SQ_STATUS_EDITING_APPROVED, SQ_STATUS_LIVE_APPROVAL))) {
				throw new SoapFault('Server', 'Can Not Complete Workflow When Asset Is In '.get_status_description($asset_status).' Status');
			}//end if

			$wm					= $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
			$current_schemas	= $wm->getSchemas($assetid, TRUE, TRUE);
			if (empty($current_schemas)) {
				throw new SoapFault('Server', 'There is no workflow schemas applied on this asset');
			}//end if

			if ($asset_status == SQ_STATUS_PENDING_APPROVAL) {
				// check whether we should go to pending approval next or pending approved next
				$new_status	= SQ_STATUS_PENDING_APPROVED;
			} else {
				$new_status	= self::getNextStatusCompleteWorkflow($asset_status);
			}//end else

			// We are starting workflow by change the status of the asset
			$running_vars	= Array (
								'assetid'			=> $assetid,
								'new_status'		=> $new_status,
								'dependants_only'	=> TRUE,
							  );

			$hh		= $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$errors	= $hh->freestyleHipo('hipo_job_edit_status', $running_vars);
			if (empty($errors)) {
				return Array (
						'CompleteWorkflowResult'	=> TRUE,
					   );
			} else {
				throw new SoapFault('Server', 'Unable To Complete Workflow');
			}//end else

		} else {
			throw new SoapFault('Server', 'Asset ID Is Not Valid. Please Provide A Valid Asset ID');
		}//end else

	}//end CompleteWorkflow()


	/**
	* Description: This operation will approve an asset in workflow
	*
	* @return void
	* @access public
	*/
	function ApproveAssetInWorkflow($request)
	{
		// This one is a very hard one, gotta be very careful
		$request_info		= (Array) $request;
		$assetid			= array_get_index($request_info, 'AssetID', '');
		$workflow_message	= array_get_index($reuqest_info, 'WorkflowMessage', '');
		if (!empty($assetid)) {
			$asset	= $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			$available_statuses	= $asset->getAvailableStatii();
			if (!in_array($asset_status, Array (SQ_STATUS_EDITING, SQ_STATUS_UNDER_CONSTRUCTION, SQ_STATUS_APPROVED, SQ_STATUS_EDITING_APPROVAL, SQ_STATUS_PENDING_APPROVAL))) {
				throw new SoapFault('Server', 'Can Not Approve Workflow When Asset Is In '.get_status_description($asset_status).' Status');
			}//end if

			$asset				= $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			$wfm				= $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
			$current_userid 	= $GLOBALS['SQ_SYSTEM']->currentUserId();

			$wfm = $GLOBALS['SQ_SYSTEM']->getWorkflowManager();
			$running_schemas = $wfm->getSchemas($owner->id, TRUE, TRUE);

			if (empty($running_schemas) && $asset_status != SQ_STATUS_UNDER_CONSTRUCTION) {
				throw new SoapFault('Server', 'There is no workflow schemas running on this asset');
			}//end if

			// are comments required?
			$require_comment = $wfm->requiresComment($asset->id, $current_userid);
			if (!empty($running_schemas) && !($require_comment && empty($message_body))) {
				// Lets do the status processing
				// We are escalating workflow by change the status of the asset
				$running_vars	= Array (
									'assetid'			=> $assetid,
									'new_status'		=> self::getNextStatusApproveAssetInWorkflow($asset_status),
									'dependants_only'	=> TRUE,
								  );

				$hh		= $GLOBALS['SQ_SYSTEM']->getHipoHerder();
				$errors	= $hh->freestyleHipo('hipo_job_edit_status', $running_vars);
				if (empty($errors)) {
					return Array (
							'CompleteWorkflowResult'	=> TRUE,
						   );
				} else {
					throw new SoapFault('Server', 'Unable To Complete Workflow');
				}//end else


				// Then send a message
				if (!empty($message_body)) {
					$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
					$msg_reps = Array(
									'message'	=> $message_body,
								);
					$message = $ms->newMessage(Array(), 'asset.workflow.userlog', $msg_reps);
					$message->parameters['assetid'] = $owner->id;
					$message->parameters['version'] = substr($owner->version, 0, strrpos($owner->version, '.'));
					$message->send();
					$updated = TRUE;
				}//end if
			}

		} else {
			throw new SoapFault('Server', 'Asset ID Is Not Valid. Please Provide A Valid Asset ID');
		}//end else

	}//end ApproveAssetInWorkflow()


	/**
	* Description: This operation will apply or revoke a workflow schema on an asset
	*
	* @return void
	* @access public
	*/
	function SetWorkflowSchema($request)
	{
		$request_info		= (Array) $request;
		$assetid			= array_get_index($request_info, 'AssetID', '');
		$schemaid			= array_get_index($request_info, 'SchemaID', '');
		$grant				= self::array_get_index_restricted_value($request_info, 'Grant', '');	// Apply, Deny, Revoke = 1, 0, -1
		$grant_level		= Array (
								'Apply'		=> '1',
								'Deny'		=> '0',
								'Revoke'	=> '-1',
							  );
		$grant_val			= $grant_level[$grant];	// We are not going check if isset index, the WSDL should check the input values for $grant
		$auto_cascade		= self::getBooleanIndex($request_info, 'AutoCascadeToNewChildren', TRUE);
		$cascade			= self::getBooleanIndex($request_info, 'Cascade', TRUE);

		$dependants			= $GLOBALS['SQ_SYSTEM']->am->getDependantChildren($assetid);
		$has_dependants		= !empty($dependants);

		if (!empty($assetid) && !empty($schemaid)) {
			$schema	= $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
			if (!($schema instanceof Workflow_Schema)) {
				throw new SoapFault('Server', 'The provided schemaid does not belong to a workflow schema asset');
			}//end if

			$hh				= $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$running_vars	= Array (
								'schema_changes'	=> Array (
														Array (
															'assetids'			=> Array ( $assetid),
															'granted'			=> $grant_val,
															'schemaid'			=> $schemaid,
															'cascades'			=> $auto_cascade,
															'previous_access'	=> NULL,
															'dependants_only'	=> !($cascade && $auto_cascade) && $has_dependants,
														),
													   ),
							  );

			$errors	= $hh->freestyleHipo('hipo_job_edit_workflow_schemas', $running_vars);
			if (empty($errors)) {
				return Array (
						'SetWorkflowSchemaResult'	=> TRUE,
					   );
			} else {
				$message = '';
				if (isset($errors[0]['message'])) $message	= $errors[0]['message'];
				throw new SoapFault('Server', 'Unable to set Workflow Schema on Asset. '.$message);
			}//end else
		} else {
			throw new SoapFault('Server', 'Asset ID or Schema ID is not valid. Please provide a valid AssetID and SchemaID');
		}//end else

	}//end SetWorkflowSchema()


	/**
	* getComplexType
	*
	* @return array
	* @access public
	*/
	public static function getComplexElements($func_list=Array())
	{
		$obj_optional			= self::getArgType('AssetObject', 0, 1);
		$grant_type				= self::getArgType('Grant', 1, 1);

		$complex_types	= Array (
							'StartWorkflow'				=> Array (
															'StartWorkflow'						=> Array (
															    									'AssetID'						=> self::$string_non_optional,
																								  ),
															'StartWorkflowResponse'				=> Array (
																									'StartWorkflowResult'			=> self::$boolean_optional,
																								   ),
														   ),
							'CancelWorkflow'			=> Array (
															'CancelWorkflow'					=> Array (
															    									'AssetID'						=> self::$string_non_optional,
																								  ),
															'CancelWorkflowResponse'			=> Array (
																									'CancelWorkflowResult'			=> self::$boolean_optional,
																								   ),
														   ),
							'CompleteWorkflow'			=> Array (
															'CompleteWorkflow'					=> Array (
															    									'AssetID'						=> self::$string_non_optional,
																								  ),
															'CompleteWorkflowResponse'			=> Array (
																									'CompleteWorkflowResult'		=> self::$boolean_optional,
																								   ),
														   ),
							'SetWorkflowSchema'			=> Array (
															'SetWorkflowSchema'					=> Array (
															    									'AssetID'						=> self::$string_non_optional,
																									'SchemaID'          			=> self::$string_non_optional,
																									'Grant'      					=> $grant_type,
																									'AutoCascadeToNewChildren'		=> self::$boolean_optional,
																									'Cascade'						=> self::$boolean_optional,
																								   ),
															'SetWorkflowSchemaResponse'			=> Array (
																									'SetWorkflowSchemaResult'		=> self::$boolean_optional,
																								   ),
														   ),
							'ApproveAssetInWorkflow'	=> Array (
															'ApproveAssetInWorkflow'			=> Array (
																							   		'AssetID'						=> self::$string_non_optional,
																   								   ),
															'ApproveAssetInWorkflowResponse'	=> Array (
																									'ApproveAssetInWorkflowResult'	=> self::$string_optional,
																   								   ),
														   ),
						  );

		$complex_types_available = parent::getComplexElements($complex_types);

		return $complex_types_available;


	}//end getComplexTypes();


	/**
	* getSimpleRestrictedTypes
	*
	* @return array
	* @access public
	*/
	public static function getSimpleRestrictedTypes($func_list=Array())
	{
		$simple_restricted_types	= Array (
							'SetWorkflowSchema'	=> Array (
												'Grant'	=> Array (
																'restriction_base'	=> 'string',
																'enumeration'		=> Array (
																						'Apply', 'Deny', 'Revoke',
																					   ),
															   ),
											   ),
						  );

		$simple_restricted_types_available = parent::processSimpleRestrictedTypes($simple_restricted_types);

		return $simple_restricted_types_available;

	}//end getSimpleRestrictedTypes()


}//end class
?>