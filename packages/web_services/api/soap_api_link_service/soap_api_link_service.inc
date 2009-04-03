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
* $Id: soap_api_link_service.inc,v 1.1 2009/04/02 23:55:00 hnguyen Exp $
*
*/

check_ssv_licence();
require_once SQ_PACKAGES_PATH.'/web_services/api/soap_api/soap_api.inc';

/**
* Soap_Api_Link_Service
*
* Purpose
*
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class Soap_Api_Link_Service extends Soap_Api
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
				'CreateLink'			=> '1',
				'DeleteLink'			=> '1',
				'GetAllChildLinks'		=> '1',
				'GetChildren'			=> '1',
				'GetDependantChildren'	=> '1',
				'GetDependantParents'	=> '1',
				'GetLinks'				=> '1',
        		'GetLinkByAsset'		=> '1',
				'GetParents'			=> '1',
				'MoveLink'				=> '1',
				'UpdateLink'			=> '1',
			   );

    }//end getFunctionList()


	/**
	* Description: This operation will create link between two assets
	*
    * @param string  $request  The request information
    *
    * <pre>
    * Array (
	*		'MajorID'		=> [The asset id of the major asset],
	*		'MinorID'		=> [The asset id of the major asset],
	* 		'LinkType'		=> [The link type being created],
	* 		'LinkValue'		=> [Value of the new link],
	* 		'SortOrder'		=> [Minor Asset position under the Major],
	* 		'IsDependant'	=> [Whether the minor asset is dependent on the major]
	*		'IsExclusive'	=> [Whether the major asset is the minor's only parent]
    *        )
    * </pre>
	*
	* @return void
	* @access public
	*/
	function CreateLink($request)
	{
		$request_info 	= (Array) $request;
		$majorid		= array_get_index($request_info, 'MajorID', '');
		$minorid		= array_get_index($request_info, 'MinorID', '');
		$link_type		= array_get_index($request_info, 'LinkType', '1');
		$link_value		= array_get_index($request_info, 'LinkValue', '');
		$sort_order		= self::getNullIndex($request_info, 'SortOrder', NULL);
		$is_dependant	= self::getBooleanIndex($request_info, 'IsDependant', 0);
		$is_exclusive	= self::getBooleanIndex($request_info, 'IsExclusive', 0);

		if (!empty($majorid) && !empty($minorid) && !empty($link_type)) {
			$major	= $GLOBALS['SQ_SYSTEM']->am->getAsset($majorid);
			$minor	= $GLOBALS['SQ_SYSTEM']->am->getAsset($minorid);
			$new_link_id	= $GLOBALS['SQ_SYSTEM']->am->createAssetLink($major, $minor, $link_type, $link_value, $sort_order, $is_dependant, $is_exclusive);
			if ($new_link_id == 0) {
				throw new SoapFault('Server', 'Unable to create link');
			}//end if
			return Array (
					'CreateLinkResult'	=> $new_link_id,
				   );
		} else {
			throw new SoapFault('Server', 'Missing information to create link. Make sure MajorID, MinorID and LinkType is provided');
		}//end else

	}//end CreateLink()


	/**
	* Description: This operation will delete a link based on the LinkID
	*
    * @param array  $request	The request information
    * <pre>
    * Array (
	*		 'LinkID'	 => [The link ID to be deleted],
    *        )
    * </pre>
	*
	* @return void
	* @access public
	*/
	function DeleteLink($request)
	{
		$request_info = (Array) $request;
		$linkid = array_get_index($request_info, 'LinkID', '');

		if (!empty($linkid)) {
			$link	= $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid);
			if (!empty($link)) {
				$GLOBALS['SQ_SYSTEM']->am->acquireLock($link['majorid'], 'all');
				$GLOBALS['SQ_SYSTEM']->am->acquireLock($link['minorid'], 'all');
				$significant_parent_links	= $GLOBALS['SQ_SYSTEM']->am->getLinks($link['minorid'], SQ_SC_LINK_SIGNIFICANT, '', TRUE, 'minor');
				if (count($significant_parent_links) > 1) {
					// More than one parent link, we can delete the link
					$result	= $GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($linkid);
				} else {
					// Only one link holly cow, lets trash it instead, otherwise we'l have an orphan asset in the system
					$result	= $GLOBALS['SQ_SYSTEM']->am->trashAsset($link['minorid']);
				}//end if

				$GLOBALS['SQ_SYSTEM']->am->releaseLock($link['minorid'], 'all');
				$GLOBALS['SQ_SYSTEM']->am->releaseLock($link['majorid'], 'all');

				if ($result) {
					return Array (
							'DeleteLinkResult'	=> $result,
						   );
				} else {
					throw new SoapFault('Server', 'Unable To Delete Link');
				}//end else
			} else {
				throw new SoapFault('Server', 'There no such link with the Link ID provided');
			}//end

		} else {
			throw new SoapFault('Server', 'The Link ID Provided Is Empty');
		}//end else

	}//end DeleteLink()


	/**
	* Description: This operation will return all child links
	*
    * @param array  $request	The request information
    * <pre>
    * Array (
	*		'AssetID'	=> [The Asset to find links under],
	*		'LinkType'	=> [The type of link]
    *        )
    * </pre>
	*
    * @param string  $assetid  the id of the asset being deleted
	*
	* @return void
	* @access public
	*/
	function GetAllChildLinks($request)
	{
		$request_info	= (Array) $request;
		$assetid		= array_get_index($request_info, 'AssetID', '');
		$link_type		= array_get_index($request_info, 'LinkType', 0);
		if (!empty($assetid)) {
			$all_child_links	= $GLOBALS['SQ_SYSTEM']->am->getAllChildLinks($assetid, $link_type);
			$link_results		= Array();
			$i	= 0;
			foreach ($all_child_links as $treeid => $link_info) {
				$link_results[$i]['TreeID']				= $treeid;
				$link_results[$i]['Level']				= $link_info['lvl'];
				$link_results[$i]['LinkID']				= $link_info['linkid'];
				$link_results[$i]['AssetID']			= $link_info['assetid'];
				$link_results[$i]['TypeCode']			= $link_info['type_code'];
				$link_results[$i]['LinkType']			= $link_info['link_type'];
				$link_results[$i]['SortOrder']			= $link_info['sort_order'];
				$link_results[$i]['Value']				= $link_info['value'];
				$link_results[$i]['IsDependant']		= $link_info['is_dependant'];
				$link_results[$i]['IsExclusive']		= $link_info['is_exclusive'];
				$link_results[$i]['DependantTreeID']	= $link_info['dependant_treeid'];
				$i++;
			}//end foreach

			return Array (
					'GetAllChildLinksResult'	=> $link_results,
				   );
		} else {
			throw new SoapFault('Server', 'Asset ID is not valid. Please provide a valid Asset ID');
		}//end else

	}//end GetAllChildLinks()


	/**
	* Description: This operation will return dependant children of the specified criteria
	*
    * @param array  $request	The request information
    * <pre>
    * Array (
	*		'AssetID'			=> [The Asset to find dependant children for],
	*		'TypeCode'			=> [The desired type code of dependants],
	*		'StrictTypeCode'	=> [Strict Type Code, TRUE | FALSE],
    *        )
    * </pre>
	*
	* @return void
	* @access public
	*/
	function GetDependantChildren($request)
	{
		$request_info		= (Array) $request;
		$assetid			= array_get_index($request_info, 'AssetID', '');
		$type_code			= array_get_index($request_info, 'TypeCode', '');
		$strict_type_code	= self::getBooleanIndex($request_info, 'StrictTypeCode', TRUE);

		if (!empty($assetid)) {
			$dependant_children	= $GLOBALS['SQ_SYSTEM']->am->getDependantChildren($assetid, $type_code, $strict_type_code);
			$dependant_results	= Array();
			$i	= 0;
			foreach ($dependant_children as $dependant_assetid	=> $type_code_info) {
				$dependant_results[$i]['AssetID']	= $dependant_assetid;
				$dependant_results[$i]['TypeCode']	= $type_code_info[0]['type_code'];
				$i++;
			}//end foreach

			return Array(
					'GetDependantChildrenResult'	=> $dependant_results,
				   );
		} else {
			throw new SoapFault('Server', 'Asset ID is not valid. Please provide a valid Asset ID');
		}//end else
	}//end GetDependantChildren()


	/**
	* Description: This operation will return dependant parents of the specified criteria
	*
    * @param array  $request	The request information
    * <pre>
    * Array (
	*		'AssetID'				=> [The Asset to find dependant children for],
	*		'TypeCode'				=> [The desired type code of dependants],
	*		'StrictTypeCode'		=> [Strict Type Code, TRUE | FALSE],
	*		'IncludeAllDependants'	=> [Whether to include all dependants or only top level dependant parents(FALSE)]
    *        )
    * </pre>
	*
	* @return void
	* @access public
	*/
	function GetDependantParents($request)
	{
		$request_info		= (Array) $request;
		$assetid			= array_get_index($request_info, 'AssetID', '');
		$type_code			= array_get_index($request_info, 'TypeCode', '');
		$strict_type_code	= self::getBooleanIndex($request_info, 'StrictTypeCode', TRUE);
		$include_all_dependants	= self::getBooleanIndex($request_info, 'IncludeAllDependants', TRUE);

		$dependant_results = Array();
		if (!empty($assetid)) {
			$dependant_parents	= $GLOBALS['SQ_SYSTEM']->am->getDependantParents($assetid, $type_code, $strict_type_code, $include_all_dependants);
			$i	= 0;
			foreach ($dependant_parents as $dependant_assetid) {
				$dependant_results[$i]['AssetID']	= $dependant_assetid;
				$i++;
			}//end foreach

			return Array(
					'GetDependantParentsResult'	=> $dependant_results,
				   );
		} else {
			throw new SoapFault('Server', 'Asset ID is not valid. Please provide a valid Asset ID');
		}//end else

	}//end GetDependantParents()


	/**
	* Description: This operation will return dependant parents of the specified criteria
	*
    * @param array  $request	The request information
    * <pre>
    * Array (
	*		'AssetID'				=> [The Asset to find links for],
	*		'LinkType'				=> [Desired link type]
	*		'TypeCode'				=> [The desired type code of linked assets],
	*		'StrictTypeCode'		=> [Strict Type Code, TRUE | FALSE],
	*		'SideOfLink'			=> [Major or Minor link],
	*		'LinkValue'				=> [Value of the link],
	*		'Dependant'				=> [If not null, either return non-depandant or dependant, if is null return both],
	*		'Exclusive'				=> [If not null, either return non-exclusive or exclusive, if is null return both],
	*		'SortBy'				=> [Sort By a field in asset table],
	*		'PermissionLevel'		=> [Read, Write, Admin],
	*		'Effective'				=> [If effective permissions should be considered or not],
    *        )
    * </pre>
	*
	* @return void
	* @access public
	*/
	function GetLinks($request)
	{
		$request_info		= (Array) $request;
		$assetid			= array_get_index($request_info, 'AssetID', '');
		$link_type			= array_get_index($request_info, 'LinkType', 1);
		$type_code			= array_get_index($request_info, 'TypeCode', '');
		$strict_type_code	= self::getBooleanIndex($request_info, 'StrictTypeCode', TRUE);
		$side_of_link		= array_get_index($request_info, 'SideOfLink', 'major');
		$value				= self::getNullIndex($request_info, 'LinkValue', NULL);
		$dependant			= self::getNullIndex($request_info, 'Dependant', NULL);
		$exclusive			= self::getNullIndex($request_info, 'Exclusive', NULL);
		$sort_by				= self::getNullIndex($request_info, 'SortBy', NULL);
		$access				= self::getNullIndex($request_info, 'PermissionLevel', NULL);
		$effective			= self::getBooleanIndex($request_info, 'Effective', TRUE);

		$access				= self::getPermValue($access);
		if (!empty($assetid)) {
			$links	= $GLOBALS['SQ_SYSTEM']->am->getLinks($assetid, $link_type, $type_code, $strict_type_code, $side_of_link, $value, $dependant, $exclusive, $sort_by, $access, $effective);

			if (!empty($links)) {
				$link_results	= Array();
				$other_side_of_link	= $side_of_link == 'major' ? 'minor' : 'major';
				$i = 0;
				foreach ($links as $link_info) {
					$link_results[$i][ucwords($other_side_of_link).'ID']		= $link_info[$other_side_of_link.'id'];
					$link_results[$i]['LinkID']			= $link_info['linkid'];
					$link_results[$i]['Value']			= $link_info['value'];
					$link_results[$i]['LinkType']		= $link_info['link_type'];
					$link_results[$i][ucwords($other_side_of_link).'TypeCode']	= $link_info[$other_side_of_link.'_type_code'];
					$link_results[$i]['SortOrder']		= $link_info['sort_order'];
					$link_results[$i]['IsDependant']	= $link_info['is_dependant'];
					$link_results[$i]['IsExclusive']	= $link_info['is_exclusive'];
					$i++;
				}//end foreach
				return Array (
						'GetLinksResult'	=> $link_results,
					   );
			} else {
				throw new SoapFault('Server', 'There is no links for the specified criterions');
			}//end else
		} else {
			throw new SoapFault('Server', 'Asset ID is not valid. Please provide a valid Asset ID');
		}//end else

	}//end GetLinks()


	/**
	* Description: This operation will move a link under a new parent asset
	*
    * @param array  $request	The request information
    * <pre>
    * Array (
	*		'LinkID'				=> [The ID of the link],
	*		'LinkType'				=> [Desired link type]
	*		'ToParentID'			=> [New parent of the minor asset of the link],
	*		'ToParentPosition'		=> [The position under the new parent],
    *        )
    * </pre>
	*
	* @return void
	* @access public
	*/
	function MoveLink($request)
	{
		$request_info		= (Array) $request;
		$linkid				= array_get_index($request_info, 'LinkID', '');
		$link_type			= array_get_index($request_info, 'LinkType', 0);
		$to_parent_id		= array_get_index($request_info, 'ToParentID', 0);
		$to_parent_position	= array_get_index($request_info, 'ToParentPosition', 0);

		if (!empty($linkid) && !empty($to_parent_id)) {
			$new_link_id	= $GLOBALS['SQ_SYSTEM']->am->moveLink($linkid, $to_parent_id, $link_type, $to_parent_position);

			if (!empty($new_link_id)) {
				return Array (
						'MoveLinkResult'	=> $new_link_id,
					   );
			} else {
				throw new SoapFault('Server', 'Unable To Move Link');
			}//end else
		} else {
			throw new SoapFault('Server', 'Either Link ID or ToParentID is invalid. Please provide valid LinkID and ToParentID');
		}//end else

	}//end MoveLink()


	/**
	* Description: This operation will update an existing link
	*
    * @param array  $request	The request information
    * <pre>
    * Array (
	*		'LinkID'				=> [The ID of the link],
	*		'LinkType'				=> [Desired link type]
	*		'LinkValue'				=> [The Value of the link],
	*		'SortOrder'				=> [The position in the links list that this link should take, if less than zero places at end of list],
    *        )
    * </pre>
	*
	* @return void
	* @access public
	*/
	function UpdateLink($request)
	{
		$request_info	= (Array) $request;
		$linkid			= array_get_index($request_info, 'LinkID', '');
		$link_type		= self::getNullIndex($request_info, 'LinkType', NULL);
		$link_value		= self::getNullIndex($request_info, 'LinkValue', NULL);
		$sort_order		= self::getNullIndex($request_info, 'SortOrder', NULL);

		if (!empty($linkid)) {
			$new_link_id	= $GLOBALS['SQ_SYSTEM']->am->updateLink($linkid, $link_type, $link_value, $sort_order);

			if (!empty($new_link_id)) {
				return Array (
						'UpdateLinkResult'	=> $new_link_id,
					   );
			} else {
				throw new SoapFault('Server', 'Unable To Update Link');
			}//end else
		} else {
			throw new SoapFault('Server', 'Link ID is not valid. Please provide a valid Link ID');
		}//end else

	}//end UpdateLink()


	/**
	* Description: This function returns the children of a specific asset base on criterions
	*
	* @param array $asset_info the info array to find children of a specific asset
    * <pre>
    * Array (
    *		'AssetID'			=> [assetid of node],
	*		'TypeCode'			=> [type code of the chilren asset],
	*		'StrictTypeCode'	=> [strict type code or not],
	*		'Dependant'			=> [dependant or not],
	*		'SortBy'			=> [asset attribute],
	*		'PermissionLevel'	=> [Read, Write, Admin],
	*		'EffectiveAccess'	=> [effective permission],
	*		'MinDepth'			=> [minimum child level],
	*		'MaxDepth'			=> [maximum child level],
	*		'DirectShadowsOnly'	=> [whether to get only shadow asset],
    *		'LinkValueWanted'	=> [link value],
    *        )
    * </pre>
	*
	* @return void
	* @access public
	*/
	function GetChildren($request)
	{
		$request_info		= (Array) $request;
		$assetid			= array_get_index($request_info, 'AssetID', '');
		$type_code			= array_get_index($request_info, 'TypeCode', '');
		$strict_type_code	= self::getBooleanIndex($request_info, 'StrictTypeCode', TRUE);
		$dependant			= self::getNullIndex($request_info, 'Dependant', NULL);
		$sort_by			= self::getNullIndex($request_info, 'SortBy', NULL);
		$access				= self::getNullIndex($request_info, 'PermissionLevel', NULL);
		$effective			= self::getBooleanIndex($request_info, 'EffectiveAccess', TRUE);
		$min_depth			= self::getNullIndex($request_info, 'MinDepth', NULL);
		$max_depth			= self::getNullIndex($request_info, 'MaxDepth', NULL);
		$direct_shadow_only	= self::getBooleanIndex($request_info, 'DirectShadowsOnly', TRUE);
		$link_value_wanted	= self::getNullIndex($request_info, 'LinkValueWanted', NULL);

		$access				= self::getPermValue($access);
		if (!empty($assetid)) {
			$children	= $GLOBALS['SQ_SYSTEM']->am->getChildren($assetid, $type_code, $strict_type_code, $dependant, $sort_by, $access, $effective, $min_depth, $max_depth, $direct_shadow_only, $link_value_wanted);
			$children_result	= Array();
			$i	= 0;
			foreach ($children as $child_assetid	=> $child_type_code_info) {
				if (!is_null($sort_by)) {
					$children_result[$i]['AssetID']		= $child_assetid;
					$children_result[$i]['TypeCode']	= $child_type_code_info;
				} else {
					$children_result[$i]['AssetID']		= $child_assetid;
					$children_result[$i]['TypeCode']	= $child_type_code_info[0]['type_code'];
				}//end if
				$i++;
			}//end foreach

			return Array (
					'GetChildrenResult'	=> $children_result,
				   );
		} else {
			throw new SoapFault('Server', 'Asset ID is not valid. Please provide a valid Asset ID');
		}//end else

	}//end GetChildren()


	/**
	* Description: This function returns all the parent of a specific asset
	*
	* @param array $asset_info the info array to find children of a specific asset
    * <pre>
    * Array (
    *		'AssetID'			=> [assetid of node],
	*		'TypeCode'			=> [type code of the chilren asset],
	*		'StrictTypeCode'	=> [strict type code or not],
	*		'Dependant'			=> [dependant or not],
	*		'SortBy'			=> [asset attribute],
	*		'PermissionLevel'	=> [Read, Write, Admin],
	*		'EffectiveAccess'	=> [effective permission],
	*		'MinDepth'			=> [minimum child level],
	*		'MaxDepth'			=> [maximum child level],
	*		'DirectShadowsOnly'	=> [whether to get only shadow asset],
    *		'LinkValueWanted'	=> [link value],
    *        )
    * </pre>
	*
	* @return void
	* @access public
	*/
	function GetParents($request)
	{
		$request_info		= (Array) $request;
		$assetid			= array_get_index($request_info, 'AssetID', '');
		$type_code			= array_get_index($request_info, 'TypeCode', '');
		$strict_type_code	= self::getBooleanIndex($request_info, 'StrictTypeCode', TRUE);
		$sort_by			= self::getNullIndex($request_info, 'SortBy', NULL);
		$access				= self::getNullIndex($request_info, 'PermissionLevel', NULL);
		$effective			= self::getBooleanIndex($request_info, 'EffectiveAccess', TRUE);
		$min_depth			= self::getNullIndex($request_info, 'MinDepth', NULL);
		$max_depth			= self::getNullIndex($request_info, 'MaxDepth', NULL);
		$ignore_bridge		= self::getBooleanIndex($request_info, 'DirectShadowsOnly', FALSE);
		$link_value_wanted	= self::getNullIndex($request_info, 'LinkValueWanted', NULL);

		$access				= self::getPermValue($access);
		if (!empty($assetid)) {
			$parents	= $GLOBALS['SQ_SYSTEM']->am->getParents($assetid, $type_code, $strict_type_code, $sort_by, $access, $effective, $min_depth, $max_depth, $ignore_bridge, $link_value_wanted);
			$parents_result	= Array();
			$i	= 0;
			foreach ($parents as $parent_assetid	=> $parent_type_code) {
				$parents_result[$i]['AssetID']	= $parent_assetid;
				$parents_result[$i]['TypeCode']	= $parent_type_code;
				$i++;
			}//end foreach
			if (!empty($parents)) {
				return Array (
						'GetParentsResult'	=> $parents_result,
					   );
			} else {
				throw new SoapFault('Server', 'Unable To Get Parents');
			}//end else
		} else {
			throw new SoapFault('Server', 'Asset ID is not valid. Please provide a valid Asset ID');
		}//end else

	}//end findStatus()


	/**
	* Description: This function returns all the parent of a specific asset
	*
	* @param array $asset_info the info array to find children of a specific asset
    * <pre>
    * Array (
    *		'AssetID'			=> [assetid of node],
	*		'OtherAssetID'		=> [type code of the asset on the other side of the link],
	*		'LinkType'			=> [Desired link type],
	*		'LinkValue'			=> [The Value of the link],
	*		'SideOfLink'		=> [Which side of the link the first assetid is on, Major or Minor],
	*		'IsDependant'		=> [The dependant status for all the links must be this (if not null)],
	*		'IsExclusive'		=> [The exclusive status for all the links must be this (if not null)],
    *        )
    * </pre>
	*
	* @return void
	* @access public
	*/
	function GetLinkByAsset($request)
	{
		$request_info		= (Array) $request;
		$assetid			= array_get_index($request_info, 'AssetID', '');
		$other_assetid		= array_get_index($request_info, 'OtherAssetID', '');
		$link_type			= array_get_index($request_info, 'LinkType', '');
		$link_value			= array_get_index($request_info, 'LinkValue', '');
		$side_of_link		= array_get_index($request_info, 'SideOfLink', 'major');
		$dependant			= self::getNullIndex($request_info, 'IsDependant', NULL);
		$exclusive			= self::getNullIndex($request_info, 'IsExclusive', NULL);

		if (!empty($assetid)) {
			$links	= $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($assetid, $other_assetid, $link_type, $link_value, $side_of_link, TRUE, $dependant, $exclusive);
			if (!empty($links)) {
				$link_results	= Array();
				$i = 0;
				$other_side_of_link	= $side_of_link == 'major' ? 'minor' : 'major';
				foreach ($links as $link_info) {
					$link_results[$i][ucwords($other_side_of_link).'ID']		= $link_info[$other_side_of_link.'id'];
					$link_results[$i]['LinkID']			= $link_info['linkid'];
					$link_results[$i]['Value']			= $link_info['value'];
					$link_results[$i]['LinkType']		= $link_info['link_type'];
					$link_results[$i][ucwords($other_side_of_link).'TypeCode']	= $link_info[$other_side_of_link.'_type_code'];
					$link_results[$i]['SortOrder']		= $link_info['sort_order'];
					$link_results[$i]['IsDependant']	= $link_info['is_dependant'];
					$link_results[$i]['IsExclusive']	= $link_info['is_exclusive'];
					$i++;
				}//end foreach

				return Array (
						'GetLinkByAssetResult'	=> $link_results,
					   );
			} else {
				throw new SoapFault('Server', 'Unable To Get Link Between Assets');
			}//end else
		} else {
			throw new SoapFault('Server', 'Asset ID is not valid. Please provide a valid Asset ID');

		}//end else


	}//end GetLinkByAsset()


	/**
	* getComplexType
	*
	* @return array
	* @access public
	*/
	public static function getComplexElements($func_list=Array())
	{
		$obj_optional			= self::getArgType('AssetObject', 0, 1);
		$link_details			= self::getArgType('LinkDetails', 0, 'unbounded', TRUE);
		$link_info_summary		= self::getArgType('LinkInfoSummary', 0, 'unbounded', TRUE);
        $link_type				= self::getArgType('LinkTypeValue', 0, 1);
        $asset_types			= self::getArgType('AssetType', 0, 1);
		$link_type				= self::getArgType('LinkTypeValue', 0, 1);

		$complex_types	= Array (
							'CreateLink'				=> Array (
															'CreateLink'			=> Array (
																						'MajorID'		=> self::$string_non_optional,
														   								'MinorID'		=> self::$string_non_optional,
														   								'LinkType'		=> $link_type,
														   								'LinkValue'		=> self::$string_optional,
																						'SortOrder'		=> self::$string_optional,
														   								'IsDependant'	=> self::$string_optional,
														   								'IsExclusive'	=> self::$string_optional,
														   							   ),
														   	'CreateLinkResponse'	=> Array (
																						'CreateLinkResult'	=> self::$string_optional,
														   							   ),
														   ),
							'DeleteLink'				=> Array (
															'DeleteLink'				=> Array (
																								'LinkID'				=> self::$string_non_optional,
																							  ),
															'DeleteLinkResponse'		=> Array (
																								'DeleteLinkResult'	=> self::$boolean_optional,
																							   ),
														   ),
							'GetAllChildLinks'			=> Array (
															'GetAllChildLinks'				=> Array (
																								'AssetID'				=> self::$string_non_optional,
																   								'LinkType'				=> $link_type,
																							   ),
															'GetAllChildLinksResponse'		=> Array (
																								'GetAllChildLinksResult'	=> $link_details,
																							   ),
														   ),
							'GetChildren'				=> Array (
															'GetChildren'					=> Array (
																								'AssetID'			=> self::$string_non_optional,
																								'TypeCode'			=> self::$string_non_optional,
																								'StrictTypeCode'	=> self::$string_optional,
																								'Dependant'			=> self::$string_optional,
																   								'SortBy'			=> self::$string_optional,
																   								'PermissionLevel'	=> self::$string_optional,
																   								'EffectiveAccess'	=> self::$string_optional,
																   								'MinDepth'			=> self::$string_optional,
																   								'MaxDepth'			=> self::$string_optional,
																   								'DirectShadowsOnly'	=> self::$string_optional,
																								'LinkValueWanted'	=> self::$string_optional,
																							   ),
															'GetChildrenResponse'			=> Array (
																								'GetChildrenResult'	=> $link_info_summary,
																							   ),
														   ),
							'GetDependantChildren'		=> Array (
															'GetDependantChildren'			=> Array (
																								'AssetID'				=> self::$string_non_optional,
																								'TypeCode'				=> $asset_types,
																								'StrictTypeCode'		=> self::$boolean_optional,
																							   ),
															'GetDependantChildrenResponse'	=> Array (
																								'GetDependantChildrenResult'	=> $link_info_summary,

																							   ),
														   ),
							'GetDependantParents'		=> Array (
															'GetDependantParents'			=> Array (
																								'AssetID'				=> self::$string_non_optional,
														   										'TypeCode'				=> self::$string_optional,
														   										'StrictTypeCode'		=> self::$boolean_optional,
														   										'IncludeAllDependants'	=> self::$boolean_optional,
																							   ),
															'GetDependantParentsResponse'	=> Array (
																								'GetDependantParentsResult'	=> $link_info_summary,
																							   ),
														   ),
							'GetLinks'					=> Array (
															'GetLinks'						=> Array (
																								'AssetID'				=> self::$string_non_optional,
													   											'LinkType'				=> $link_type,
														   										'TypeCode'				=> $asset_types,
													   											'StrictTypeCode'		=> self::$boolean_optional,
													   											'SideOfLink'			=> self::$string_optional,
														   										'LinkValue'				=> self::$string_optional,
																								'Dependant'				=> self::$boolean_optional,
														   										'Exclusive'				=> self::$boolean_optional,
														   										'SortBy'					=> self::$string_optional,
														   										'PermissionLevel'		=> self::$string_optional,
														   										'Effective'				=> self::$boolean_optional,
																							   ),
															'GetLinksResponse'				=> Array (
																								'GetLinksResult'		=> $link_details,
																							   ),
														   ),
							'GetLinkByAsset'			=> Array (
															'GetLinkByAsset'				=> Array (
																								'AssetID'				=> self::$string_non_optional,
													   											'OtherAssetID'			=> self::$string_non_optional,
													   											'LinkType'				=> $link_type,
													   											'LinkValue'				=> self::$string_optional,
														   										'SideOfLink'			=> self::$string_optional,
														   										'IsDependant'			=> self::$boolean_optional,
														   										'IsExclusive'			=> self::$boolean_optional,
																							   ),
															'GetLinkByAssetResponse'		=> Array (
																								'GetLinkByAssetResult'	=> $link_details,
																							   ),
														   ),
							'MoveLink'					=> Array (
															'MoveLink'						=> Array (
																								'LinkID'				=> self::$string_non_optional,
													   											'LinkType'				=> $link_type,
													   											'ToParentID'			=> self::$string_optional,
													   											'ToParentPosition'		=> self::$string_optional,
																							   ),
															'MoveLinkResponse'				=> Array (
																								'MoveLinkResult'		=> self::$int_optional,
																							   ),
														   ),
							'UpdateLink'				=> Array (
															'UpdateLink'					=> Array (
																								'LinkID'				=> self::$string_non_optional,
													   											'LinkType'				=> $link_type,
													   											'LinkValue'				=> self::$string_optional,
													   											'SortOrder'				=> self::$string_optional,
																							   ),
															'UpdateLinkResponse'			=> Array (
																								'UpdateLinkResult'		=> self::$string_optional,
																							   ),
														   ),
							'GetParents'				=> Array (
															'GetParents'					=> Array (
																								'AssetID'				=> self::$string_non_optional,
																								'TypeCode'				=> $asset_types,
													   											'StrictTypeCode'		=> self::$string_optional,
													   											'SortBy'				=> self::$string_optional,
													 											'PermissionLevel'				=> self::$string_optional,
													   											'Effective'				=> self::$string_optional,
													   											'MinDepth'				=> self::$string_optional,
													   											'MaxDepth'				=> self::$string_optional,
													   											'DirectShadowOnly'		=> self::$string_optional,
													   											'LinkValueWanted'		=> self::$string_optional,

																							   ),
															'GetParentsResponse'				=> Array (
																								'GetParentsResult'		=> $link_info_summary,
																							   ),
														   ),
						  );

		$complex_types_available = parent::getComplexElements($complex_types);

		return $complex_types_available;


	}//end getComplexElements();


    /**
    * getComplexTypes: return all the complex types used by the API
    *
    * @return array
    * @access public
    */
    public static function getComplexTypes($func_list=Array())
    {
    	$link_details	= Array (
							'LinkDetails'	=> Array (
												'TreeID'			=> self::$string_optional,
										   	'LinkID'			=> self::$string_non_optional,
												'Level'				=> self::$string_optional,
												'MinorID'			=> self::$string_optional,
	        									'MajorID'			=> self::$string_optional,
												'MinorTypeCode'		=> self::$string_optional,
												'MajorTypeCode'		=> self::$string_optional,
												'AssetID'			=> self::$string_optional,
												'TypeCode'			=> self::$string_optional,
												'LinkType'			=> self::$string_optional,
												'SortOrder'			=> self::$string_optional,
												'Value'				=> self::$string_optional,
												'IsDependant'		=> self::$string_optional,
												'IsExclusive'		=> self::$string_optional,
												'DependantTreeID'	=> self::$string_optional,
											   ),
						  );

        $complex_types    = Array (
                            'GetAllChildLinks'		=> $link_details,
        					'GetLinks'				=> $link_details,
        					'GetLinkByAsset'		=> $link_details,
							'GetDependantParents'	=> Array (
                                           			     	'LinkInfoSummary'			=> Array (
																							'AssetID'			=> self::$string_non_optional,
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
        $asset_types_info           = $GLOBALS['SQ_SYSTEM']->am->getAssetTypes(TRUE, TRUE);
        $asset_types    = Array();
        foreach ($asset_types_info as $asset_type_info) {
            $asset_types[]  = $asset_type_info['type_code'];
        }//end foreach

        $common_types               = Array (
                                        'LinkTypeValue'  => Array (
                                                        'restriction_base'  => 'string',
                                                        'enumeration'       => Array (
                                                                                '1', '2', '4', '8'
                                                                               ),
                                                       ),
                                      );


		$simple_restricted_types	= Array (
										'CreateLink'				=> Array (
																	'LinkTypeValue'	=> $common_types['LinkTypeValue'],
																   ),
										'GetLinks'				=> Array (
																	'LinkTypeValue'	=> $common_types['LinkTypeValue'],
																	'AssetType'         => Array (
																							'restriction_base'  => 'string',
																							'enumeration'       => $asset_types,
																						   ),
																   ),
										'MoveLink'				=> Array (
																	'LinkTypeValue'	=> $common_types['LinkTypeValue'],
																   ),
										'UpdateLink'			=> Array (
																	'LinkTypeValue'	=> $common_types['LinkTypeValue'],
																   ),
										'GetLinkByAsset'		=> Array (
																	'LinkTypeValue'	=> $common_types['LinkTypeValue'],
																   ),
										'GetAllChildLinks'		=> Array (
																	'LinkTypeValue'	=> $common_types['LinkTypeValue'],
																   ),
										'GetChildren'			=> Array (
																	'AssetType'         => Array (
																							'restriction_base'  => 'string',
																							'enumeration'       => $asset_types,
																						   ),
																   ),
										'GetDependantChildren'	=> Array (
																	'AssetType'         => Array (
																							'restriction_base'  => 'string',
																							'enumeration'       => $asset_types,
																						   ),
																   ),
										'GetDependantParents'	=> Array (
																	'AssetType'         => Array (
																							'restriction_base'  => 'string',
																							'enumeration'       => $asset_types,
																						   ),
																   ),
										'GetParents'			=> Array (
																	'AssetType'         => Array (
																							'restriction_base'  => 'string',
																							'enumeration'       => $asset_types,
																						   ),
																   ),
						  );

		$simple_restricted_types_available = parent::processSimpleRestrictedTypes($simple_restricted_types);

		return $simple_restricted_types_available;

	}//end getSimpleRestrictedTypes()


	private static function getPermValue($permission)
	{
		$perm_level			= Array (
								'Read'		=> '1',
								'Write'		=> '2',
								'Admin'		=> '3',
							  );
		if (empty($permission) || !isset($perm_level[$permission])) return NULL;

		return $perm_level[$permission];

	}//end getPermValue()

}//end class
?>