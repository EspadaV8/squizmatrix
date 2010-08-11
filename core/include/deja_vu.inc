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
*$Id: deja_vu.inc,v 1.2 2010/07/14 00:27:42 ewang Exp $
*
*/

// We use a control file to enable/disable Deja Vu, since it must be flushed
// each time it is turned off. See scripts/dejavu_management.php
define('SQ_DEJA_VU_CONTROL', SQ_DATA_PATH.'/private/conf/.dejavu');
define('SQ_DEJA_VU_MEMCACHE_PREFIX', 'sq_deja_vu|');

// Storage types
define('SQ_DEJA_VU_ASSET', 'asset');
define('SQ_DEJA_VU_METADATA', 'metadata');
define('SQ_DEJA_VU_LOCALE', 'locale_manager');

/**
* Deja_Vu
*
* The blurry lines between short term and long term memory. Ever thought you'd seen that 
* black cat before?
*
* Deja_Vu provides a lazy memory store via memcached for asset objects and metadata values. 
* It is designed to move common and predictable data loading into a more persistent
* memory, purely to improve scalability.
*
* @author  James Hurst <jhurst@squiz.co.uk>
* @version $Revision: 1.2 $
* @package MySource_Matrix
*/
class Deja_Vu extends MySource_Object
{
	/**
	 * Memcache
	 *
	 * @var Object Memcache
	 */
	private $_memcache;
	
	
	/**
	 * Enabled flag.
	 *
	 * @var int
	 */
	private $_enabled;
	

	/**
	* Constructor
	*
	* Runs a bunch of assertions for the config, which will throw fatals. 
	* 
	* @return Deja_Vu
	* @access public
	*/
	function __construct()
	{
		$this->_enabled = 0;
		if (is_file(SQ_DEJA_VU_CONTROL)) {
			$this->_enabled = (int) file_get_contents(SQ_DEJA_VU_CONTROL);
			if ($this->_enabled !== 1) $this->_enabled = 0;
		}
		
		if ($this->enabled() === 1) {
			assert_true(extension_loaded('memcache'), 'Cannot use Deja Vu; it requires the memcache PECL extension installed within , which is not installed');
			assert_true(file_exists(SQ_DATA_PATH.'/private/conf/memcache.inc'), 'Cannot use Deja Vu; the Memcache configuration file is not set');

			$memcache_conf = require(SQ_DATA_PATH.'/private/conf/memcache.inc');
			$hosts =& $memcache_conf['hosts'];
			$services =& $memcache_conf['services'];

			assert_true(count($hosts) > 0, 'Cannot use Deja Vu; no hosts are defined in the Memcache configuration file');
			assert_true(array_key_exists('deja_vu', $services) === TRUE, 'Cannot use Deja Vu; no Memcache hosts are assigned');
			assert_true(count($services['cache_manager']) > 0, 'Cannot use Deja Vu; no Memcache hosts are assigned');

			// If has the memcache module installed, instantiate it and try to load some config.
			$this->_memcache = new Memcache;

			foreach ($services['deja_vu'] as $host_key => $weight) {
				assert_true(array_key_exists($host_key, $hosts) === TRUE, 'Cannot use Deja Vu; host key "'.$host_key.'" assigned for use for Deja Vu but not defined as a host');
				$host = $hosts[$host_key];
				$this->_memcache->addServer($host['host'], $host['port'], $host['persistent'], $weight, $host['timeout'], $host['retry_interval'], $host['status']);
			}

			$this->_memcache->setCompressThreshold($memcache_conf['compression_threshold'], $memcache_conf['compression_min_saving']);

			// Register for updates, so that we can keep our memories fresh.
			$em = $GLOBALS['SQ_SYSTEM']->getEventManager();
			$em->addEventListener($this, Array(	'AssetStatusUpdate',
												'assetDeleted',
												'AssetUpdate',
												'ContentsUpdated',
												'attributeChange',
												'AssetTypeUpdate',
												'MetadataUpdate',
												'MetadataDeleted',									
												)
											);
		}
		
	}//end constructor
	
	
	/**
	 * Flag to control enabled/disabled state.
	 *
	 * @return bool
	 * @access public
	 */
	public function enabled() 
	{
		return $this->_enabled;
		
	}//end enabled()
	
	
	/**
	* Fetch an item from memcached, or null if not found.
	*
	* @param String $type The type of data sort, asset or metadata currently.
	* @param String $assetid 
	*
	* @return mixed (Object or Array)
	* @access public
	*/
	public function recall($type, $assetid = NULL, $contextid = NULL)
	{
		//ignore shadow assets
		if(strpos($assetid, ':') !== FALSE) return NULL;
		$cache_key = $this->_getCacheKey($type, $assetid, $contextid);
		switch ($type) {
		case 'asset':
			$obj = $this->_memcache->get($cache_key);
			if (!$obj) {
			//	log_error("Cache miss for $cache_key");
				return NULL;
			}
			
			// If $obj is an undefined object, it's an asset stored without the class being included yet.
			// We must include the correct asset and try again.
			if ($obj instanceof __PHP_Incomplete_Class) {
				$obj_def = (array) $obj;
				// Determine the correct type code
				$type_code = strtolower($obj_def['__PHP_Incomplete_Class_Name']);
				
				$am	=& $GLOBALS['SQ_SYSTEM']->am;
				if ($am->installed($type_code)) {
					$am->includeAsset($type_code);
					
					// We need to re-get the object to re-unserialize.
					$obj = $this->_memcache->get($cache_key);
				} else {
					// Should never happen, but just in case..
					log_error('Deja Vu could not load asset type "'.$type_code.'" for #'.$assetid.'. Assuming the asset type has been uninstalled, so forgetting this asset.');
					$this->forget($assetid, $type);
					return NULL;
				}
			}//end if
			
			return $obj;
			break;
			
		default:
			$data = $this->_memcache->get($cache_key);

			if (!$data) {
				return NULL;
			} else {
				return $data;
			}
			break;
			
		}//end switch
		
	}//end recall
	

