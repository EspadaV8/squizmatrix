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
* $Id: form_action_soap_call.inc,v 1.2 2010/04/29 04:09:43 ewang Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/form/form_action/form_action.inc';


// WSDL sometime defines a complex type in a loop, we need to get out of this loop if it repeats too many times while parsing
define('MAX_COMPLEX_TYPE_PASRSE_LOOP', 3);

/**
* Form_Action_SOAP_Call
*
* Purpose
* Make a SOAP call before submitting form
*
* @author  Edison Wang <ewang@squiz.com.au>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage web_service
*/


class Form_Action_SOAP_Call extends Form_Action
{


	private static $recent_parsed_type = Array();
	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* Paint inline interface
	*
	*
	*/
	public static function paintInlineInterface(Form $form, $settings, Backend_Outputter $o, $prefix)
	{
		?>

		<p class="sq-backend-section-subheading">Connection Details</p>

		<div id="<?php echo $prefix ?>_connection_div" style="padding: 0.5em 0px 2em 30px;">
			<table class="sq-backend-table" >
				<tr>
					<td width="100"><p>WSDL</p></td>
					<td><p><?php text_box($prefix.'_wsdl', array_get_index($settings, 'wsdl', ''), 120); ?></p></td>
				</tr>
				<tr>
					<td><?php echo translate('login'); ?></td>
					<td><p><?php text_box($prefix.'_login', array_get_index($settings, 'login', ''), 35); ?><br />
					</p></td>
				</tr>
				<tr>
					<td><?php echo translate('password'); ?></td>
					<td><p><?php password_box($prefix.'_password', array_get_index($settings, 'password', ''), 35); ?></p></td>
				</tr>
				<tr>
					<td colspan="2"><?php echo translate('use_http_authentication'); ?></td>
				</tr>
			</table>
		</div>

		<div id="<?php echo $prefix ?>_https_div" style="padding: 0.5em 0px 2em 30px;">
			<table class="sq-backend-table" >
				<tr>
					<td><p><?php echo translate('local_cert'); ?></p></td>
					<td><p><?php asset_finder($prefix.'_local_cert', array_get_index($settings, 'local_cert', ''), Array('file'=>'D')); ?><br/>
				</tr>
				<tr>
					<td><p><?php echo translate('passphrase'); ?></p></td>
					<td><p><?php password_box($prefix.'_passphrase', array_get_index($settings, 'passphrase', ''), 35); ?></p></td>
				</tr>
				<tr>
					<td colspan="2"><?php echo translate('use_https'); ?></td>
				</tr>
			</table>
		</div>
	<?php if(!isset($settings['actions']) || empty($settings['actions'])) return;?>
	
		<p class="sq-backend-section-subheading"><?php echo translate('request_message'); ?></p>

		<div id="<?php echo $prefix ?>_action_selection" style="padding: 0.5em 0px 2em 30px;">
			<table class="sq-backend-table" >
				<tr>
					<td><strong><?php echo translate('action');?></strong><br /><br /></td>
					<td>
						<?php
							foreach ($settings['actions'] as $name => $detail) {
								$action_list[$name] = $name;
							}
							combo_box($prefix.'_action', $action_list, FALSE, array_get_index($settings, 'action', ''));
						?>
						<br /><br />
					</td>
				</tr>
				<tr>
					<td><span style="padding-top:10px;"><strong><?php echo translate('params');?></strong></span></td>
					<td>
						<?php
							$i = 0;
							if(isset($settings['actions'][$settings['action']])) {
								foreach ($settings['actions'][$settings['action']] as $parameter_xml) { 
									text_area($prefix.'_'.$settings['action'].'parameters'.$i, array_get_index($settings,$settings['action'].'parameters'.$i, $parameter_xml), 80, 15);
									echo '<br/>';
									$i++;
								}
							}
						?>
						<br />
					</td>
				</tr>
				<tr>
					<td colspan="2">
						<?php echo translate('request_message_note'); ?>
					</td>
				</tr>
			</table>
		</div>
		
		
		
		<p class="sq-backend-section-subheading"><?php echo translate('response_message'); ?></p>

		<div id="<?php echo $prefix ?>_response" style="padding: 0.5em 0px 2em 30px;">
			<table class="sq-backend-table" >
				<tr>
					<td>
						<?php
							$i = 0;
							if(isset($settings['responses'][$settings['action']])) {
								// echo the response xml, with style
								echo (nl2br(str_replace(' ', '&nbsp;&nbsp;', htmlentities($settings['responses'][$settings['action']]))));
							}
						?>
						<br />
					</td>
				</tr>			
				<tr>
					<td>
						<?php echo translate('response_message_note'); ?>
					</td>
				</tr>
				<tr>
				</tr>
			</table>
		</div>
		
		
			<p class="sq-backend-section-subheading"><?php echo translate('response_keywords'); ?></p>

		<div id="<?php echo $prefix ?>_response_keywords" style="padding: 0.5em 0px 2em 30px;">
			<table class="sq-backend-table" >
				<tr>
					<td>
						<?php
							$i = 0;
							if(isset($settings['response_keywords'][$settings['action']])) {
								// echo the response xml, with style
								echo ($settings['response_keywords'][$settings['action']]);
							}
						?>
						<br />
					</td>
				</tr>			
				<tr>
				</tr>
			</table>
		</div>
			
		<?php

	}//end paintInlineInterface()


