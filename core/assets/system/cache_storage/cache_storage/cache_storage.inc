<?php
/**
 * +--------------------------------------------------------------------+
 * | Squiz.net Open Source Licence                                      |
 * +--------------------------------------------------------------------+
 * | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
 * +--------------------------------------------------------------------+
 * | This source file may be used subject to, and only in accordance    |
 * | with, the Squiz Open Source Licence Agreement found at             |
 * | http://www.squiz.net/licence.                                      |
 * | Make sure you have read and accept the terms of that licence,      |
 * | including its limitations of liability and disclaimers, before     |
 * | using this software in any way. Your use of this software is       |
 * | deemed to constitute agreement to be bound by that licence. If you |
 * | modify, adapt or enhance this software, you agree to assign your   |
 * | intellectual property rights in the modification, adaptation and   |
 * | enhancement to Squiz Pty Ltd for use and distribution under that   |
 * | licence.                                                           |
 * +--------------------------------------------------------------------+
 *
 * $Id: cache_storage.inc,v 1.1 2009/09/11 03:24:24 lwright Exp $
 *
 */

require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
 * Cache_Storage
 *
 * Abstract cache storage class.
 *
 * @author  James Hurst <james@riverratrecords.com>
 * @author  Luke Wright <lwright@squiz.net>
 * @version $Revision: 1.1 $
 * @package MySource_Matrix_Packages
 * @subpackage __core__
 */
class Cache_Storage extends Asset
{


	/**
	 * Constructor
	 *
	 * @param string	$assetid	the asset id to be loaded
	 *
	 */
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	 * Is this cache storage mechanism available?
	 *
	 * This is written as a static function so that it can be called by things
	 * like the Cache Manager's edit interface without having to load it up.
	 *
	 * By default, cache storage is always available. You may wish to override
	 * by letting it depend on the presence of some other feature.
	 *
	 * @return boolean
	 */
	public static function isAvailable()
	{
		return TRUE;

	}//end constructor


	/**
	 * Saves the specified contents to the cache, which will be identified
	 * by the specfied asset and key/url
	 *
	 * If cache entry already exists, it will be overwritten with the new contents
	 * and expiry time
	 *
	 * @param string     $cache_key  the cache key for this entry
	 * @param string     $perm_key   the permissions key for this entry
	 * @param string    	$url		the url for this entry
	 * @param integer    $assetid    the asset id for this entry
	 * @param string		$data		the cache contents
	 * @param integer	$expiry		the expiry time in seconds
	 *
	 * @return boolean
	 * @access public
	 */
	function store($cache_key, $perm_key, $url, $assetid, $data, $expiry)
	{
		return FALSE;

	}//end store()


	/**
	 * Loads the contents from a cache file for the specified asset
	 * and unique key
	 *
	 * returns FALSE if no cache version exist
	 *
	 * @param string     $cache_key  the cache key for this entry
	 * @param string     $perm_key   the permissions key for this entry
	 * @param string    	$url		the url for this entry
	 * @param integer    $assetid    the asset id for this entry
	 *
	 * @return string
	 * @access public
	 */
	function read($cache_key, $perm_key, $url, $assetid)
	{
		return FALSE;

	}//end read()


	/**
	 * Removes all cache
	 *
	 * @return void
	 * @access public
	 */
	function clearAll()
	{
		return;

	}//end clearAll()


	/**
	 * Returns true if this storage method can clear cache for specific assetids.
	 *
	 * @return	bool
	 * @access	public
	 */
	function canClearByAssetid()
	{
		return FALSE;

	}//end canClearByAssetid()


	/**
	 * Removes all cache entries related to this assetid
	 *
	 * @param array  $assetids   id of the the asset that owns the cache file
	 *
	 * @return void
	 * @access public
	 */
	function clear($assetids)
	{
		return;

	}//end clear()


	/**
	 * Returns an array of file paths for clear cache HIPO job
	 *
	 * @param array  $assetids   id of the the asset that owns the cache file
	 *
	 * @return array
	 * @access public
	 */
	function getFilePaths($assetids)
	{
		return Array();

	}//end getFilePaths()


	/**
	 * Returns an array of all file paths for clear cache HIPO job
	 *
	 * @param string $option the option of the result
	 *
	 * @return array
	 * @access public
	 */
	function getAllFilePaths($option='')
	{
		return Array();

	}//end getAllFilePaths()


}//end class

?>