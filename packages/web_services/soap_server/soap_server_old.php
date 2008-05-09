<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: soap_server_old.php,v 1.1 2008/05/09 04:54:02 hnguyen Exp $
*
*/

ini_set("soap.wsdl_cache_enabled", "0"); 	// disabling WSDL cache
require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';

/**
* Soap Server
*
* Purpose
*
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class Soap_Server extends Page
{

	// Standard Naming for MESSAGE: [function_name]SoapIn(Out)

	const SOAP_XML_SCHEMA_VERSION = 'http://www.w3.org/2001/XMLSchema';
	const SOAP_XML_SCHEMA_INSTANCE = 'http://www.w3.org/2001/XMLSchema-instance';
	const SOAP_SCHEMA_ENCODING = 'http://schemas.xmlsoap.org/soap/encoding/';
	const SOAP_ENVELOP = 'http://schemas.xmlsoap.org/soap/envelope/';
	const SCHEMA_SOAP_HTTP = 'http://schemas.xmlsoap.org/soap/http';
	const SCHEMA_SOAP = 'http://schemas.xmlsoap.org/wsdl/soap/';
	const SCHEMA_WSDL = 'http://schemas.xmlsoap.org/wsdl/';

	private $function_list = Array();
	private $complex_types = Array();
	
	private $message_node;
	private $portType_node;
	private $binding_node;
	private $schema_node;
	
	private $wsdl;
	private $root;
	
	private $ns = 'http://charlie.squiz.net/hnguyen_dev/web_services/soap_server_3';

    /**
    * Constructor
    *
    * @param int    $assetid    the asset id to be loaded
    *
    */
    function __construct($assetid=0)
    {	
		$this->_ser_attrs = TRUE;
        parent::__construct($assetid);

    }//end constructor


    /**
    * Returns an array of all the permitted links type, the type asset and the cardinality
    *
    * @return array
    * @access private
    * @see Asset::_getAllowLinks()
    */
    public function _getAllowedLinks()
    {
        return Array(
                SQ_LINK_TYPE_2 => Array('soap_api' => Array('card' => 'M', 'exclusive' => FALSE)),
                SQ_LINK_TYPE_1 => Array('soap_api' => Array('card' => 'M', 'exclusive' => FALSE)),
                );

    }//end _getAllowedLinks()


	/**
	*
	*
	* @return void
	* @access public
	*/
	function printFrontend()
	{
		if (isset($_SERVER['REQUEST_METHOD']) && $_SERVER['REQUEST_METHOD'] == 'POST') {
			$this->service($HTTP_RAW_POST_DATA);
		} else {
			if (isset($_SERVER['QUERY_STRING']) && strcasecmp($_SERVER['QUERY_STRING'], 'wsdl') == 0) {
				$this->getWSDL();	
			} else {
				parent::printFrontend();
			}//end else
		}//end else

	}//end if


	/**
	*
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		if (!isset($_GET['desc'])) {
			$children = $GLOBALS['SQ_SYSTEM']->am->getChildren($this->id, 'soap_api', FALSE);
			foreach ($children as $api_id => $type_info) {
				$child = $GLOBALS['SQ_SYSTEM']->am->getAsset($api_id);
				$type_code = $type_info[0]['type_code'];

				$functions = $child->attr('function_list');
				echo '<ul>';
				foreach ($functions as $func_name => $switch) {
					//if ($switch && method_exists($child, $func_name)) {
					if ($switch) {
						echo '<li style="padding-bottom:10px;"><a href="'.$_SERVER['PHP_SELF'].'?desc='.$func_name.'&group='.str_replace('soap_api_', '', $type_code).'" style="color:#336699;font-weight:bold;font-size:13px;font-family:Tahoma;">
								'.ucwords($func_name).'
							</a></li><br />';

					}//end if
				}//end if
				echo '</ul>';
			}//end foreach
		} else {
			$func_name = $_GET['desc'];
			if (isset($_GET['group'])) {
				$group = $_GET['group'];
			}//end if

			if (!empty($func_name) && !empty($group)) {
				$GLOBALS['SQ_SYSTEM']->am->includeAsset('soap_api_'.$group);
				$class = new ReflectionClass(str_replace(' ', '_', ucwords(str_replace('_', ' ', 'soap_api_'.$group))));
				$method = $class->getMethod($func_name);
				$comment = $this->cleanComment($method->getDocComment());
				bam($comment);
			}//end if
			
		}//end else
		
	}//end printBody()


	function cleanComment($comment)
	{
		$comment = str_replace('/*', '' , $comment);
		$comment = str_replace('*/', '' , $comment);
		$comment = str_replace('*', '' , $comment);

		return $comment;

	}//end if


	/**
	* Description: This function call SOAP Server extension to handle requests.
	*
	* @return void
	* @access public
	*/
	function service($http_raw_post_data)
	{
		$url = $this->getUrl();
		$server = new SoapServer(NULL, Array ('uri'	=> $this->getUrl()));
		$children = $GLOBALS['SQ_SYSTEM']->am->getChildren($this->id, 'soap_api', FALSE);
		$function_list = Array();
		foreach ($children as $api_id => $type_info) {
			$child = $GLOBALS['SQ_SYSTEM']->am->getAsset($api_id);
			$type_code = $type_info[0]['type_code'];
			$functions = unserialize($child->attr('function_list'));
			$function_list = array_merge($function_list, array_fill_keys(array_keys($functions), $type_code));
		}//end foreach
		

	}//end service()


	/**
	* Description: This function returns the WSDL for this soap server, including all available methods of
	* all APIs linked underneath.
	*
	* @return void
	* @access public
	*/
	private function getWSDL()
	{
		header("Content-type: text/xml");
		//$server = new SoapServer($url.'?wsdl');
		$server = new SoapServer(NULL, Array ('uri'	=> $this->getUrl()));
		echo $this->getXML();

	}//end getWSDL()


	/**
	*
	*
	* @return void
	* @access public
	*/
	private function getXML() 
	{
		$wsdl = new DomDocument("1.0", 'utf-8');
		
		// Set all the header stuff
		$root = $wsdl->createElementNS('http://schemas.xmlsoap.org/wsdl/', 'wsdl:definitions');
		$root->setAttributeNS('http://www.w3.org/2000/xmlns/', 'xmlns:s','http://www.w3.org/2001/XMLSchema');
		$root->setAttributeNS('http://www.w3.org/2000/xmlns/', 'xmlns:tns', $this->ns);
		$root->setAttributeNS('http://www.w3.org/2000/xmlns/', 'xmlns:soap',self::SCHEMA_SOAP);
		$root->setAttributeNS('http://www.w3.org/2000/xmlns/', 'xmlns:wsdl',self::SCHEMA_WSDL);
		$root->setAttributeNS('http://www.w3.org/2000/xmlns/', 'xmlns:soapenc',self::SOAP_SCHEMA_ENCODING);
		$root->setAttribute('targetNamespace', $this->ns);

		// Create the Types Node and set Name
		$types_node = $wsdl->createElementNS(self::SCHEMA_WSDL, 'types');
		$this->schema_node = $wsdl->createElementNS(self::SOAP_XML_SCHEMA_VERSION, 'schema');
		// Lets set to to the instance variable and let it go.
		
		// Create the PortType Node and set Name
		$this->portType_node = $wsdl->createElementNS(self::SCHEMA_WSDL, 'portType');
		$this->portType_node->setAttribute('name', 'MatrixSOAPService');
		
		// Create the Binding Node and set Name
		$binding_node = $wsdl->createElementNS(self::SCHEMA_WSDL, 'binding');
		$binding_node->setAttribute('name', 'MatrixSOAPService');
		$binding_node->setAttribute('type', 'tns:SoapServer');
		
		// Create the Soap Binding Node under Binding Node and set attributes
		$soap_binding_node = $wsdl->createElementNS(self::SCHEMA_SOAP, 'binding');
		$soap_binding_node->setAttribute('style', 'document');
		$soap_binding_node->setAttribute('transport', self::SCHEMA_SOAP_HTTP);
		
		// Assign soap binding node to an instance var so we can append stuff to it
		$this->binding_node = $soap_binding_node;	// Lets set to to the instance variable and let it go.
		
		$this->root = $root;
		$this->wsdl = $wsdl;		
		
		// Append Stuff to all the nodes
		$this->findFuncInfo();

		log_dump($this->complex_types);
		$types_node->appendChild($this->schema_node);
		$this->root->appendChild($types_node);


		// Create the Message Node and set Name
		$io = Array ('in', 'out');
		foreach ($this->function_list as $func_name => $func_info) {
			foreach ($io as $element) {
				// Do the type before doing the messages

				$this->message_node = $wsdl->createElementNS(self::SCHEMA_WSDL, 'message');
				$this->message_node->setAttribute('name', $func_name.'Soap'.ucwords($element)
				);
				$part_node = $wsdl->createElementNS(self::SCHEMA_WSDL, 'part');
				$part_node->setAttribute('name', 'parameters');
				$part_node->setAttribute('element', 'tns:'.$func_name.(($element == 'out')?'Response':'Request'));
				$this->message_node->appendChild($part_node);
				$this->root->appendChild($this->message_node);

			}//end foreach
		}//end foreach


		// Append the Soap Binding Node to the Binding Node
		$binding_node->appendChild($soap_binding_node);

		// Append all the major nodes to root node
		
		$this->root->appendChild($this->portType_node);
		$this->root->appendChild($binding_node);
		
		// Append the root node to the DOM document
		$wsdl->appendChild($root);
		
		// Finally produce XML
		return $this->wsdl->saveXML();

	}//end getXML()


	/**
	*
	*
	* @return void
	* @access public
	*/
	function findFuncInfo()
	{
		$children = $GLOBALS['SQ_SYSTEM']->am->getChildren($this->id, 'soap_api', FALSE);
		foreach ($children as $api_id => $type_info) {
			$child = $GLOBALS['SQ_SYSTEM']->am->getAsset($api_id);
			$type_code = $type_info[0]['type_code'];
			$functions = $child->attr('function_list');
			$this->complex_types = array_merge($this->complex_types, $child->getComplexTypes());

			foreach ($functions as $key => $value) {
				$functions[$key]	= Array (
										'class_name' => $type_code,
									  );
			}//end foreach

			$this->function_list = array_merge($this->function_list, $functions);
		}//end foreach

		foreach ($this->function_list as $func_name => $func_info) {	
				$class = new ReflectionClass(str_replace(' ', '_', ucwords(str_replace('_', ' ', $func_info['class_name']))));
				if (method_exists($func_info['class_name'], $func_name))
				{
					$input_output = Array ('input', 'output');
					
					// Adding OPERATION NODE to BINDING NODE
					$operation_node = $this->wsdl->createElementNS(self::SCHEMA_WSDL, 'operation');
					$operation_node->setAttribute('name', $func_name);
					
					
					// Get a copy of the Node here since we don't need the io_node here.
					$operation_node_2 = clone $operation_node;
					
					// Adding IO node to each of the operations inside SOAP BINDING NODE.
					foreach ($input_output as $element) {
						$io_node = $this->wsdl->createElementNS(self::SCHEMA_WSDL, $element);
						$body_node = $this->wsdl->createElementNS(self::SCHEMA_SOAP, 'body');
						$body_node->setAttribute('use', 'literal');
						$io_node->appendChild($body_node);
						$operation_node->appendChild($io_node);
					}//end foreach
					
					$this->binding_node->appendChild($operation_node);
					
					// Adding OPERATION NODE to PORTTYPE NODE using $operation_node_2
					foreach ($input_output as $element) {
						$io_message_node = $this->wsdl->createElementNS(self::SCHEMA_WSDL, $element);
						$io_message_node->setAttribute('message', 'tns:'.$func_name.'Soap'.ucwords($element));
						$operation_node_2->appendChild($io_message_node);
					}//end foreach
					$this->portType_node->appendChild($operation_node_2);
					
					// Get the PHPDoc comment					
					$method = $class->getMethod($func_name);
					$doc_comment = $method->getDocComment();
					
					$matches = Array();
					preg_match_all('|@param\s+(\w+)\s+(\&)?\$(\w+)|', $doc_comment, $matches);

					$func_args_info	= Array (
										'arg_type'	=> isset($matches[1][0]) ? $matches[1][0] : '',
										'arg_ref'	=> isset($matches[2][0]) ? $matches[2][0] : '',
										'arg_name'	=> isset($matches[3][0]) ? $matches[3][0] : '',
									  );

					$matches_2 = Array();
					preg_match('|@return\s+(\w+)|', $doc_comment, $matches_2);
					$func_args_info['return_type']	= $matches_2[1];

					// Add TYPES
					foreach ($input_output as $element) {
						if (strcasecmp($func_args_info['arg_type'],'string') == 0) {
							$element_node = $this->wsdl->createElementNS(self::SOAP_XML_SCHEMA_VERSION, 'element');
							$element_node->setAttribute('name', $func_name.(($element == 'output')?'Response':'Request'));
							$complex_type_node = $this->wsdl->createElementNS(self::SOAP_XML_SCHEMA_VERSION, 'complexType');
							$sequence_node = $this->wsdl->createElementNS(self::SOAP_XML_SCHEMA_VERSION, 'sequence');
							$subelement_node = $this->wsdl->createElementNS(self::SOAP_XML_SCHEMA_VERSION, 'element');
							$subelement_node->setAttribute('name', $func_args_info['arg_name']);
							$subelement_node->setAttribute('type', 's:'.$func_args_info['arg_type']);
							// If element is optional or not
							if (TRUE) {
								$subelement_node->setAttribute('minOccurs', '0');
								$subelement_node->setAttribute('maxOccurs', '1');
							}//end if
							$sequence_node->appendChild($subelement_node);
							$complex_type_node->appendChild($sequence_node);
							
							$element_node->appendChild($complex_type_node);
							$this->schema_node->appendChild($element_node);
						
						}//end if
					}//end foreach
					
					$this->function_list[$func_name] = array_merge($func_info, $func_args_info);
					 
					//$params = $method->getParameters();
					//foreach ($params as $param) {
						//bam(var_export($param->__toString(),TRUE));
						//bam(var_export($param->getClass(),TRUE));
						//bam(var_export($param->getDeclaringClass(),TRUE));
						//we need to get the return value of the function without using docComment, PIECES OF SHIT!!
					//}//end foreach
				}//end if

		}//end foreach

		//bam($this->function_list);
		log_dump($this->function_list);

	}//end findFuncInfo()



	/**
	*
	*
	* @return void
	* @access public
	*/
	private function emptys()
	{

	}//end empty()


}//end class
?>
