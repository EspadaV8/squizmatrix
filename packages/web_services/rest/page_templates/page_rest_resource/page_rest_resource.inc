<?php 
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: page_rest_resource.inc,v 1.13 2013/10/04 00:41:11 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';


/**
* Page_REST_Resource
*
* Example:
*
* @author  James Hurst <jhurst@squiz.co.uk>
* @version $ $
* @package Squiz_UK_Clients
* @subpackage rest
*/
class Page_REST_Resource extends Page
{
	protected $_res = Array();

	// These are the headers to be sent if send_headers is true. 
	// Only those with values found in the first response will be forwarded.
	private $_response_headers = Array(
		'Content-Type',
		'Content-Length',
		'Last-Modified',
		'Cache-Control',
		'Expires',
	);

	// For storing extra param replacements for use in the HTTP request attribute.
	protected $_extra_replacements = Array();


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
	 * Print this asset.
	 *
	 * $force_process defaults to TRUE, ensuring the code to run the requests always happens.
	 * However, the %asset_contents% keyword overrides this to allow _process to use an existing result,
	 * in the event of being viewed in a paint layout.
	 *
	 * @return void
	 * @access public
	 */
	public function printBody()
	{
		// Run the request(s).
		$this->_process();

		$binary_data = FALSE;
		if (!empty($this->_res['responses'])) {
			if ($this->attr('send_headers')) {
				foreach ($this->_response_headers as $h) {
					if (isset($this->_res['response']['headers'][$h])) {
						header("$h: " . $this->_res['response']['headers'][$h]);
						if (!$binary_data && strtolower($h) == 'content-type' && strpos($this->_res['response']['headers'][$h], 'text/') !== 0) {
							$binary_data = TRUE;
						}
					}
				}
			}

			// Send the response. If there are multiple responses, send them all. This might
			// seem odd, but occasionally you might be building a document from multiple fragments.
			$error_response = $this->attr('error_response');
			foreach ($this->_res['responses'] as &$resp) {
				if ($resp['info']['http_code'] >= 400 && !empty($error_response)) {
					echo $error_response;
				} else {
					$content = $resp['body'];
					// If the keywords in rest content are not supposed to be replaced, escape the keywords.
					// However if the content is binary, leave the content as it as
					if (!$this->attr('allow_global_replace') && !$binary_data) {
						require_once SQ_FUDGE_PATH.'/general/text.inc';
						escape_matrix_keywords($content);
					}
					echo $content;
				}
			}
		}

	}//end printBody()


	/**
	 * Process the requests.
	 *
	 * @return void
	 * @access protected
	 */
	protected function _process()
	{
		$GLOBALS['SQ_SYSTEM']->pm->startTimer($this,'HTTPRequest');
		// Only run the requests if we are being forced to, or if they have not yet been run.
		if (($this->attr('allow_multiple_runs')) || (empty($this->_res['responses']))) {
			// Run the request(s).
			$http_attr = $this->getAttribute('http_request');
			$http_attr->setKeywordReplacements($this->_extra_replacements);

			// all the available keywords need to be passed to the attribute to have keyword set in the attribute to be replaced
			$available_keywords = parent::getAvailableKeywords();
			foreach ($available_keywords as $keyword => $val) {
				$available_keywords[$keyword] = $this->getKeywordReplacement($keyword);
			}

			$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
			$metadata_fields = $mm->getMetadata($this->id);
			foreach($metadata_fields as $schema_fieldid => $field_data) {
				foreach ($field_data as $item) {
					$available_keywords['asset_metadata_'.$item['name']] = $item['value'];
				}
			}

			$http_attr->setKeywordReplacements($available_keywords);

			$this->_res =& $http_attr->run();

			// trigger the event with this REST asset as a triggering asset and a response received
			$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_rest_response_received', $this, $this->_res);
		}
		$GLOBALS['SQ_SYSTEM']->pm->stopTimer($this,'HTTPRequest');
	}//end _process()
	

	/**
	 * Get keyword replacement for %asset_rest_contents%
	 *
	 * @param string $keyword    Keyword to find a replacement for
	 *
	 * @return string
	 * @access public
	 */
	public function getAssetRestContentsKeywordReplacement()
	{
		ob_start();
		$this->printBody();

		return ob_get_clean();

	}//end getAssetContentsKeywordReplacement()


	/**
	 * Get bodycopy keyword replacement
	 *
	 * @param string $keyword    Keyword to find a replacement for
	 *
	 * @return string
	 * @access public
	 */
	public function getKeywordReplacement($keyword)
	{
		if (substr($keyword, 0, strlen('asset_rest_contents'))  == 'asset_rest_contents') {

			$params = substr($keyword, strlen('asset_rest_contents') + 1);
			$params = explode('_', $params);

			$i = 1;
			foreach ($params as $p) {
				$this->_extra_replacements["rest_param_$i"] = $p;
			}

			return $this->getAssetRestContentsKeywordReplacement();

		} else if (substr($keyword, 0, strlen('rest')) == 'rest') {
			// If the keyword starts with REST, dig in the data for a replacement.
			return $this->getKeywordReplacementFromArray($this->_res, substr($keyword, strlen('rest_')));

		}

		return parent::getKeywordReplacement($keyword);

	}//end getKewordReplacement()


	/**
	 * Looks inside an array/hash for values indexed by a keyword string.
	 *
	 * Example: 
	 *
	 *  $array['resources'][1]['headers']['Content-Type']
	 *
	 *  ..can be accessed with:
	 *
	 *  rest_resource_1_headers_Content-Type
	 *
	 * @return string
	 * @access public
	 */
	public function getKeywordReplacementFromArray($array, $keyword)
	{
		// If $keyword is a 1st level index of this array, then great..
		if (isset($array[$keyword])) return $array[$keyword];

		// Otherwise, split the keyword into parts, and try the first part, recursing if there's a match.
		$parts = explode('_', $keyword);
		$part = array_shift($parts);
	
		if (isset($array[$part])) {
			return $this->getKeywordReplacementFromArray($array[$part], join($parts, '_'));
		}

	}//end getKeywordReplacementFromArray()

}
?>