	/**
	* Process inline interface
	*
	* @return boolean
	* @access public
	*/
	public static function processInlineInterface(Form $form, &$settings, Backend_Outputter $o, $prefix)
	{

	
		$settings['local_cert']				= isset($_POST[$prefix.'_local_cert']['assetid']) ? $_POST[$prefix.'_local_cert']['assetid']: NULL;
		$settings['wsdl']					= array_get_index($_POST, $prefix.'_wsdl', NULL);
		$settings['login']					= array_get_index($_POST, $prefix.'_login', NULL);
		$settings['password']				= array_get_index($_POST, $prefix.'_password', NULL);
		$settings['passphrase']				= array_get_index($_POST, $prefix.'_passphrase', NULL);
		$settings['action']					= array_get_index($_POST, $prefix.'_action', NULL);

		// This submission needs to be executed before form get submitted
		$settings['before_submit']			= TRUE;

		
		if(isset ($settings['wsdl']) && !empty($settings['wsdl'])) {
			
			$options = self::_getSoapOptions($settings);
			try {
				$client = new SoapClient($settings['wsdl'], $options);
				$functions = $client->__getFunctions();
				$types = $client->__getTypes();
			}
			catch (Exception $e) {
				trigger_error($e->getMessage());
				return TRUE;	
			}

			if(!empty($functions)) {
				foreach ($functions as $function) {
					$function_name = preg_replace('/^\w+ (\w+)\(.*\)/', '$1', $function);
					$function_params = explode(', ', preg_replace('/^\w+ \w+\((.*)\)/', '$1', $function));
					$function_response = preg_replace('/^(\w+) \w+\(.*\)/', '$1', $function);
					
					// parse parameters
					$parsed_params = Array();
					foreach ($function_params as $param) {
						// Get XML representation of params of the functions
						$xml_param = self::_parse_type ($param, $types);
						$parsed_params[$param] = self::_formatXmlString(empty($xml_param) ? $param : $xml_param->asXML());
					}
					$function_list[$function_name] = $parsed_params;
					
					// parse response
					$xml_response = self::_parse_type ($function_response, $types);
					$response_list[$function_name] = self::_formatXmlString(empty($xml_response) ? '' : $xml_response->asXML());
					
					// parse response keywords
					$keywords_list[$function_name] = self::_parse_response_keywords ($xml_response, 'globals_session_soap_');
				}
				$settings['actions'] = $function_list;
				$settings['responses'] = $response_list;
				$settings['response_keywords'] = $keywords_list;	
			}
		}

		// Set up current params for current action
		if(isset($settings['actions'][$settings['action']])) {		
			for($i = 0; $i < count($settings['actions'][$settings['action']]); $i++) {
				if(isset($_POST[$prefix.'_'.$settings['action'].'parameters'.$i]))
					$settings[$settings['action'].'parameters'.$i] = $_POST[$prefix.'_'.$settings['action'].'parameters'.$i];
			}
		}

		return TRUE;

	}//end processInlineInterface()


