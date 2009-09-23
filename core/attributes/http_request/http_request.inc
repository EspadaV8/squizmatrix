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
* $Id: http_request.inc,v 1.1 2009/09/23 06:57:48 akarelia Exp $
*
*/

require_once SQ_ATTRIBUTES_PATH.'/serialise/serialise.inc';
require_once SQ_ATTRIBUTES_PATH.'/option_list/option_list.inc';

/**
* HTTP Request Asset Attribute
*
* <!-- Sample Edit Interface XML Node -->
* 	<http_request />
*
* @author  James Hurst <jhurst@squiz.co.uk>
* @version $Revision: 1.1 $
* @package MySource_Matrix
* @subpackage attributes
*/
class Asset_Attribute_HTTP_Request extends Asset_Attribute_Serialise
{
	// Options.
	private $_methods = Array(
		'GET'    => 'GET', 
		'POST'   => 'POST', 
		'HEAD'   => 'HEAD', 
		'PUT'    => 'PUT', 
		'DELETE' => 'DELETE'
	);

	private $_authentication_types = Array(
		'NONE' => 'None', 
		'HTTP_Basic' => 'HTTP Basic',
		'Matrix' => 'Matrix Cookie Passthrough',
		'OAuth' => 'OAuth',
	);

	private $_cache_options = Array(
		'NEVER'		=> 'Never Cache',
		'DEFAULT'	=> 'Use Default Expiry',
		'HTTP'		=> 'Use HTTP Expiry Headers',
	);

	private $_document_encoding = Array(
		'Auto' => 'Auto Detect',
		'ISO-8859-1' => 'ISO-8859-1',
		'ISO-8859-15' => 'ISO-8859-15',
		'CP866' => 'CP866',
		'CP1251' => 'CP1251',
		'CP1252' => 'CP1252',
		'KOI8-R' => 'KOI8-R',
		'BIG5' => 'BIG5',
		'GB2312' => 'GB2312',
		'BIG5-HKSCS' => 'BIG5-HKSCS',
		'Shift_JIS' => 'Shift_JIS',
		'EUC-JP' => 'EUC-JP',
		'UTF-8' => 'UTF-8',
	);
	

	// Data.
	protected $_res = Array(); // This is where everything lives.
	protected $_config = Array(); // The initial config.
	private $_keyword_replacements = Array(); // For replacements set from outside, to be replaced into URLs, headers, post body etc.
	private $_urls_to_request = Array(); // The URLs that need fetching. Some indexes may be blank.
	private $_headers = Array(); // The latest reponse headers (cURL callback).
	private $_last_header = FALSE; // Flag to deal with separating sets of headers when following redirects.


	/**
	 * Constructor
	 *
	 * @param string $assetid    the asset id to be loaded
	 *
	 */
	function __construct($attribute = 0, $value = NULL)
	{
		parent::__construct($attribute, $value);

	}//end constructor


