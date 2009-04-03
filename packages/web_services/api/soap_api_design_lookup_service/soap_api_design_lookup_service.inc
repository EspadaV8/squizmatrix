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
* $Id: soap_api_design_lookup_service.inc,v 1.1 2009/04/03 00:16:23 hnguyen Exp $
*
*/

check_ssv_licence();
require_once SQ_PACKAGES_PATH.'/web_services/api/soap_api/soap_api.inc';

/**
* Soap_Api_Design_Lookup_Service
*
* Purpose
*
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class Soap_Api_Design_Lookup_Service extends Soap_Api
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
				'ApplyDesign'			=> '1',
				'RemoveDesign'			=> '1',
				'GetDesignFromURL'		=> '1',
				'ApplyAssetPaintLayout'	=> '1',
        		'RemoveAssetPaintLayout'=> '1',
			   );

    }//end getFunctionList()


	/**
	* Description: This operation will apply a design to an asset
	*
    * @param array  $request	The request information
    * <pre>
    * Array (
	*		 'DesignID'	 				=> [The Design ID],
	*		 'AssetID'			 		=> [The AssetID of the asset being applied the design],
	*		 'DesignType'				=> [Type of Design: Frontend, OverrideFrontend, Login, OverrideLogin, UserDefined, OverrideUserDefined],
    *        'UserDefinedDesignName'    => [Name of the new User Defined design],

    *        )
    * </pre>
	*
	* @return string
	* @access public
	*/
	function ApplyDesign($request)
	{
		$request_info				= (Array) $request;
		$new_designid				= array_get_index($request_info, 'DesignID', '');
		$assetid					= array_get_index($request_info, 'AssetID', '');
		$design_type				= array_get_index($request_info, 'DesignType', '');		// ( frontend || user-defined || login ) || override
		$user_defined_design_name	= array_get_index($request_info, 'UserDefinedDesignName', '');

		$design_name	= self::getDesignNameFromType($design_type);
		$designs_ok		= TRUE;
		$design_changed	= FALSE;

		if (!empty($assetid) && !empty($new_designid)) {
			$asset	= $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			$design	= $GLOBALS['SQ_SYSTEM']->am->getAsset($new_designid);

			if (!($design instanceof Design)) {
				throw new SoapFault('Server', 'The DesignID Provided Does Not Belong To A Design Asset');
			}//end if

        	$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
	        $GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

			$update_design_cascade = FALSE;
	        if ($design_name == ('design::user::'.$user_defined_design_name)) {
	        	$old_designid = 0;
	        	if (empty($user_defined_design_name)) {
	        		throw new SoapFault('Server', 'User Defined Design Name is either empty or not valid.');
	        	} else {
	                if ($GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'design', FALSE, $user_defined_design_name)) {
	                	throw new SoapFault('Server', 'Design Name '.$user_defined_design_name.' is already in use');
	                    trigger_localised_error('SYS0134', E_USER_WARNING, $user_defined_design_name);
	                }// end if
	        	}//end else
	        } else {
	        	// Is it not user defined design
				if (in_array($design_type, Array('OverrideUserDefined', 'UserDefined'))) {
		        		$design_name	= $design_name.$user_defined_design_name;
		        }//end if

				$old_design_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'design', FALSE, $design_name);
				$old_designid    = (empty($old_design_link)) ? 0 : $old_design_link['minorid'];

	        }//end else

			if ($new_designid != $old_designid) {

				if ($old_designid) {
                    if (!$asset->deleteLink($old_design_link['linkid'])) {
                        $GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
                        $designs_ok = FALSE;
                        break;
                    }//end if
				}//end if

                if ($new_designid) {
                    $design = $GLOBALS['SQ_SYSTEM']->am->getAsset($new_designid);
                    if (is_null($design)) {
                        $GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
                        $designs_ok = FALSE;
                        break;
                    }//end if

                    if (!$asset->createLink($design, SQ_LINK_NOTICE, $design_name)) {
                        $GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
                        $designs_ok = FALSE;
                        break;
                    }//end if
                }//end if
                $update_design_cascade  = TRUE;
			} else {
				throw new SoapFault('Server', 'The Design Has Already Been Applied To The Asset. Operation Aborted.');
			}//end if
            $designs_changed = TRUE;

	        if ($designs_ok) {
	            if ($designs_changed) {
	                if ($update_design_cascade == TRUE) {
	                    $assets_affected_lookups[] = $asset->id;
	                }//end if
	            }//end if
	            $GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
	        }//end if

	        $GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			$update_lookup_result	= $asset->updateLookups();

			if (!$update_lookup_result) {
				throw new SoapFault('Server', 'Update Lookups For Asset Failed. Try Running Update Lookups Again Another Time');
			}//end if

			return Array (
					'ApplyDesignResult'	=> $designs_changed,
				   );

		} else {
			throw new SoapFault('Server', 'Asset ID is not valid. Please provide a valid Asset ID');
		}//end else

	}//end ApplyDesign()


	/**
	* This function return the according design name for a design type
	*
	* @param $design_type	string	The design type, refer to apply and remove design function
	*
	* @return string
	* @access private
	*/
	private static function getDesignNameFromType($design_type)
	{
		$design_name	= '';
		switch ($design_type) {
			case 'OverrideFrontend':
				$design_name	= 'override::design::system::frontend';
				break;
			case 'OverrideLogin':
				$design_name	= 'override::design::system::login';
				break;
			case 'OverrideUserDefined':
				$design_name	= 'override::design::user::';
				break;
			case 'Frontend':
				$design_name	= 'design::system::frontend';
				break;
			case 'Login':
				$design_name	= 'design::system::login';
				break;
			case 'UserDefined':
				$design_name	= 'design::user::';
				break;
			default:
				break;
		}//end switch
		return $design_name;

	}//end getDesignNameFromType()


	/**
	* Description: This operation will remove a design to an asset
	*
    * @param array  $request	The request information
    * <pre>
    * Array (
	*		 'DesignID'	 				=> [The Design ID],
	*		 'AssetID'			 		=> [The AssetID of the asset being applied the design],
	*		 'DesignType'				=> [Type of Design: Frontend, OverrideFrontend, Login, OverrideLogin, UserDefined, OverrideUserDefined],
    *        'UserDefinedDesignName'    => [Name of the new User Defined design],

    *        )
    * </pre>
	*
	* @return void
	* @access public
	*/
	function RemoveDesign($request)
	{
		$request_info				= (Array) $request;
		$design_id					= array_get_index($request_info, 'DesignID', '');
		$assetid					= array_get_index($request_info, 'AssetID', '');
		$design_type				= array_get_index($request_info, 'DesignType', '');		// ( frontend || user-defined || login ) || override
		$user_defined_design_name	= array_get_index($request_info, 'UserDefinedDesignName', '');


		if (!empty($assetid) && !empty($design_id)) {
			$asset	= $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			$design	= $GLOBALS['SQ_SYSTEM']->am->getAsset($design_id);

			$design_name	= self::getDesignNameFromType($design_type);
			$designs_ok		= TRUE;

			if (!($design instanceof Design)) {
				throw new SoapFault('Server', 'The DesignID Provided Does Not Belong To A Design Asset');
			}//end if

        	$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
	        $GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

	        	// Is it not user defined design
				if (in_array($design_type, Array('OverrideUserDefined', 'UserDefined'))) {
		        		$design_name	= $design_name.$user_defined_design_name;
		        }//end if

				$old_design_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'design', FALSE, $design_name);
				$old_designid    = (empty($old_design_link)) ? 0 : $old_design_link['minorid'];

				if ($old_designid && $old_designid == $design_id) {
                    if (!$asset->deleteLink($old_design_link['linkid'])) {
                        $GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
                        $designs_ok = FALSE;
                        break;
                    }//end if
				} else {
					throw new SoapFault('Server', 'No Such Design Found For Asset');
				}//end else

	        if ($designs_ok) {
	            $GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
	        }//end if

			$update_lookup_result	= $asset->updateLookups();

			if (!$update_lookup_result) {
				throw new SoapFault('Server', 'Update Lookups For Asset Failed. Try Running Update Lookups Again Another Time');
			}//end if

			return Array (
					'RemoveDesignResult'	=> $designs_ok,
				   );

		} else {
			throw new SoapFault('Server', 'Asset ID is not valid. Please provide a valid Asset ID');
		}//end else

	}//end RemoveDesign()


	/**
	* Description: This operation returns the effective design of an URL
	*
	* @param array $asset_info the info array to find children of a specific asset
    * <pre>
    * Array (
    *		'URL'					=> [The URL in question],
	*		'DesignType'			=> [Type of Design: Frontend, OverrideFrontend, Login, OverrideLogin, UserDefined, OverrideUserDefined],
	*		'UserDefinedDesignName'	=> [Name of the new User Defined design],
    *        )
    * </pre>
	*
	* @return void
	* @access public
	*/
	function GetDesignFromURL($request)
	{
		$request_info				= (Array) $request;
		$url						= array_get_index($request_info, 'URL', '');
		$design_type				= array_get_index($request_info, 'DesignType', '');		// ( frontend || user-defined || login ) || override
		$user_defined_design_name	= array_get_index($request_info, 'UserDefinedDesignName', '');

		$design_name	= self::getDesignNameFromType($design_type);
        // Is it not user defined design
		if (in_array($design_type, Array('OverrideUserDefined', 'UserDefined'))) {
        		$design_name	= $design_name.$user_defined_design_name;
        }//end if

		if (empty($design_name)) {
			$design_name	= 'design::system::frontend';
		}//end if

		$designs		= $GLOBALS['SQ_SYSTEM']->am->getDesignFromURL($url, $design_name);
		$design_result	= Array (
							'DesignID'	=> $designs['designid'],
							'TypeCode'	=> $designs['type_code'],
						  );

		return Array (
				'GetDesignFromURLResult'	=> $design_result,
			   );

	}//end GetDesignFromURL()


	/**
	* Description: This operation will apply a paintlayout to an asset
	*
	* @param array $request		the request information
    * <pre>
    * Array (
    *		'PaintLayoutID'		=> [The assetid of the paintlayout],
	*		'AssetID'			=> [The asset being applied paintlayout to],
	*		'PaintLayoutType'	=> [Type of paint layout Frontend or OverrideFrontend],
    *        )
    * </pre>
	*
	* @return void
	* @access public
	*/
	function ApplyAssetPaintLayout($request)
	{
		$request_info				= (Array) $request;
		$paint_layout_id			= array_get_index($request_info, 'PaintLayoutID', '');
		$assetid					= array_get_index($request_info, 'AssetID', '');
		$paint_layout_type			= array_get_index($request_info, 'PaintLayoutType', '');		// ( Frontend | OverrideFrontend )

		if (!empty($assetid)) {
			$asset	= $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			if (empty($paint_layout_id)) {
				throw new SoapFault('Server', 'The PaintLayoutID provided is invalid');
			}//end if
			$result	= self::changePaintLayout($asset, $paint_layout_id, $paint_layout_type);
			return Array (
					'ApplyAssetPaintLayoutResult'	=> $result,
				   );

		} else {
			throw new SoapFault('Server', 'The AssetID supplied is either empty or not valid');
		}//end else

	}//end ApplyPaintLayout()


	/**
	* Description: This operation will remove a paintlayout to an asset
	*
	* @param array $request		the request information
    * <pre>
    * Array (
    *		'PaintLayoutID'		=> [The assetid of the paintlayout],
	*		'AssetID'			=> [The asset being remove paintlayout from],
	*		'PaintLayoutType'	=> [Type of paint layout Frontend or OverrideFrontend],
    *        )
    * </pre>
	*
	* @return void
	* @access public
	*/
	function RemoveAssetPaintLayout($request)
	{
		$request_info				= (Array) $request;
		$paint_layout_id			= array_get_index($request_info, 'PaintLayoutID', '');
		$assetid					= array_get_index($request_info, 'AssetID', '');
		$paint_layout_type			= array_get_index($request_info, 'PaintLayoutType', '');		// ( Frontend | OverrideFrontEnd )

		if (!empty($assetid)) {
			$asset	= $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			// We should check whether they are telling the truth
			$paint_layout_end_name	= '::system::frontend';
			$value_name				= $paint_layout_type == 'Frontend' ? 'paint_layout' : 'override::paint_layout';
			$paint_layout_name		= $value_name.$paint_layout_end_name;
			$existing_layout_link	= $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'paint_layout_page', TRUE, $paint_layout_name);
			if (empty($existing_layout_link) || ($existing_layout_link['minorid'] != $paint_layout_id)) {
				throw new SoapFault('Server', 'The Asset Does Not Have A PaintLayout Applied With The Provided PaintLayoutID');
			}//end if

			$result	= self::changePaintLayout($asset, 0, $paint_layout_type);
			return Array (
					'RemoveAssetPaintLayoutResult'	=> $result,
				   );
		} else {
			throw new SoapFault('Server', 'The AssetID supplied is either empty or not valid');
		}//end else
	}//end RemoveAssetPaintLayout()


	/**
	* This function will apply paint layout to an asset, refer to Lookup Setting screen
	*
	* @param $asset				object	The asset being applied paintlayout
	* @param $paint_layout_id	string	The assetid of the paintlayout
	* @param $paint_layout_type	string	The paint layout type
	*
	* @return boolean
	* @access private
	*/
	private static function changePaintLayout($asset, $paint_layout_id, $paint_layout_type)
	{
		$paint_layout			= NULL;
		if ($paint_layout_id != 0) {
			$paint_layout			= $GLOBALS['SQ_SYSTEM']->am->getAsset($paint_layout_id);
			if (!($paint_layout instanceof Paint_Layout_Page)) {
				throw new SoapFault('Server', 'The PaintLayoutID Provided Does Not Belong To A Paint Layout Page Asset');
			}//end if
		}//end if
		$paint_layout_end_name	= '::system::frontend';
		$value_name				= $paint_layout_type == 'Frontend' ? 'paint_layout' : 'override::paint_layout';
		$paint_layout_name		= $value_name.$paint_layout_end_name;

	    $GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
	    $GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$GLOBALS['SQ_SYSTEM']->am->acquireLock($asset->id, 'all');

		$existing_layout_link	= $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, 'paint_layout_page', TRUE, $paint_layout_name);
		$lookup_values			= $asset->getLookupValues(FALSE);
		$layouts_changed		= FALSE;
		$layouts_ok				= TRUE;
		$values					= Array(
									'asset'	=> Array (
												'paint_layout'				=> Array (
																				'value'	=> '',
																			   ),
												'override::paint_layout'	=> Array (
																				'value'	=> '',
																			   ),
											   ),
								  );

		$create_new	= $delete_existing	= FALSE;
		if (empty($existing_layout_link)) {
			$create_new	= TRUE;
			// We are creating new layout notice link
		} else if ($existing_layout_link['minorid'] !== $paint_layout_id) {
			$delete_existing	= TRUE;
			$create_new			= TRUE;
		}//end else

		if ($delete_existing) {
			if ($existing_layout_link) {
				if (!$asset->deleteLink($existing_layout_link['linkid'])) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$layouts_ok = FALSE;
					break;
				} else {
					$layouts_changed = TRUE;
				}//end if

				// also need to delete any other URLs which use the same value
				// deleting an asset based value means all other URLs which use the same value are also deleted
				if (!$new_values = $asset->deleteLookupValue($paint_layout_name, $existing_layout_link['minorid'])) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$layouts_ok = FALSE;
					break;
				} else {
					$lookup_values = $new_values;
					unset($new_values);
				}//end if
			}//end if
		}//end if

		if ($create_new) {
		    // create new notice link
		    if (is_null($paint_layout)) {
		        $GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		        $layouts_ok = FALSE;
		    } else if (!$asset->createLink($paint_layout, SQ_LINK_NOTICE, $paint_layout_name)) {
		        $GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		        $layouts_ok = FALSE;
		        break;
		    } else {
		        $layouts_changed = TRUE;
		    }//end else
		}//end if
		// delete if no value

		if (empty($paint_layout_id)) {
		    if (isset($lookup_values['asset'][$value_name])) {
		        unset($lookup_values['asset'][$value_name]);
		    }//end if

		    // if value is deleted, run updateLookups() so the asset based definition can take effect if there is one defined
		    $layouts_changed = TRUE;
		} else {
		    if (!isset($lookup_values['asset'])) {
		        $lookup_values['asset'] = Array();
		    }//end if
		    $lookup_values['asset'][$value_name] = Array('value' => $paint_layout_id);
		}//end else

		if (!$asset->setLookupValues($lookup_values)) {
		    $GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		    $GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		    return FALSE;
		}//end if

		if ($layouts_ok) {
		    if ($layouts_changed) {
		        if (!$asset->updateLookups()) {
		            $GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		            $GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		            return FALSE;
		        }//end if
		    }//end if
		}//end if

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		$GLOBALS['SQ_SYSTEM']->am->releaseLock($asset->id, 'all');

		return TRUE;

	}//end changePaintLayout()


	/**
	* getComplexElements
	*
	* @return array
	* @access public
	*/
	public static function getComplexElements($func_list=Array())
	{
		$obj_optional	= self::getArgType('AssetObject', 0, 1);
		$design_link_summary	= self::getArgType('DesignLinkSummary', 0, 1);

		$complex_types	= Array (
							'ApplyDesign'			=> Array (
														'ApplyDesign'				=> Array (
																						'DesignID'				=> self::$string_non_optional,
																						'AssetID'				=> self::$string_non_optional,
																						'DesignType'			=> self::$string_non_optional,
																						'UserDefinedDesignName'	=> self::$string_optional,
																					   ),
														'ApplyDesignResponse'		=> Array (
																						'ApplyDesignResult'		=> self::$boolean_optional,
																					   ),
													   ),
							'RemoveDesign'			=> Array (
														'RemoveDesign'				=> Array (
																						'DesignID'				=> self::$string_non_optional,
																						'AssetID'				=> self::$string_non_optional,
																						'DesignType'			=> self::$string_optional,
														   								'UserDefinedDesignName'	=> self::$string_optional,
																					   ),
														'RemoveDesignResponse'		=> Array (
																						'RemoveDesignResult'	=> self::$boolean_optional,
																					   ),
													   ),
							'GetDesignFromURL'		=> Array (
														'GetDesignFromURL'			=> Array (
																						'URL'					=> self::$string_non_optional,
																						'DesignType'			=> self::$string_optional,
														   								'UserDefinedDesignName'	=> self::$string_optional,
																					   ),
														'GetDesignFromURLResponse'	=> Array (
																						'GetDesignFromURLResult'	=> $design_link_summary,
																					   ),
													   ),
							'ApplyAssetPaintLayout'	=> Array (
														'ApplyAssetPaintLayout'		=> Array (
																						'PaintLayoutID'			=> self::$string_non_optional,
													   									'AssetID'				=> self::$string_optional,
													   									'PaintLayoutType'		=> self::$string_non_optional,
													   								   ),

														'ApplyAssetPaintLayoutResponse'	=> Array (
																							'ApplyAssetPaintLayoutResult'	=> self::$boolean_optional,
													   								   ),
													   ),
							'RemoveAssetPaintLayout'	=> Array (
														'RemoveAssetPaintLayout'	=> Array (
																						'PaintLayoutID'			=> self::$string_non_optional,
													   									'AssetID'				=> self::$string_optional,
													   									'PaintLayoutType'		=> self::$string_non_optional,
													   								   ),

														'RemoveAssetPaintLayoutResponse'	=> Array (
																						'RemoveAssetPaintLayoutResult'	=> self::$boolean_optional,
													   								   ),
													   ),

							'ApplyURLPaintLayout'	=> Array (
														'ApplyURLPaintLayout'		=> Array (

													   								   ),

														'ApplyURLPaintLayoutResponse'	=> Array (
																						'ApplyURLPaintLayoutResult'	=> self::$boolean_optional,
													   								   ),
													   ),
						  );

		$complex_types_available = parent::getComplexElements($complex_types);

		return $complex_types_available;

	}//end getComplexTypes();


    /**
    * getComplexTypes: return all the complex types used by the API
    *
    * @return array
    * @access public
    */
    public static function getComplexTypes($func_list=Array())
    {
        $complex_types    = Array (
							'GetDesignFromURL'		=> Array (
                                           			     	'DesignLinkSummary'			=> Array (
																							'DesignID'			=> self::$string_non_optional,
                                           			       									'TypeCode'			=> self::$string_optional,
                                           			       								   ),
                                           			   ),
                            'GetSchemasOnAsset'    => Array (
															'SchemaSetInfo'				=> Array (
																						   	'SchemaID'			=> self::$string_non_optional,
																							'Granted'			=> self::$int_non_optional,
																						   ),
                                           			       ),
                          );

        $complex_types_available = parent::getComplexElements($complex_types);

        return $complex_types_available;

    }//end getComplexTypes()


	/**
	* getSimpleRestrictedTypes
	*
	* @return array
	* @access public
	*/
	public static function getSimpleRestrictedTypes($func_list=Array())
	{
		$design_type	= Array (
							'restriction_base'	=> 'EnumType',
							'enumeration'		=> Array (
													'Frontend', 'OverrideFrontend', 'Login', 'OverrideLogin', 'UserDefined', 'OverrideUserDefined',
												   ),
						  );

		$simple_restricted_types	= Array (
							'ApplyDesign'		=> Array (
													'DesignType'	=> $design_type,
												   ),
							'RemoveDesign'		=> Array (
													'DesignType'	=> $design_type,
												   ),
							'GetDesignFromURL'	=> Array (
													'DesignType'	=> $design_type,
												   ),
						  );

		$simple_restricted_types_available = parent::getSimpleRestrictedTypes($simple_restricted_types);

		return $simple_restricted_types_available;

	}//end getSimpleRestrictedTypes()


}//end class
?>