	/**
	* Paint summary description
	*
	* @return void
	* @access public
	*/
	public static function paintSummary(Form $form, $settings, Backend_Outputter $o, $prefix)
	{
	
		?><table class="no-borders">
			<colgroup>
				<col width="80" />
				<col/>
			</colgroup>
			<tbody>
				<tr>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><strong><?php echo translate('action');?></strong></p></td>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><?php
					if (isset ($settings['action'])) {
						echo $settings['action'];
					} else {
						?><span class="sq-backend-warning"><?php echo translate('no_soap_action');?></span><?php
					}
					
					if (!isset ($settings[$settings['action'].'parameters0'])) {
						?><br/><span class="sq-backend-warning"><?php echo translate('no_parameter');?></span><?php	
					}
					?></p></td>
				</tr>
			</tbody>
		</table>
		<?php

	}//end paintSummary()


	/**
	* Execute form action
	*
	* @param
	* @param
	*
	* @return boolean
	* @access public
	*/
	public static function execute(Form $form, $settings)
	{
		
		if(!isset($settings['actions'][$settings['action']])) {
			$form->submission_errors = array_merge($form->submission_errors, Array(translate('no_soap_action')));
			return FALSE;
		}
		if(!isset($settings['wsdl'])) {
			$form->submission_errors = array_merge($form->submission_errors, Array(translate('no_wsdl')));
			return FALSE;
		}
		
		$function = $settings['action'];
		$wsdl = $settings['wsdl'];
		$options = self::_getSoapOptions($settings);
	

		// Work out the parameters of that soap function
		for($i = 0, $param = Array(); $i < count($settings['actions'][$settings['action']]); $i++) {
			if(isset($settings[$settings['action'].'parameters'.$i])) {	
				$soap_message = self::getKeywordReplacementsFromForm($settings[$settings['action'].'parameters'.$i], $form);		
				$object = @simplexml_load_string($soap_message);

				if($object === FALSE) {
					// Just string argument, replace file content keywords
					$object = self::getFileContentReplacementsFromForm ($soap_message, $form);
				}
				else {
					
					// Convert XML to array, replace file content keywords, xml is not friendly for binary data
					$object = self::_toArray($object, $form);	
				}
				$param[] = $object;
			}
		}



		// Get an array of parameters
		$eval_string = '';
		foreach ($param as $index => $object) {
			$eval_string .= $index == 0 ? '' : ', ';
			$eval_string .= '$param['.$index.']';
		}

		// Make SOAP call		
		try {
			$client = new SoapClient($wsdl, $options);
			eval('$data = $client->$function('.$eval_string.');');
		}
		catch (Exception $e) {
			// SOAP error should be treated as normal form error
			$form->submission_errors = array_merge($form->submission_errors, Array($e->getMessage()));
			return FALSE;
		}

		// Store results in session
		self::_store_results ($data);

		return TRUE;

	}//end execute()


	/**
	* This function recursively checks an object properties and store them in session
	*
	* @return void
	* @access private
	*/
	private static function _store_results($object, $prefix = 'soap_') {
		
		if (empty($object)) return;
		
		$vars = get_object_vars($object);
		if (empty($vars)) return;
		
		foreach ($vars as $index => $value) {
			if (is_object($value)) {
				self::_store_results ($value, $prefix.$index);
			}
			else {
				$_SESSION[SQ_SESSION_SANDBOX_INDEX][$prefix.$index] = $value;
			}
		}
	}//end _store_results()
	
	
	/**
	* This function returns XML representation of param struct
	*
	* @return object | void
	* @access private
	*/
	private static function _parse_type($type, $types) {
		$type_name = preg_replace('/ .*/', '', $type);
		//$param_name = preg_replace('/.* (.*);/', '$1', $param);
		$sub_types = self::_is_complex_type($type_name, $types);

		// Don't worry about simple type arguments
		if (!$sub_types) return NULL;
		
		// reset infinity loop check counter
		self::$recent_parsed_type = Array();
		
		// It's a complext type, let's parse it recursively
		$node = new SimpleXMLElement('<'.$type_name.'/>');
		self::_parse_type_recursive($type, $types, $node);
		return $node;
	}//end _parse_type()
	
	
	/**
	* This function recursively parse the complex param type
	*
	* @return null
	* @access private
	*/
	private static function _parse_type_recursive($param, $types, &$node) {
		// Keep recently parsed type, to do an infinity check
		foreach (self::$recent_parsed_type as $parsed) {
			if ($parsed == $param) return;
		}
		array_push(self::$recent_parsed_type, $param);
		if(count(self::$recent_parsed_type) > MAX_COMPLEX_TYPE_PASRSE_LOOP) {
			array_shift(self::$recent_parsed_type);		
		}
		
		$param_type = preg_replace('/ .*/', '', $param);
		$param_name = preg_replace('/.* (.*);/', '$1', $param);

		$sub_types = self::_is_complex_type($param_type, $types);
		if(!$sub_types) return;

		// It's a complext type, let's parse its children
		foreach ($sub_types as $type) {
			$sub_type_type = preg_replace('/ .*/', '', $type);
			$sub_type_name = preg_replace('/.* (.*);/', '$1', $type);
			$sub_sub_types = self::_is_complex_type($sub_type_type, $types);
			
			if (!$sub_sub_types || ($sub_type_type == $param_type)) {
				// it's a simple type
				$node->addChild($sub_type_name, '??');
			}
			else {
				// it's complex type, so step into its children
				$new_node = $node->addChild($sub_type_name);
				self::_parse_type_recursive($type, $types, $new_node);
				
			}			
		}
	}//end _parse_type_recursive()
	