	/**
	* Print the interface for filling in a value
	*
	* @param string		$prefix		prefix for the form elements
	* @param boolean	$read_only	are we just printing the value
	*
	* @return void
	* @access public
	*/
	public function paint($prefix, $read_only = FALSE)
	{
		$prefix = str_replace(':', '_', $prefix);
		$current_value = @unserialize($this->value);

		// Set default values.
		if (!is_array($current_value)) $current_value = Array();
		if (!isset($current_value['urls'])) $current_value['urls'] = Array();
		if (!isset($current_value['method'])) $current_value['method'] = 'GET';
		if (!isset($current_value['timeout'])) $current_value['timeout'] = 10;
		if (!isset($current_value['follow_redirect'])) $current_value['follow_redirect'] = TRUE;
		if (!isset($current_value['cache_options'])) $current_value['cache_options'] = 'HTTP';
		if (!isset($current_value['cache_post_requests'])) $current_value['cache_post_requests'] = FALSE;
		if (!isset($current_value['default_cache_expiry'])) $current_value['default_cache_expiry'] = 60;
		if (!isset($current_value['authentication_type'])) $current_value['authentication_type'] = 'NONE';
		if (!isset($current_value['http_auth_options'])) $current_value['http_auth_options'] = Array('username' => '', 'password' => '');
		if (!isset($current_value['request_headers'])) $current_value['request_headers'] = Array();
		if (!isset($current_value['request_body'])) $current_value['request_body'] = '';
		if (!isset($current_value['convert_to_utf8'])) $current_value['convert_to_utf8'] = FALSE;
		if (!isset($current_value['document_encoding'])) $current_value['document_encoding'] = 'Auto';
		if (!isset($current_value['run_test'])) $current_value['run_test'] = FALSE;
		if (!isset($current_value['oauth_asset']['assetid'])) $current_value['oauth_asset'] = Array('assetid' => '0', 'url' => '', 'linkid' => '');
		?>

		<table border="0" class="sq-backend-table">
			<tr>
				<th width="20%"><?php echo translate('http_request_method'); ?></th>
				<td>
					<?php
						if ($read_only) {
							echo htmlspecialchars($current_value['method']);
						} else {
							combo_box($prefix.'_method', $this->_methods, FALSE, $current_value['method']);
						}
					?>
				</td>
			</tr>
			<?php 
				if ($current_value['method'] == 'POST') {
					?>
					<tr>
						<th><?php echo translate('http_request_cache_post_requests'); ?></th>
						<td>
							<?php
								if ($read_only) {
									echo htmlspecialchars($current_value['cache_post_requests'] ? 'Yes' : 'No');
								} else {
									check_box($prefix.'_cache_post_requests', 1, $current_value['cache_post_requests']);
								}
							?>
						</td>
					</tr>
					<?php
				}
			?>
			<tr>
				<th><?php echo translate('http_request_urls'); ?></th>
				<td>
					<?php
					$ol = new Asset_Attribute_Option_List();
					$ol->value = implode($ol->delimiter, $current_value['urls']);
					$ol->setEditParam('width', '70');
					$ol->paint($prefix.'_urls', $read_only);
					?>
				</td>
			</tr>
			<tr>
				<th><?php echo translate('http_request_timeout'); ?></th>
				<td>
					<?php
						if ($read_only) {
							echo htmlspecialchars($current_value['timeout']);
						} else {
							text_box($prefix.'_timeout', $current_value['timeout'], 4);
						}
					?>
				</td>
			</tr>
			<tr>
				<th><?php echo translate('http_request_follow_redirect'); ?></th>
				<td>
					<?php
						if ($read_only) {
							echo htmlspecialchars($current_value['follow_redirect'] ? 'Yes' : 'No');
						} else {
							check_box($prefix.'_follow_redirect', 1, $current_value['follow_redirect']);
						}
					?>
				</td>
			</tr>
			<tr>
				<th><?php echo translate('http_request_cache_options'); ?></th>
				<td>
					<?php
						if ($read_only) {
							echo htmlspecialchars($this->_cache_options[$current_value['cache_options']]);
						} else {
							combo_box($prefix.'_cache_options', $this->_cache_options, FALSE, $current_value['cache_options']);
						}
					?>
				</td>
			</tr>
			<tr>
				<th><?php echo translate('http_request_default_cache_expiry'); ?></th>
				<td>
					<?php
						if ($read_only) {
							echo htmlspecialchars($current_value['default_cache_expiry']);
						} else {
							text_box($prefix.'_default_cache_expiry', $current_value['default_cache_expiry'], 7);
						}
					?>
				</td>
			</tr>
			<tr>
				<th><?php echo translate('http_request_authentication_type'); ?></th>
				<td>
					<?php
						if ($read_only) {
							echo htmlspecialchars($this->_authentication_types[$current_value['authentication_type']]);
						} else {
							combo_box($prefix.'_authentication_type', $this->_authentication_types, FALSE, $current_value['authentication_type']);

							switch ($current_value['authentication_type']) {
							case 'HTTP_Basic':
								?>
								<table>
									<tr>
										<th><?php echo translate('username'); ?></th>
										<td>
											<?php
												text_box($prefix.'_http_auth_user', $current_value['http_auth_options']['username'], '', '', 'autocomplete="off"');
											?>
										</td>
									</tr>
									<tr>
										<th><?php echo translate('password'); ?></th>
										<td>
											<?php
												password_box($prefix.'_http_auth_pass', $current_value['http_auth_options']['password'], '', '', 'autocomplete="off"');
											?>
										</td>
									</tr>
								</table>
							<?php
								break;
							case 'Matrix':
							?>
								<span>Note: This requires the global setting <strong>Allow IP Change</strong></span>
							<?php
								break;
							case 'OAuth':
								$type_codes = Array(
									'page_rest_resource_oauth_session' => 'D',
									);
								?>
								<table>
									<tr>
										<th><?php translate('oauth_asset'); ?></th>
										<td>
										<?php
											asset_finder($prefix.'_oauth_asset', $current_value['oauth_asset']['assetid'], $type_codes);
										?>
										</td>
									</tr>
								</table>
								<?php
								break;
							}
						}
					?>
				</td>
			</tr>
			<tr>
				<th><?php echo translate('http_request_request_headers'); ?></th>
				<td>
					<?php
					$hl = new Asset_Attribute_Option_List();
					$hl->value = implode($hl->delimiter, $current_value['request_headers']);
					$hl->setEditParam('width', '60');
					$hl->paint($prefix.'_request_headers', $read_only);
					?>
				</td>
			</tr>
			<?php 
			if (($current_value['method'] == 'POST') || ($current_value['method'] == 'PUT')) {
			?>
			<tr>
				<th><?php echo translate('http_request_request_body'); ?></th>
				<td>
					<?php
						if ($read_only) {
							echo htmlspecialchars($current_value['request_body']);
						} else {
							text_area($prefix.'_request_body', $current_value['request_body'], 70, 20, 0, 'style="font-family: monospace;"');
						}
					?>
				</td>
			</tr>
			<?php 
			} else {
				hidden_field($prefix.'_request_body', $current_value['request_body']);
			}	
			?>
			<tr>
				<th><?php echo translate('http_request_convert_to_utf8'); ?></th>
				<td>
					<?php
						if ($read_only) {
							echo $current_value['convert_to_utf8'] ? 'Yes' : 'No';
						} else {
							check_box($prefix.'_convert_to_utf8', 1, $current_value['convert_to_utf8']);
						}
					?>
				</td>
			</tr>
			<?php 
				if ($current_value['convert_to_utf8'] == TRUE) {
					?>
					<tr>
						<th><?php echo translate('http_request_document_encoding'); ?></th>
						<td>
							<?php
								if ($read_only) {
									echo $this->_document_encoding[$current_value['document_encoding']];
								} else {
									echo combo_box($prefix.'_document_encoding', $this->_document_encoding, FALSE, $current_value['document_encoding']);
								}
							?>
						</td>
					</tr>
					<?php
				}
			?>
			<tr>
				<th><?php echo translate('http_request_run_test'); ?></th>
				<td>
					<?php
						if ($read_only) {
							echo $current_value['run_test'] ? 'Yes' : 'No';
						} else {
							check_box($prefix.'_run_test', 1, 0);

							// Run the test?
							if ($current_value['run_test'] == 1) {
								$res = $this->run();

								if (!empty($res) || $res['responses'] != '' ) {
									// Truncate the bodies to be nice..
								 	foreach ($res['responses'] as &$response) {
										if (strlen($response['body']) > 50) {
											$response['body'] = substr($response['body'], 0, 50) . '...';
										}
									}
								}

								// This is not a debug statement.
								pre_echo($res);
							}
							
						}
					?>
				</td>
			</tr>
		</table>
		<?php

	}//end paint()


