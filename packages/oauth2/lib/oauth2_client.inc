<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
*$Id: oauth2_client.inc,v 1.2 2013/08/05 01:55:10 ewang Exp $
*
*/


/**
* Oauth2 library
*
*
* @author  Edison Wang <ewang@squiz.com.au>
* @version $Revision: 1.2 $
* @package MySource_Matrix
*/
class Oauth2_Client extends MySource_Object
{
	/**
	* Different AUTH method
	*/
	    const AUTH_TYPE_URI = 0;
	    const AUTH_TYPE_AUTHORIZATION_BASIC = 1;
	    const AUTH_TYPE_FORM = 2;

	    /**
	* Different Access token type
	*/
	    const ACCESS_TOKEN_URI = 0;
	    const ACCESS_TOKEN_BEARER = 1;
	    const ACCESS_TOKEN_OAUTH = 2;
	    const ACCESS_TOKEN_MAC = 3;

	    /**
	* Different Grant types
	*/
	    const GRANT_TYPE_AUTH_CODE = 'authorization_code';
	    const GRANT_TYPE_PASSWORD = 'password';
	    const GRANT_TYPE_CLIENT_CREDENTIALS = 'client_credentials';
	    const GRANT_TYPE_REFRESH_TOKEN = 'refresh_token';

	    /**
	* HTTP Methods
	*/
	    const HTTP_METHOD_GET = 'GET';
	    const HTTP_METHOD_POST = 'POST';
	    const HTTP_METHOD_PUT = 'PUT';
	    const HTTP_METHOD_DELETE = 'DELETE';
	    const HTTP_METHOD_HEAD = 'HEAD';

	    /**
	* HTTP Form content types
	*/
	    const HTTP_FORM_CONTENT_TYPE_APPLICATION = 0;
	    const HTTP_FORM_CONTENT_TYPE_MULTIPART = 1;

	    /**
	* Client ID
	*
	* @var string
	*/
	    protected $client_id = null;

	    /**
	* Client Secret
	*
	* @var string
	*/
	    protected $client_secret = null;

	    /**
	* Client Authentication method
	*
	* @var int
	*/
	    protected $client_auth = self::AUTH_TYPE_URI;

	    /**
	* Access Token
	*
	* @var string
	*/
	    protected $access_token = null;

	    /**
	* Access Token Type
	*
	* @var int
	*/
	    protected $access_token_type = self::ACCESS_TOKEN_URI;

	    /**
	* Access Token Secret
	*
	* @var string
	*/
	    protected $access_token_secret = null;

	    /**
	* Access Token crypt algorithm
	*
	* @var string
	*/
	    protected $access_token_algorithm = null;

	    /**
	* Access Token Parameter name
	*
	* @var string
	*/
	    protected $access_token_param_name = 'access_token';

	    /**
	* cURL options
	*
	* @var array
	*/
	    protected $curl_options = array();
	    
	    
	   /**
	* verify SSL peer
	*
	* @var bool
	*/    
	    protected  $verify_ssl_peer = false;



	    /**
	* Construct
	*
	* @param string $client_id Client ID
	* @param string $client_secret Client Secret
	* @param int $client_auth (AUTH_TYPE_URI, AUTH_TYPE_AUTHORIZATION_BASIC, AUTH_TYPE_FORM)
	* @param string $ Indicates if we want to use a certificate file to trust the server. Optional, defaults to null.
	* @param bool $ If we want to verify SSL peer. Optional, defaults to fasle.
	* @return void
	*/
	    public function __construct($client_id, $client_secret, $client_auth = self::AUTH_TYPE_URI, $verify_ssl_peer = false)
	    {
		if (!extension_loaded('curl')) {
		    throw new Oauth_Exception('The PHP exention curl must be installed to use this library.', Oauth_Exception::CURL_NOT_FOUND);
		}

		$this->client_id = $client_id;
		$this->client_secret = $client_secret;
		$this->client_auth = $client_auth;
		$this->verify_ssl_peer = $verify_ssl_peer;
	    }

	    /**
	* Get the client Id
	*
	* @return string Client ID
	*/
	    public function getClientId()
	    {
		return $this->client_id;
	    }

	    /**
	* Get the client Secret
	*
	* @return string Client Secret
	*/
	    public function getClientSecret()
	    {
		return $this->client_secret;
	    }