	/**
	* This function checks if a param is of complex type from types list, if so, return the sub types
	*
	* @return false | array
	* @access private
	*/
	private static function _is_complex_type($param, $types) {
		foreach ($types as $type) {
			if (preg_match ('/struct '.$param.'/', $type)) {
				// find and return sub types
				preg_match_all ('/\w+ \w+;/', $type, $matches);
				return $matches[0];
			}
		}
		return FALSE;
	}//end _parse_param()
	
	
	/**
	* This function formats a xml, inserts indentation, newlines for better display
	*
	* @return string
	* @access private
	*/
	private static function _formatXmlString($xml) {  
	  
	  // add marker linefeeds to aid the pretty-tokeniser (adds a linefeed between all tag-end boundaries)
	  $xml = preg_replace('/(>)(<)(\/*)/', "$1\n$2$3", $xml);
	  
	  // now indent the tags
	  $token      = strtok($xml, "\n");
	  $result     = ''; // holds formatted version as it is built
	  $pad        = 0; // initial indent
	  $matches    = array(); // returns from preg_matches()
	  
	  // scan each line and adjust indent based on opening/closing tags
	  while ($token !== false) : 
	  
	    // test for the various tag states
	    
	    // 1. open and closing tags on same line - no change
	    if (preg_match('/.+<\/\w[^>]*>$/', $token, $matches)) : 
	      $indent=0;
	    // 2. closing tag - outdent now
	    elseif (preg_match('/^<\/\w/', $token, $matches)) :
	      $pad--;
	    // 3. opening tag - don't pad this one, only subsequent tags
	    elseif (preg_match('/^<\w[^>]*[^\/]>.*$/', $token, $matches)) :
	      $indent=1;
	    // 4. no indentation needed
	    else :
	      $indent = 0; 
	    endif;
	    
	    // pad the line with the required number of leading spaces
	    $line    = str_pad($token, strlen($token)+$pad, ' ', STR_PAD_LEFT);
	    $result .= $line . "\n"; // add to the cumulative result, with linefeed
	    $token   = strtok("\n"); // get the next token
	    $pad    += $indent; // update the pad size for subsequent lines    
	  endwhile; 
	  
	  return $result;
	}
	
	
	/**
	* This function recursively extarcts available session keywords from response message
	*
	* @return string
	* @access private
	*/
	private static function _parse_response_keywords (&$response_xml, $prefix = '')
	{
		if (!is_object($response_xml)) return '';
		$data = get_object_vars($response_xml);
		$keywords_list = '';
		foreach ($data as $index => $sub_object) {
			if(is_object($sub_object) && !empty($sub_object)) {
				$keywords_list .= self::_parse_response_keywords($sub_object, $prefix.$index);
			}
			else {
				$keywords_list .= '%'.$prefix.$index.'%<br/>';
			}
		}
		return $keywords_list;
	}//end _parse_response_keywords
	
	
	/**
	* This function replace all the keywords for each of the field
	*
	* @return string
	* @access public
	*/
	public static function getKeywordReplacementsFromForm($content, &$form)
	{
		$matches = Array();
		$found = preg_match_all('/%(response_(\\d+_)?q\\d+(_raw)?)%/U', $content, $set_matches, PREG_SET_ORDER);
		$matches = array_merge($matches, $set_matches);
		$found = preg_match_all('/%(question_name_(\\d+_)?q\\d+(_raw)?)%/U', $content, $set_matches, PREG_SET_ORDER);
		$matches = array_merge($matches, $set_matches);
		$found = preg_match_all('/%(form_submission_.*)%/U', $content, $set_matches, PREG_SET_ORDER);
		$matches = array_merge($matches, $set_matches);

		foreach ($matches as $match) {
			if (empty($match)) continue;
			$count = 0;
			do {		
				$content = preg_replace('/%'.$match[1].'%/U', $form->_getThankYouKeywordReplacement($match[1]), $content, 1, $count);
			} while ($count > 0);
		}//end foreach
		
		
		return $content;
	}//end getKeywordReplacementsFromForm()
	
