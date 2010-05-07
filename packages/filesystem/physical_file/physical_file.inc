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
* $Id: physical_file.inc,v 1.1.1.1 2010/03/29 23:32:28 bpearson Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Physical File
*
* Purpose
*
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.1.1.1 $
* @package MySource_Matrix_Packages
* @subpackage file
*/
class Physical_File extends Asset
{

	/**
	* the asset ID of the image that the question belongs to
	* @var int
	*/
	var $_bridgeid = 0;

	/**
	* the shadow ID of the variety - unique for each parent asset
	* @var int
	*/
	var $_shadowid = 0;


	/**
	* Constructor
	*
	* @param string	$assetid    the asset id to be loaded
	* @param Array	$data		the shadow asset data that will construct this variety
	*
	*/
	function __construct($assetid=0, $data=Array())
	{
		parent::__construct();

		if (!$assetid) return;
		list($this->_bridgeid, $this->_shadowid) = explode(':', $assetid);

		$bridge = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_bridgeid);
		if (is_null($bridge)) return;

		// set the data paths
		$this->data_path        = $bridge->data_path.'/physical_file';
		$this->data_path_suffix = $bridge->data_path_suffix.'/physical_file';
		$this->data_path_public = $bridge->data_path_public.'/physical_file';

		// if there is no shadowid then we haven't been created yet, so don't do anymore
		if (!$this->_shadowid) return;

		$this->id = $assetid;

		foreach ($data as $attr_name => $attr_value) {
			if (!isset($this->vars[$attr_name])) continue;
			$this->vars[$attr_name]['value'] = ($this->vars[$attr_name]['type'] == 'serialise') ? unserialize($attr_value) : $attr_value;
		}

		$full_path = $this->getFilePath();
		$name = basename($full_path);

		// set general object properties, now we have the name of the thing
		$this->name = $name;
		$this->short_name = $name;
		$this->status = $bridge->status;
		$this->version = '0.1';

	}//end constructor


	/**
	 * Print frontend, this is used for binary files
	 *
	 * @return void
	 * @access public
	 */
	public function printFrontend()
	{
		require_once SQ_FUDGE_PATH.'/standards_lists/mime_types.inc';
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		// if we are in limbo, we want to show our editing interface and design
		// instead of just sending our file over
		// there is an exception to it: In Limbo, if we have ?a=<assetid> at the end of the URL
		// and <assetid> value is different from the current asset assetid we are sending our file over
		if (SQ_IN_LIMBO && !(isset($_REQUEST['a']) && $_REQUEST['a'] == $this->id)) {
			return parent::printFrontend();
		}

		if (!$this->readAccess()) {
			$GLOBALS['SQ_SYSTEM']->paintLogin(translate('login'), translate('cannot_access_asset', $this->name));
			return;
		}

		// we want to tell mysource::start() that it should not try to replace
		// keywords in this output, it could be a binary file after all
		$GLOBALS['SQ_SYSTEM']->setGlobalDefine('SQ_REPLACE_MYSOURCE_LEVEL_KEYWORDS', FALSE);

		$type = $this->getMimeType();

		// get protocol information
		$ssl_connection = FALSE;
		$url_info = parse_url(current_url());
		$protocol = (isset($url_info['scheme'])) ? $url_info['scheme'] : NULL;
		if (!is_null($protocol) && $protocol == 'https') {
			$ssl_connection = TRUE;
		}

		// send cacheable headers if file asset meets the credentials
		if (basename($_SERVER['PHP_SELF']) != SQ_CONF_NOCACHE_SUFFIX && ($GLOBALS['SQ_SYSTEM']->user instanceof Public_User)
			&& $this->effectiveUnrestricted() && empty($_POST) && SQ_CONF_SEND_CACHEABLE_HEADER && current_protocol() != 'https') {
			$cm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');

			$browser_cache_expiry = $cm->getBrowserCacheExpiry($this->type(), $this->id);
			if (empty($browser_cache_expiry)) {
				$browser_cache_expiry = $cm->getExpiry($this->type(), $this->id);
			}

			$last_updated = $this->getEffectiveLastUpdatedTime(Array());
			header('Expires: '.gmdate('D, d M Y H:i:s', time() + $browser_cache_expiry).' GMT');
			header('Cache-Control: max-age='.$browser_cache_expiry.', public');
			header('Pragma: cache');
			header('Last-Modified: '.gmdate('D, d M Y H:i:s',$QWERTY).' GMT');
		} else if (!$ssl_connection) {
			// Internet Explorer must forcibly reload some file types from the server for correct inline operation in popup windows. (ref. http://support.microsoft.com/default.aspx?scid=kb;EN-US;q297822)
			// If such files do not load as expected, "Send no-cache header option for HTTP requests" should be set to "No" on the System Configuration screen. The default operation is to send this header (i.e; set to "Yes").

			if (SQ_CONF_SEND_NO_CACHE_HEADER) {
				header('Cache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0');
			} else {
				header('Cache-Control: must-revalidate, post-check=0, pre-check=0');
			}

			header('Pragma: no-cache');
			header('Expires: '.gmdate('D, d M Y H:i:s', time()-3600).' GMT');
		} else {
			// internet explorer has a problem with SSL connection (https)
			// cant send no-cache header or we will get "cannot download file" error
			// http://support.microsoft.com/default.aspx?scid=kb;en-us;812935
			// http://support.microsoft.com/default.aspx?scid=kb;en-us;316431
			header('Cache-Control: private, max-age=0, must-revalidate');
			header('Pragma: private');
			header('Expires: '.gmdate('D, d M Y H:i:s', time()-3600).' GMT');
		}

		$file_path = $this->getFilePath();
		header('Content-Type: '.$type);
		header('Content-Disposition: inline; filename='.$this->name.';');
		header('Content-Length: '.filesize($file_path));

		readfile($file_path);
		@ob_flush();

	}//end printFrontend()


	/**
	 * Print body, this is used for text files when using a design
	 *
	 * @return void
	 * @access public
	 */
	public function printBody()
	{
		if (empty($this->bridgeid) && empty($this->_shadowid)) {
			list($this->_bridgeid, $this->_shadowid) = explode(':', $this->id);
		}//end if

		$bridge = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_bridgeid);
		if (is_null($bridge)) return '';

		$type = $this->getMimeType();

		if (strpos($type, 'text') === 0) {
			$location = $bridge->attr('path');
			$location = rtrim($location, '/');
			$full_path = $location.'/'.$this->_shadowid;

			$content = file_get_contents($full_path);
			$content = nl2br($content);
			echo $content;
		}//end if

	}//end printBody()


	/**
	 * Get the file path of the current asset 
	 *
	 * @return void
	 * @access public
	 */
	public function getFilePath()
	{
		if (empty($this->bridgeid) && empty($this->_shadowid)) {
			list($this->_bridgeid, $this->_shadowid) = explode(':', $this->id);
		}//end if

		$bridge = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_bridgeid);
		if (is_null($bridge)) return '';

		$location = $bridge->attr('path');
		$location = rtrim($location, '/');
		$full_path = $location.'/'.$this->_shadowid;

		return $full_path;

	}//end getFilePath()


	/**
	 * Get the mime type of the current asset 
	 *
	 * @return void
	 * @access public
	 */
	public function getMimeType()
	{
		require SQ_FUDGE_PATH.'/standards_lists/mime_types.inc';
		$ext = get_file_type($this->name);
		$type = array_get_index($standards_lists_mime_types, $ext, 'text/plain');

		return $type;

	}//end getMimeType()

}//end class
?>