	    /**
	* getAuthenticationUrl
	*
	* @param string $auth_endpoint Url of the authentication endpoint
	* @param string $redirect_uri Redirection URI
	* @param array $extra_parameters Array of extra parameters like scope or state (Ex: array('scope' => null, 'state' => ''))
	* @return string URL used for authentication
	*/
	    public function getAuthenticationUrl($auth_endpoint, $redirect_uri, array $extra_parameters = array())
	    {
		$parameters = array_merge(array(
		    'response_type' => 'code',
		    'client_id' => $this->client_id,
		    'redirect_uri' => $redirect_uri
		), $extra_parameters);
		$symbol = strpos($auth_endpoint, '?') !== FALSE ? '&' : '?';
		return $auth_endpoint . $symbol . http_build_query($parameters, null, '&');
	    }

	    /**
	* getAccessToken
	*
	* @param string $token_endpoint Url of the token endpoint
	* @param int $grant_type Grant Type ('authorization_code', 'password', 'client_credentials', 'refresh_token', or a custom code (@see GrantType Classes)
	* @param array $parameters Array sent to the server (depend on which grant type you're using)
	* @return array Array of parameters required by the grant_type (CF SPEC)
	*/
	    public function getAccessToken($token_endpoint, $grant_type, array $parameters)
	    {
		if (!$grant_type) {
		    throw new Oauth_InvalidArgumentException('The grant_type is mandatory.', Oauth_InvalidArgumentException::INVALID_GRANT_TYPE);
		}

		$parameters['grant_type'] = $grant_type;
		$http_headers = array();
		switch ($this->client_auth) {
		    case self::AUTH_TYPE_URI:
		    case self::AUTH_TYPE_FORM:
			$parameters['client_id'] = $this->client_id;
			$parameters['client_secret'] = $this->client_secret;
			break;
		    case self::AUTH_TYPE_AUTHORIZATION_BASIC:
			$http_headers['Authorization'] = 'Basic ' . base64_encode($this->client_id . ':' . $this->client_secret);
			break;
		    default:
			throw new Oauth_Exception('Unknown client auth type.', Oauth_Exception::INVALID_CLIENT_AUTHENTICATION_TYPE);
			break;
		}

		return $this->executeRequest($token_endpoint, $parameters, self::HTTP_METHOD_POST, $http_headers, self::HTTP_FORM_CONTENT_TYPE_APPLICATION);
	    }

	    /**
	* setToken
	*
	* @param string $token Set the access token
	* @return void
	*/
	    public function setAccessToken($token)
	    {
		$this->access_token = $token;
	    }

	    /**
	* Set the client authentication type
	*
	* @param string $client_auth (AUTH_TYPE_URI, AUTH_TYPE_AUTHORIZATION_BASIC, AUTH_TYPE_FORM)
	* @return void
	*/
	    public function setClientAuthType($client_auth)
	    {
		$this->client_auth = $client_auth;
	    }

	    /**
	* Set an option for the curl transfer
	*
	* @param int $option The CURLOPT_XXX option to set
	* @param mixed $value The value to be set on option
	* @return void
	*/
	    public function setCurlOption($option, $value)
	    {
		$this->curl_options[$option] = $value;
	    }

	    /**
	* Set multiple options for a cURL transfer
	*
	* @param array $options An array specifying which options to set and their values
	* @return void
	*/
	    public function setCurlOptions($options)
	    {
		$this->curl_options = array_merge($this->curl_options, $options);
	    }

	    /**
	* Set the access token type
	*
	* @param int $type Access token type (ACCESS_TOKEN_BEARER, ACCESS_TOKEN_MAC, ACCESS_TOKEN_URI)
	* @param string $secret The secret key used to encrypt the MAC header
	* @param string $algorithm Algorithm used to encrypt the signature
	* @return void
	*/
	    public function setAccessTokenType($type, $secret = null, $algorithm = null)
	    {
		$this->access_token_type = $type;
		$this->access_token_secret = $secret;
		$this->access_token_algorithm = $algorithm;
	    }