	/**
	* Store an item in memcached.
	*
	* @param String $type    The type of data sort, asset or metadata currently.
	* @param String $assetid
	* @param Mixed  $object  The object to store. Which might not be an Object.
	*
	* @return boolean
	* @access public
	*/
	public function remember($type, $assetid, $object, $contextid = NULL)
	{
		//ignore shadow assets
		if(strpos($assetid, ':') !== FALSE) return NULL;
		return $this->_memcache->set($this->_getCacheKey($type, $assetid, $contextid), $object);
	}//end remember()


	/**
	* Remove an item from memcached
	*
	* @param string $itemid	id of the item to remove
	*
	* @return boolean	false if the item is not in the cache or failure
	* @access public
	*/
	public function forget($type, $assetid = NULL)
	{	
		return $this->_memcache->delete($this->_getCacheKey($type, $assetid));
	}//end forget()
	
	
	/**
	 * Forget everything.
	 *
	 * @return bool Success|Failure
	 * @access public
	 */
	public function forgetAll()
	{
		return $this->_memcache->flush();
	
	}//end forgetAll()
	
	
	/**
	 * Generate the cache key for the given type / assetid.
	 *
	 * Assetid may be null since it's not always relevant.
	 *
	 * @param string $type
	 * @param string $assetid
	 * @return string
	 * @access private
	 */
	private function _getCacheKey($type, $assetid = NULL, $contextid = NULL)
	{
		switch ($type) {
			case SQ_DEJA_VU_LOCALE:
				return SQ_DEJA_VU_MEMCACHE_PREFIX.'|'.$type;
				break;
			default:
				$contextid = is_null($contextid) ? $GLOBALS['SQ_SYSTEM']->getContextId() : $contextid;
				return SQ_DEJA_VU_MEMCACHE_PREFIX.$contextid.'\\'.$assetid.'|'.$type;
				break;
		}
		
	}//end _getCacheKey()
	
	
	/******* EVENT HANDLERS *************************************************/
		
	
	/**
	 * Event handler for when asset status changes.
	 *
	 * @param Object broadcaster
	 * @param Array	 vars
	 * @return void
	 * @access public
	 */
	public function onAssetStatusUpdate(&$broadcaster, $vars)
	{
		$this->forget(SQ_DEJA_VU_ASSET, $broadcaster->id);
		
	}//end onAssetStatusUpdate
	
