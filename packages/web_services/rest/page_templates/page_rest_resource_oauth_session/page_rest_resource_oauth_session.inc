<?php 
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: page_rest_resource_oauth_session.inc,v 1.3 2009/10/07 23:48:34 akarelia Exp $
*
*/

check_ssv_licence();
require_once SQ_PACKAGES_PATH.'/web_services/rest/page_templates/page_rest_resource/page_rest_resource.inc';


/**
* Page_REST_Resource_OAuth_Session
*
* Calls OAuth process and sets authorization info in the session variables
*
* @author  Maki Maruyama <mmaruyama@squiz.co.uk>
* @version $ $
* @package Squiz_UK_Clients
* @subpackage rest
*/
class Page_REST_Resource_OAuth_Session extends Page_REST_Resource
{
	private $_errors = Array();

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
	 * @return void
	 * @access public
	 */
	public function printBody()
	{

		// reset the oauth related session variables to restart the oauth authorization process in one of the following cases:
		// - if the user click on 'Reset Existing Authorization Data'
		// if only request token is available but not access token, which is likely the case that the user didn't complete the authorization process earlier
		if (isset($_POST[$this->getPrefix().'_oauth_reset_session_vars']) || (isset($_SESSION['oauth'][$this->id]['oauth_token']) && !isset($_SESSION['oauth'][$this->id]['access_token']))) {
			$this->resetOAuthSessionVariables();
			unset($_POST[$this->getPrefix().'_oauth_reset_session_vars']);
		}

		// Run the request(s).
		$this->_process();

		// display the responses if access token is not available yet
		// if access token is available, the contents should be controlled in paint layout
		// using oauth_access_token_available keyword
		if (!$this->isAccessTokenAvailable()) {
			if (!empty($this->_res['responses'])) {
				foreach ($this->_res['responses'] as &$resp) {
					// if the http code of the response is over 400, it's an error
					// clear the session variable set so far to prepare for a restart
					if ($resp['info']['http_code'] >= 400) {
						$this->resetOAuthSessionVariables();
					} else {
						// the response is likely to be authorization/login display provided by the service provider 
						echo $resp['body'];
					}
				}
			} else {
				// if we don't have any data in responses, something must have gone wrong somewhere
				// clear the session variables set so far to prepare for a restart
				$this->resetOAuthSessionVariables();
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
		$oauth = $this->getAttribute('oauth');

		// if access token already exists in the session, use the existing access token
		if (isset($_SESSION['oauth'][$this->id]['access_token'])) return;

		$oauth->setKeywordReplacements($this->_extra_replacements);

		// all the available keywords need to be passed to the attribute to have keyword set in the attribute to be replaced
        $available_keywords = $this->getAvailableKeywords();
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

        $oauth->setKeywordReplacements($available_keywords);

		// this function is called when we start the oauth process from requesting a request token
		// and also when the request token has been authorized by the user and the service provider calls back.
		// when it's a call back, the request parameter will have 'oauth_token' and 'oauth_verifier'.
		// if we don't have oauth_token or oauth_verifier, that means we haven't started the oauth process or 
		// something went wrong somewhere -  either way, we should start from scratch
		if (!isset($_GET['oauth_token']) && !isset($_GET['oauth_verifier'])) {
			// request a Request Token from the service provider
			if (!isset($_SESSION['oauth'][$this->id]['oauth_token'])) {
				$this->_res = $oauth->getRequestToken();
				$this->_errors = $oauth->getErrors();

				// if a requst token is returned, have it authorized by the user
				if (isset($this->_res['request_token']['oauth_token']) && isset($this->_res['request_token']['oauth_token_secret'])) {
					// set the token and token secret in the session for later use
					$_SESSION['oauth'][$this->id]['oauth_token'] = $this->_res['request_token']['oauth_token'];
					$_SESSION['oauth'][$this->id]['oauth_token_secret'] = $this->_res['request_token']['oauth_token_secret'];

					$this->_res =& $oauth->authorize($this->_res['request_token']);
					$this->_errors = $oauth->getErrors();
				}
			}
		} else {
			// if we have 'oauth_token' and 'oauth_verifier', we can exchange the authorized request token to an access token.
			// however, if the user hasn't grant access, we still receive token & verifier but it'll be an invalid token
			// and the exchange will fail.
			if (!isset($_SESSION['oauth'][$this->id]['access_token']) && isset($_SESSION['oauth'][$this->id]['oauth_token_secret'])) {
				$this->_res =& $oauth->getAccessToken($_GET['oauth_token'], $_GET['oauth_verifier'], $_SESSION['oauth'][$this->id]['oauth_token_secret']);
				$this->_errors = $oauth->getErrors();

				// if the access token and secret are returned from the service provider, set them in the session variables
				if (isset($this->_res['access_token']['oauth_token']) && isset($this->_res['access_token']['oauth_token_secret'])) {
					$_SESSION['oauth'][$this->id]['access_token'] = $this->_res['access_token']['oauth_token'];
					$_SESSION['oauth'][$this->id]['access_token_secret'] = $this->_res['access_token']['oauth_token_secret'];
				} 
			}
		}

	}//end _process()


	/**
	 * Clear session variables related to OAuth
	 *
	 * @param none
	 *
	 * @return void
	 * @access public
	 */
	public function resetOAuthSessionVariables()
	{
		$oauth_session_vars = isset($_SESSION['oauth'][$this->id]) ? $_SESSION['oauth'][$this->id] : Array();
		foreach ($oauth_session_vars as $session_var_name => $session_var_val) {
			unset($_SESSION['oauth'][$this->id][$session_var_name]);
		}
	}//end resetOAuthSessionVariables()


	/**
	 * Get Authorization Headers for user data access
	 *
	 * @param	Array	urls	urls that needs Authrization Headers for user data acess
	 *
	 * @return	Array	key: urls passed  value: an Authorization Header for OAuth	
	 * @access	public
	 */
	public function getUserDataRequestAuthHeaders($urls=Array())
	{
		$auth_headers = Array();
		if (empty($urls)) return Array();

		$oauth = $this->getAttribute('oauth');

		// if an access token is not available, there is no way user data can be accessed
		// process each url only when we have an access token
		if ($this->isAccessTokenAvailable()) {
			foreach ($urls as $url) {
				$auth_headers[$url] = $oauth->getUserDataAuthHeader($url, $_SESSION['oauth'][$this->id]['access_token'], $_SESSION['oauth'][$this->id]['access_token_secret']);
			}
		}

		return $auth_headers;

	}// end getUserDataRequestAuthHeaders()


	/**
	 * Check if access token has been received
	 *
	 * @param	none
	 *
	 * @return	boolean
	 * @access	public
	 */
	public function isAccessTokenAvailable()
	{
		if (isset($_SESSION['oauth'][$this->id]['access_token']) && isset($_SESSION['oauth'][$this->id]['access_token_secret'])) {
			return TRUE;
		} else {
			return FALSE;
		}
	}//end isAccessTokenAvailable()


	/**
	 * Return the keywords for this asset type.
	 *
	 * @param	none
	 *
	 * @return	array
	 * @access	public
	 */
	public function getAvailableKeywords()
	{
		$res = parent::getAvailableKeywords();
		$res['oauth_reset_session_vars'] = translate('oauth_reset_session_vars_keyword');
		$res['oauth_access_token_available'] = translate('oauth_access_token_available');
		$res['oauth_errors'] = translate('oauth_errors');
		return $res;

	}//end getAvailableKeywords()


	/**
	 * Get keyword replacement for resetting OAuth session variables (submit button)
	 *
	 * @param	none
	 *
	 * @return	string
	 * @access	public
	 */
	public function getOauthResetSessionVarsKeywordReplacement()
	{
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';
		ob_start();
			?>
			<form id="<?php echo $this->getPrefix() ?>_oauth_reset_session_vars_form" method="POST" action="<?php echo current_url(TRUE, TRUE); ?>">
				<?php echo submit_button($this->getPrefix().'_oauth_reset_session_vars', translate('oauth_reset_session_vars')); ?>
			</form>
			<?php
			$replacement = ob_get_contents();

		ob_end_clean();
		return $replacement;

	}// end getOauthResetSessionVarsKeywordReplacement()


	/**
	 * Get keyword replacement for access token available
	 *
	 * @param	none
	 *
	 * @return	boolean (0/1)
	 * @access	public
	 */
	public function getOauthAccessTokenAvailableKeywordReplacement()
	{
		return $this->isAccessTokenAvailable();

	}//end getOauthAccessTokenAvailableKeywordReplacement()


	/**
	 * Get keyword replacement for errors encountered during OAuth process
	 *
	 * @param	none
	 *
	 * @return	string
	 * @access	public
	 */
	public function getOauthErrorsKeywordReplacement()
	{
		return implode('<br />', $this->_errors); 

	}//end getOauthErrorsKeywordReplacement()

}
?>