	    /**
	* Fetch a protected ressource
	*
	* @param string $protected_ressource_url Protected resource URL
	* @param array $parameters Array of parameters
	* @param string $http_method HTTP Method to use (POST, PUT, GET, HEAD, DELETE)
	* @param array $http_headers HTTP headers
	* @param int $form_content_type HTTP form content type to use
	* @return array
	*/
	    public function fetch($protected_resource_url, $parameters = array(), $http_method = self::HTTP_METHOD_GET, array $http_headers = array(), $form_content_type = self::HTTP_FORM_CONTENT_TYPE_MULTIPART)
	    {
		if ($this->access_token) {
		    switch ($this->access_token_type) {
			case self::ACCESS_TOKEN_URI:
			    if (is_array($parameters)) {
				$parameters[$this->access_token_param_name] = $this->access_token;
			    } else {
				throw new Oauth_InvalidArgumentException(
				    'You need to give parameters as array if you want to give the token within the URI.',
				    Oauth_InvalidArgumentException::REQUIRE_PARAMS_AS_ARRAY
				);
			    }
			    break;
			case self::ACCESS_TOKEN_BEARER:
			    $http_headers['Authorization'] = 'Bearer ' . $this->access_token;
			    break;
			case self::ACCESS_TOKEN_OAUTH:
			    $http_headers['Authorization'] = 'OAuth ' . $this->access_token;
			    break;
			case self::ACCESS_TOKEN_MAC:
			    $http_headers['Authorization'] = 'MAC ' . $this->generateMACSignature($protected_resource_url, $parameters, $http_method);
			    break;
			default:
			    throw new Oauth_Exception('Unknown access token type.', Oauth_Exception::INVALID_ACCESS_TOKEN_TYPE);
			    break;
		    }
		}
		return $this->executeRequest($protected_resource_url, $parameters, $http_method, $http_headers, $form_content_type);
	    }

	    /**
	* Generate the MAC signature
	*
	* @param string $url Called URL
	* @param array $parameters Parameters
	* @param string $http_method Http Method
	* @return string
	*/
	    private function generateMACSignature($url, $parameters, $http_method)
	    {
		$timestamp = time();
		$nonce = uniqid();
		$parsed_url = parse_url($url);
		if (!isset($parsed_url['port']))
		{
		    $parsed_url['port'] = ($parsed_url['scheme'] == 'https') ? 443 : 80;
		}
		if ($http_method == self::HTTP_METHOD_GET) {
		    if (is_array($parameters)) {
			$parsed_url['path'] .= '?' . http_build_query($parameters, null, '&');
		    } elseif ($parameters) {
			$parsed_url['path'] .= '?' . $parameters;
		    }
		}

		$signature = base64_encode(hash_hmac($this->access_token_algorithm,
			    $timestamp . "\n"
			    . $nonce . "\n"
			    . $http_method . "\n"
			    . $parsed_url['path'] . "\n"
			    . $parsed_url['host'] . "\n"
			    . $parsed_url['port'] . "\n\n"
			    , $this->access_token_secret, true));

		return 'id="' . $this->access_token . '", ts="' . $timestamp . '", nonce="' . $nonce . '", mac="' . $signature . '"';
	    }