	/**
	* Process the interface for filling in a value
	*
	* @param string	$prefix	prefix for the form elements
	*
	* @return void
	* @access public
	*/
	public function process($prefix)
	{
		$prefix = str_replace(':', '_', $prefix);

		$value = Array();
		
		// Assign values
		$value['method'] = (isset($_REQUEST[$prefix.'_method'])) ? $_REQUEST[$prefix.'_method'] : '';

		$ol = new Asset_Attribute_Option_List();
		$ol->process($prefix.'_urls');
		$value['urls'] = trim($ol->value) ? explode($ol->delimiter, $ol->value) : NULL;
		
		$value['timeout'] 				= (isset($_REQUEST[$prefix.'_timeout'])) ? $_REQUEST[$prefix.'_timeout'] : 0;
		$value['follow_redirect'] 		= (isset($_REQUEST[$prefix.'_follow_redirect'])) ? $_REQUEST[$prefix.'_follow_redirect'] : FALSE;
		$value['cache_options'] 		= (isset($_REQUEST[$prefix.'_cache_options'])) ? $_REQUEST[$prefix.'_cache_options'] : 'HTTP';
		$value['cache_post_requests'] 	= (isset($_REQUEST[$prefix.'_cache_post_requests'])) ? $_REQUEST[$prefix.'_cache_post_requests'] : FALSE;
		$value['default_cache_expiry']	= (isset($_REQUEST[$prefix.'_default_cache_expiry'])) ? $_REQUEST[$prefix.'_default_cache_expiry'] : 60;
		$value['authentication_type'] 	= (isset($_REQUEST[$prefix.'_authentication_type'])) ? $_REQUEST[$prefix.'_authentication_type'] : '';
		$value['request_body'] 			= (isset($_REQUEST[$prefix.'_request_body'])) ? trim($_REQUEST[$prefix.'_request_body']) : '';
		
		$hl = new Asset_Attribute_Option_List();
		$hl->process($prefix.'_request_headers');
		$value['request_headers'] = trim($hl->value) ? explode($hl->delimiter, $hl->value) : NULL;

		$value['convert_to_utf8'] = (isset($_REQUEST[$prefix.'_convert_to_utf8'])) ? $_REQUEST[$prefix.'_convert_to_utf8'] : FALSE;
		$value['document_encoding'] = (isset($_REQUEST[$prefix.'_document_encoding'])) ? $_REQUEST[$prefix.'_document_encoding'] : 'Auto';
		$value['run_test'] = (isset($_REQUEST[$prefix.'_run_test'])) ? trim($_REQUEST[$prefix.'_run_test']) : FALSE;
		$value['http_auth_options']['username'] = (isset($_REQUEST[$prefix.'_http_auth_user'])) ? $_REQUEST[$prefix.'_http_auth_user'] : '';
		$value['http_auth_options']['password'] = (isset($_REQUEST[$prefix.'_http_auth_pass'])) ? $_REQUEST[$prefix.'_http_auth_pass'] : '';

		$value['oauth_asset'] = (isset($_REQUEST[$prefix.'_oauth_asset'])) ? $_REQUEST[$prefix.'_oauth_asset'] : '';

		$this->processed = $this->setValue($value);

	}//end process()