	/**
	* This function replace the keyword %file_upload_1111_q1% with the uploaded file content
	*
	* @return string
	* @access public
	*/
	public static function getFileContentReplacementsFromForm($content, &$form)
	{
		if(preg_match('|%file_upload_([0-9]*_)?q([0-9]*)?%|', $content, $question_matches)) {
			if (empty($question_matches[1])) {
				// One asset ID
				$section_assetid = $form->id;
				$question_assetid = $question_matches[2];
			} else {
				$section_assetid = str_replace('_', '', $question_matches[1]);
				$question_assetid = $question_matches[2];
			}
			$question_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($section_assetid.':q'.$question_assetid);
			if(empty($question_asset) || get_class($question_asset) != 'Form_Question_Type_File_Upload') return $content;
			if(isset($question_asset->extra_data['filesystem_path'])) {
				$content = file_get_contents($question_asset->extra_data['filesystem_path']);
			}
		}//end foreach	
		return $content;
	}//end getKeywordReplacementsFromForm()

	
	/**
	* Get soap options from setting, e.g http auth, https cert.
	*
	* @return array
	* @access private
	*/
	private static function _getSoapOptions($settings)
	{
	
		// Work out HTTP auth
		if (isset($settings['login']) && isset($settings['password'])) {
			$options['login'] = $settings['login'];
			$options['password'] = $settings['password'];	
		}
		
		// Work out HTTPS certificate and passphrase
		if(!empty($settings['local_cert'])) {
			$file = $GLOBALS['SQ_SYSTEM']->am->getAsset($settings['local_cert']);
			if(empty($file) || !method_exists($file, 'getExistingFile')) {
				$form->submission_errors = array_merge($form->submission_errors,  Array(translate('no_cert')));
				return FALSE;
			}
			$file_info = $file->getExistingFile();
			$options['local_cert']  = $file_info['path'];
		}
		
		if(isset($settings['passphrase'])) {
			$options['passphrase'] = $settings['passphrase'];
		}
		return $options;
	}//end _getSoapOptions()	

	
	/**
	* Converts xml object to array
	* Replace file content keyword
	* 
	* @return array
	* @access public
	*/
	private static function _toArray ($data, &$form)
	{	
	  if (is_object($data)) $data = get_object_vars($data);
	  // Don't play with empty object
	  if(empty($data)) return '';
	  if(is_array($data)) {
	  	$result = Array();
	  	foreach ($data as $index => $element) {
	  		$result[$index] = self::_toArray($element, $form);
	  	}
	  	return $result;
	  }
	  else {
	  	// Also replace file content keyword with actual binary file
	  	return (self::getFileContentReplacementsFromForm($data, $form));
	  }
	}//end _toArray

	
	/**
	* Execute form action
	*
	* @return void
	* @access public
	*/
	public static function isValid(Form $form, $settings)
	{

		// Must have a wsdl
		if (empty($settings['wsdl'])) {
			return FALSE;
		}

		// The SOAP service must have actions
		if (empty($settings['actions'])) {
			return FALSE;
		}//end if

		// The action we try to use must be specified
		if (!isset($settings['action'])) {
			return FALSE;
		}//end if

		// Expect at least one parameter
		if (!isset ($settings[$settings['action'].'parameters0'])) {
			return FALSE;
		}//end if
		return TRUE;

	}//end isValid()

}//end class

?>