	    /**
	* Execute a request (with curl)
	*
	* @param string $url URL
	* @param mixed $parameters Array of parameters
	* @param string $http_method HTTP Method
	* @param array $http_headers HTTP Headers
	* @param int $form_content_type HTTP form content type to use
	* @return array
	*/
	    private function executeRequest($url, $parameters = array(), $http_method = self::HTTP_METHOD_GET, array $http_headers = null, $form_content_type = self::HTTP_FORM_CONTENT_TYPE_MULTIPART)
	    {
		$curl_options = array(
		    CURLOPT_RETURNTRANSFER => true,
		    CURLOPT_SSL_VERIFYPEER => true,
		    CURLOPT_CUSTOMREQUEST => $http_method
		);

		switch($http_method) {
		    case self::HTTP_METHOD_POST:
			$curl_options[CURLOPT_POST] = true;
			/* No break */
		    case self::HTTP_METHOD_PUT:

	/**
	* Passing an array to CURLOPT_POSTFIELDS will encode the data as multipart/form-data,
	* while passing a URL-encoded string will encode the data as application/x-www-form-urlencoded.
	* http://php.net/manual/en/function.curl-setopt.php
	*/
			if(is_array($parameters) && self::HTTP_FORM_CONTENT_TYPE_APPLICATION === $form_content_type) {
			    $parameters = http_build_query($parameters, null, '&');
			}
			$curl_options[CURLOPT_POSTFIELDS] = $parameters;
			break;
		    case self::HTTP_METHOD_HEAD:
			$curl_options[CURLOPT_NOBODY] = true;
			/* No break */
		    case self::HTTP_METHOD_DELETE:
		    case self::HTTP_METHOD_GET:
			$symbol = strpos($url, '?') !== FALSE ? '&' : '?';
			if (is_array($parameters)) {
			    $url .= $symbol . http_build_query($parameters, null, '&');
			} elseif ($parameters) {
			    $url .= $symbol . $parameters;
			}
			break;
		    default:
			break;
		}

		$curl_options[CURLOPT_URL] = $url;

		if (is_array($http_headers)) {
		    $header = array();
		    foreach($http_headers as $key => $parsed_urlvalue) {
			$header[] = "$key: $parsed_urlvalue";
		    }
		    $curl_options[CURLOPT_HTTPHEADER] = $header;
		}

		$ch = curl_init();
		curl_setopt_array($ch, $curl_options);
		
		// https handling
		if (substr($url, 0, 5) == 'https') {
		    if ($this->verify_ssl_peer) {
			curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, true);
			curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, 2);
		    }
		    else {
			curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
		    }
		}
		
		if (!empty($this->curl_options)) {
		    curl_setopt_array($ch, $this->curl_options);
		}
		
		/*
		 *  use Matrix proxy setting
		 removed proxy support because most of HTTPS urls of oauth providers doesn't play well with proxy anyway
    		static $_proxies_loaded = FALSE;
		$proxy_set = FALSE;
		if ($_proxies_loaded === FALSE) {
			$proxy_auth_file = SQ_DATA_PATH.'/private/conf/proxy_authentication.inc';
			if (file_exists($proxy_auth_file) === TRUE) {
				require_once $proxy_auth_file;
				$_proxies_loaded = TRUE;
			}
		}

		if ($_proxies_loaded === TRUE && defined('SQ_PA_ENABLED') && SQ_PA_ENABLED) {
			$proxy = get_proxy_info_for_url($url);
			if (empty($proxy['host']) === FALSE) {
				// Only set this proxytunnel option if we're going to a https url.
				// Otherwise you end up getting 405 errors.
				if (substr($url, 0, 5) == 'https') {
					curl_setopt($ch, CURLOPT_HTTPPROXYTUNNEL, TRUE);
				}
				curl_setopt($ch, CURLOPT_PROXY, $proxy['host']);
				if ($proxy['port']) {
					curl_setopt($ch, CURLOPT_PROXYPORT, $proxy['port']);
				}
				if (isset($options['proxy']) === TRUE) {
					if (isset($options['proxy']['user']) === TRUE && 
					isset($options['proxy']['password']) === TRUE) {
						curl_setopt($ch, CURLOPT_PROXYUSERPWD, $options['proxy']['user'].':'.$options['proxy']['password']);
					}
				} else {
					if ($proxy['user'] && $proxy['password']) {
						curl_setopt($ch, CURLOPT_PROXYUSERPWD, $proxy['user'].':'.$proxy['password']);
					}
				}
				$proxy_set = TRUE;
			}
		}//end if
		if(!$proxy_set) curl_setopt($ch, CURLOPT_PROXY, NULL);
		*/
	
		
		$result = curl_exec($ch);
		$http_code = curl_getinfo($ch, CURLINFO_HTTP_CODE);
		$content_type = curl_getinfo($ch, CURLINFO_CONTENT_TYPE);
		if ($curl_error = curl_error($ch)) {
		    throw new Oauth_Exception($curl_error, Oauth_Exception::CURL_ERROR);
		} else {
		    $json_decode = json_decode($result, true);
		}
		curl_close($ch);

		return array(
		    'result' => (null === $json_decode) ? $result : $json_decode,
		    'code' => $http_code,
		    'content_type' => $content_type
		);
	    }

	    /**
	* Set the name of the parameter that carry the access token
	*
	* @param string $name Token parameter name
	* @return void
	*/
	    public function setAccessTokenParamName($name)
	    {
		$this->access_token_param_name = $name;
	    }

}//end class


class Oauth_Exception extends Exception
{
    const CURL_NOT_FOUND = 0x01;
    const CURL_ERROR = 0x02;
    const GRANT_TYPE_ERROR = 0x03;
    const INVALID_CLIENT_AUTHENTICATION_TYPE = 0x04;
    const INVALID_ACCESS_TOKEN_TYPE = 0x05;
}

class Oauth_InvalidArgumentException extends InvalidArgumentException
{
    const INVALID_GRANT_TYPE = 0x01;
    const CERTIFICATE_NOT_FOUND = 0x02;
    const REQUIRE_PARAMS_AS_ARRAY = 0x03;
    const MISSING_PARAMETER = 0x04;
}


?>