	/**
	 * Event handler for when asset is deleted.
	 *
	 * @param Object broadcaster
	 * @param Array	 vars
	 * @return void
	 * @access public
	 */
	public function onAssetDeleted(&$broadcaster, $vars)
	{
		$this->forget(SQ_DEJA_VU_ASSET, $broadcaster->id);
		
	}//end onAssetDeleted


	/**
	 * Event handler for when asset is updated.
	 *
	 * @param Object broadcaster
	 * @param Array	 vars
	 * @return void
	 * @access public
	 */
	public function onAssetUpdate(&$broadcaster, $vars = Array())
	{
		$this->forget(SQ_DEJA_VU_ASSET, $broadcaster->id);
		
	}//end onAssetUpdate


	/**
	 * Event handler for when asset's contents are updated.
	 *
	 * @param Object broadcaster
	 * @param Array	 vars
	 * @return void
	 * @access public
	 */
	public function onContentsUpdated(&$broadcaster, $vars)
	{
		$this->forget(SQ_DEJA_VU_ASSET, $broadcaster->id);
		
	}//end onContentsUpdated


	/**
	 * Event handler for when asset attributes change.
	 *
	 * @param Object broadcaster
	 * @param Array	 vars
	 * @return void
	 * @access public
	 */
	public function onAttributeChange(&$broadcaster, $vars)
	{
		$this->forget(SQ_DEJA_VU_ASSET, $broadcaster->id);
		
	}//end onAttributeChange
	
	
	/**
	 * Event handler for when asset type changes (morphing).
	 *
	 * @param Object broadcaster
	 * @param Array	 vars
	 * @return void
	 * @access public
	 */
	public function onAssetTypeUpdate(&$broadcaster, $vars = Array())
	{
		$this->forget(SQ_DEJA_VU_ASSET, $broadcaster->id);
		
	}//end onAttributeChange
	
	
	/**
	 * Event handler for when metadata is updated.
	 *
	 * @param Object broadcaster
	 * @param Array	 vars
	 * @return void
	 * @access public
	 */
	public function onMetadataUpdate(&$broadcaster, $vars)
	{
		$this->forget(SQ_DEJA_VU_METADATA, $broadcaster->id);
		
	}//end onMetadataUpdate
	
	
	/**
	 * Event handler for when metadata is deleted.
	 *
	 * @param Object broadcaster
	 * @param Array	 vars
	 * @return void
	 * @access public
	 */
	public function onMetadataDeleted(&$broadcaster, $vars)
	{
		$this->forget(SQ_DEJA_VU_METADATA, $broadcaster->id);
		
	}//end onMetadataDeleted
	
	
	/******* UTILITIES *************************************************/
	
	
	/**
	 * Enable DeJa Vu
	 *
	 * @return bool Success|Failure
	 * @access public
	 */
	public function enable()
	{
		if (!$this->enabled()) {
			$rv = (bool) file_put_contents(SQ_DEJA_VU_CONTROL, '1');
			if ($rv) {
				$this->_enabled = 1;
			}
		}
		return $rv;
		
	}//end enable()
	
	
	/**
	 * Disable DeJa Vu
	 *
	 * @return bool Success|Failure
	 * @access public
	 */
	public function disable()
	{
		$rv = FALSE;
		if ($this->enabled()) {
			$rv = (bool) file_put_contents(SQ_DEJA_VU_CONTROL, '0');
			if ($rv) {
				$this->_enabled = 0;
			}
		}
		return $rv;
		
	}//end disable()


}//end class

?>