	/**
	* Sets keyword replacements to be replaced into URLs, headers, post bodies, auth etc.
	*
	* @param Array	$replacements	Keyword indexed array of replacements, e.g. 'my_key_word' => 'my value'
	*
	* @return void
	* @access public
	*/
	public function setKeywordReplacements($replacements)
	{
		$this->_keyword_replacements = $replacements;

	}//end setKeywordReplacements()


	/**
	 * Runs the remote requests.
	 *
	 * @access public
	 * @return void
	 */
	public function run()
	{
		$this->_config = @unserialize($this->value);
		if (empty($this->_config)) return;
		if ($this->_config['authentication_type'] == 'OAuth' && $this->_config['oauth_asset']['assetid'] == 0) return;

		// Build the result data structure. This will be returned eventually, and is also used to form the cache keys.
		$this->_res = Array(
			'request' => Array(
				'method' => $this->_config['method'],
				'headers' => count($this->_config['request_headers']) ? $this->_config['request_headers'] : Array(),
				'body' => $this->_config['request_body'],
				'urls' => count($this->_config['urls']) ? $this->_config['urls'] : Array(),
				'auth' => Array(
					'type' => $this->_config['authentication_type'],
				),
			),
			'responses' => Array(),
			'response'  => Array(),
		);

		// Do nothing if there are no URLs.
		if (count($this->_res['request']['urls']) < 1) return $this->_res;

		// Replace keywords in each URL.
		foreach ($this->_res['request']['urls'] as &$url) {
			$this->_replaceKeywords($url);
		}
		
		// Replace keywords in each Header.
		for ($i = 0; $i < count($this->_res['request']['headers']); $i++) {
			$this->_replaceKeywords($this->_res['request']['headers'][$i]);

			// Discard badly formatted headers.
			if (preg_match('/(.+):(.+)/', $this->_res['request']['headers'][$i]) == 0) {
				unset($this->_res['request']['headers'][$i]); 
			}
		}

		// Replace keywords in the body.
		$this->_replaceKeywords($this->_res['request']['body']);

		// Auth options.
		switch ($this->_res['request']['auth']['type']) {
			case 'HTTP_Basic':
				$this->_res['request']['auth']['http_auth_options'] = $this->_config['http_auth_options'];
				$this->_replaceKeywords($this->_res['request']['auth']['http_auth_options']['username']);
				$this->_replaceKeywords($this->_res['request']['auth']['http_auth_options']['password']);
				break;

				case 'Matrix':
					array_push($this->_res['request']['headers'], 'Set-Cookie: SQ_SYSTEM_SESSION=' . $_COOKIE['SQ_SYSTEM_SESSION']);
					break;

				case 'OAuth':
					// at this point, the oauth asset needs to have access token available...otherwise no data will be received back...
					$oauth_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->_config['oauth_asset']['assetid'], '', TRUE);
					if (!is_null($oauth_asset)) {
						$this->_res['request']['oauth_headers'] = $oauth_asset->getUserDataRequestAuthHeaders($this->_res['request']['urls']);
					}

				// Other auth types here.
				}

		// Go!
		$this->_performRequests();

		// encode responses if necessary
		if (isset($this->_config['convert_to_ut8']) && ($this->_config['convert_to_utf8'])) {
			$doc_encoding = $this->_config['document_encoding'];

			// UTF-8 is not supported yet. this option is going to be used when Entity conversion is supported.
			// as a temporary solution, do nothing if UTF-8 is selected
			if ($doc_encoding != 'UTF-8') {
				foreach ($this->_res['responses'] as $index => $response) {

					// if the document encoding option is Auto, try to find the encoding from the responses
					if ($doc_encoding == 'Auto') {
						$doc_encoding = $this->_findDocumentEncoding($response);
					}

					// if the document encoding is supported and it is not UTF-8, convert the encoding
					// ToDo: when http entity conversion is supported, UTF-8 will also need to be converted
					if (strlen($doc_encoding) > 0 && array_get_index($this->_document_encoding, $doc_encoding) && $doc_encoding != 'UTF-8') {
						$this->_res['responses'][$index]['body'] = iconv($doc_encoding, 'UTF-8', $response['body']);
					}
				}
			}
		}
		
		// Set url to the first in urls
		$this->_res['request']['url'] = &$this->_res['request']['urls'][0];

		// Set the first in responses to response
		$this->_res['response'] =& $this->_res['responses'][0];

		return $this->_res;

	}//end run()


	/**
	 * Returns the document encoding by detecting from content in the response.
	 *
	 * @param   array   a response with headers & body
	 * @access  protected
	 * @return  string
	 */
	protected function _findDocumentEncoding($response)
	{
		$charset = '';

		// try finding the document encoding from the header first
		$content_type_info = explode(';', $response['info']['content_type']);
		if (isset($content_type_info[0])) {
			$content_type = $content_type_info[0];

			// look for encoding only when the content type is text/html
			if (strcasecmp($content_type, 'text/html') == 0) {
				// if the encoding is set in the header, great! let's use it!
				if (isset($content_type_info[1]) && substr(trim($content_type_info[1]), 0, strlen('charset='))  == 'charset=') {
					$charset = substr(trim($content_type_info[1]), strlen('charset='));
				} else {
					// if the encoding is not in the header, need to look into the body to see if there is any encoding set...
					preg_match_all("|<meta http-equiv=[^>]+content=[\"\']text/html;[\s]?charset=[^>]+/>|U", $response['body'], $matches);
					if (isset($matches[0][0])) {
						$charset = preg_replace("/.*charset=/i", "", $matches[0][0]);
						$charset = trim(str_replace(Array("/", ">", "\""), "", $charset));
					}
				}
			}
		}
		return strtoupper($charset);

	}//end _findDocumentEncoding()


	/**
	 * Replaces global and custom keywords in $text.
	 *
	 * @param string &$text
	 * @access protected
	 * @return void
	 */
	protected function _replaceKeywords(&$text)
	{
		if (strlen($text)) {
			replace_keywords($text, $this->_keyword_replacements);
			replace_global_keywords($text);
		}

	}//end _replaceKeywords()
	
	
	/**
	 * Run request and cacheing logic for each URL
	 *
	 * @return void
	 * @access protected
	 */
	protected function _performRequests()
	{
		if ($this->_canCacheResponses()) {
			$cm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
			$rm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('rest_manager');
			
			// For each URL, we must create a serialized object of the request combined with the URL
			// to make a unique cache key for this request.
			// This way, a URL with the same settings will be a cache hit even if it was requested somewhere
			// else as part of a different set of requests.
			for ($key = 0; $key < count($this->_res['request']['urls']); $key++) {
				$url = $this->_res['request']['urls'][$key];

				// We'll need a new request structure for each url
				$current_request = $this->_res['request'];
				$current_request['urls'] = NULL; 
				$current_request['current_url'] = $url;
				$cache_key = serialize($current_request);

				// Attempt to load from cache. 
				// We pin the cache entries against the REST Manager, 
				// so that they can be shared across multiple assets.
				$cache =& $cm->loadFromCache($rm->id, 'cache_manager', $cache_key, FALSE);

				if ($cache !== FALSE) {
					// HIT!
					$this->_res['responses'][$key] = unserialize($cache);
					$this->_res['responses'][$key]['source'] = 'cache';
				} else {
					// MISS!
					$this->_res['responses'][$key] = Array();
					$this->_curlAddUrl($url, $key);
				}
			}
		} else {
			// No cacheing, so get everything.
			for ($key = 0; $key < count($this->_res['request']['urls']); $key++) {
				$this->_res['responses'][$key] = Array();
				$this->_curlAddUrl($this->_res['request']['urls'][$key], $key);
			}
		}

		// Go!
		$this->_curlExec();

	}//end _performRequests()	


	/**
	 * Determine if the request can be cached or not.
	 *
	 * @return boolean
	 * @access protected
	 */
	protected function _canCacheResponses()
	{
		switch ($this->_config['cache_options']) {
		case 'NEVER':
			return FALSE; 
		case 'HTTP':
		case 'DEFAULT':
			switch ($this->_res['request']['method']) {
			case 'GET':
				return TRUE;
			case 'POST':
				return $this->_config['cache_post_requests'];
			default:
				return FALSE;
			}
		}

	}//end _canCacheResponse()


	/**
	 * Adds a URL to be requested.
	 *
	 * @return void
	 * @access protected
	 */
	protected function _curlAddUrl($url, $key)
	{
	  	$ch	= curl_init();

	  	// oauth requires a unique auth header for each url
	  	if (isset($this->_res['request']['oauth_headers'])) {
	  		if (isset($this->_res['request']['oauth_headers'][$url])) {
	  			$this->_res['request']['headers'][] = $this->_res['request']['oauth_headers'][$url];
	  		}
	  	}

		curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
		curl_setopt($ch, CURLOPT_URL, $url);
		curl_setopt($ch, CURLOPT_FOLLOWLOCATION, $this->_config['follow_redirect']);
		curl_setopt($ch, CURLOPT_TIMEOUT, $this->_config['timeout']);
		curl_setopt($ch, CURLOPT_HEADERFUNCTION, Array($this, '_readCurlHeaders'));
		curl_setopt($ch, CURLOPT_HTTPHEADER, $this->_res['request']['headers']);

		// Determine the HTTP method
		switch ($this->_res['request']['method']) {
		case 'GET':
			curl_setopt($ch, CURLOPT_HTTPGET, 1);
			break;
		case 'HEAD':
			curl_setopt($ch, CURLOPT_HTTPGET, 1);
			curl_setopt($ch, CURLOPT_NOBODY, 1);
			break;
		case 'POST':
			curl_setopt($ch, CURLOPT_POST, 1);
			curl_setopt($ch, CURLOPT_POSTFIELDS, $this->_res['request']['body']);
			break;
		case 'PUT':
			curl_setopt($ch, CURLOPT_CUSTOMREQUEST, 'PUT');
			curl_setopt($ch, CURLOPT_POSTFIELDS, $this->_res['request']['body']);
			break;
		case 'DELETE':
			curl_setopt($ch, CURLOPT_CUSTOMREQUEST, 'DELETE');
			break;
		}

		// Set authentication if required.
		switch ($this->_res['request']['auth']['type']) {
		case "HTTP_Basic":
			curl_setopt($ch, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
			curl_setopt($ch, CURLOPT_USERPWD, $this->_res['request']['auth']['http_auth_options']['username'].':'.$this->_res['request']['auth']['http_auth_options']['password']);
			break;
		}

		$this->_urls_to_request[$key] = Array('handle' => $ch, 'url' => $url);

	}//end _curlAddUrl()


	/**
	 * Runs the curl requests.
	 *
	 * @return void
	 */
	protected function _curlExec()
	{
		// Loop over *all* URLs to make sure keys are synced. We only do stuff for the entries in _urls_to_request
		for ($key = 0; $key < count($this->_res['request']['urls']); $key++) {

			// There might be nothing to do for this URL index (perhaps indexes 0 and 1 are cached but 2 isn't)
			if (!isset($this->_urls_to_request[$key]['url'])) continue;

			$response =& $this->_res['responses'][$key];

			// Populate this response.
			$response['body'] = curl_exec($this->_urls_to_request[$key]['handle']);
			$response['headers'] = $this->_headers;
			$response['info'] = curl_getinfo($this->_urls_to_request[$key]['handle']);
			$response['source'] = 'remote';

			// Clean up.
			curl_close($this->_urls_to_request[$key]['handle']);
			$this->_headers = Array();

			// Attempt to cache this response.
			if ($this->_canCacheResponses()) {
				$this->_setCacheExpiry($response);

				// Only cache if we have a valid expiry (passing 0 tells the cache manager to use asset type expiry).
				if ($response['expiry'] > 0) {

					// We'll need a new request structure for each url, for unique cache keys.
					$current_request = $this->_res['request'];
					$current_request['urls'] = NULL;
					$current_request['current_url'] = $this->_urls_to_request[$key]['url'];
					$cache_key = serialize($current_request);

					// Save to cache.
					$cm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
					$rm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('rest_manager');
					$cm->saveToCache($rm->id, 'cache_manager', $cache_key, serialize($response), FALSE, $response['expiry']);
				}
			}
		}

		// Reset now that we've finished, since this can be run twice by paint layouts and the like.
		$this->_urls_to_request = Array();

	}//end _curlExec()


	/**
	 * Calculates the appropriate cache expiry, based on config settings and 
	 * values in the HTTP response headers.
	 *
	 * This is not an exhaustive implementation, but is based on 
	 * RFC2616 http://www.w3.org/Protocols/rfc2616/rfc2616-sec14.html#sec14.9
	 *
	 * @access protected
	 * @return void
	 */
	protected function _setCacheExpiry(&$response)
	{
		// Only HTTP and DEFAULT are possible..
		switch ($this->_config['cache_options']) {
		case 'HTTP':

			// Use HTTP headers to control cacheing behaviour.

			$cache_control = isset($response['headers']['Cache-Control']) ? $response['headers']['Cache-Control'] : NULL;
			$pragma = isset($response['headers']['Pragma']) ? $response['headers']['Pragma'] : NULL;
			$expires = isset($response['headers']['Expires']) ? $response['headers']['Expires'] : NULL;

			// Cache-Control directives take priority over Expires..
			if ((!is_null($cache_control)) || (!is_null($pragma))) {

				// For HTTP 1.0 servers.
				if ($pragma == 'no-cache') {
					$response['expiry'] = 0;
				}

				// Cache-Control directives can be comma separated.
				$cache_directives = split(',', $cache_control);
				foreach ($cache_directives as $directive) {

					// Directives may contain an extension, with a value.
					$cache_ext = split('=', $directive);

					// no-cache and equivalents override max-age
					if (isset($cache_ext[1])) {
						switch (strtolower(trim($cache_ext[0]))) {
						case 'no-cache':
						case 'must-revalidate':
						case 'no-store':
						case 'private':
							$response['expiry'] = 0;
							break;
						case 'public':
						case 's-maxage':
							// s-maxage overrides max-age
							$response['expiry'] = (int) $cache_ext[1];
							break;
						case 'max-age':
							$response['expiry'] = (int) $cache_ext[1];
							break;
						default:
							$response['expiry'] = 0;
						}
					}
				}

			} elseif (!is_null($expires)) {
				// Parse date from the Expires header..
				$time = (strtotime($expires) - time());
				if ($time < 0) $time = 0;
				$response['expiry'] = $time;
			} else {
				$response['expiry'] = 0; 
			}
			break;

		case 'DEFAULT':
			$response['expiry'] = $this->_config['default_cache_expiry'];
			break;
		}
	}//end setCacheExpiry()

	
	/**    
	* Callback to read CURL headers.
	*  
	* @return integer
	* @access protected
	*/    
	protected function _readCurlHeaders($ch, $header)
	{
		// Indicates we're following a redirect.. so we can discard the last set of headers.
		if ($this->_last_header) {
			$this->_headers = Array();
			$this->_last_header = FALSE;
		} else {
			// If we find a blank row, this could be the last header, but we might
			// be following a redirect.
			if (trim($header) == '') {
				$this->_last_header = TRUE;
			} else {
				$pair = split(': ', $header);
				if (count($pair) == 2) {
					$this->_headers[trim($pair[0])] = trim($pair[1]);
				}
			}
		}

		return strlen($header);

	}//end _readCurlHeaders()

}//end class

?>