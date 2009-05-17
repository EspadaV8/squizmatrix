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
* $Id: form_question_rule_type_file_virus_check.inc,v 1.1 2009/05/15 06:34:39 ewang Exp $
*
*/


require_once dirname(__FILE__).'/../../form_question_rule/form_question_rule.inc';
require_once SQ_FUDGE_PATH.'/antivirus/antivirus.inc';

/**
* Form_Question_Rule_Type_File_Virus_Check
*
* Purpose
*
*
* @author  Edison Wang <ewang@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Form_Question_Rule_Type_File_Virus_Check extends Form_Question_Rule
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor



	/**
	* processes a test when the question is being processed
	*
	* The value being tested in this case is the PHP temp file which is stored in /tmp/
	* If virus is detected, it will be removed, and test failed.
	*
	* @param mixed	$answer			the answer being passed to us
	* @param string	$operator		the operator being used to compare
	* @param mixed	&$comparison	either a value or a form_question asset (based on
	*								comparison question id), being used to compare
	*
	* @static
	* @access public
	* @return boolean true only if rule succeeds
	*/
	function evaluate($answer, $rule_data, &$q_asset)
	{
		$answer = $_FILES['q'.$q_asset->id];
		$temp_file = $answer['tmp_name'];
		$answer_name = $answer['name'];
		
		// if no file uploaded, no need to scan virus
		if ((empty($answer_name) || $answer_name == 'No file uploaded') && !isset($temp_file)) return TRUE;
		
		// if disabled, no need to proceed
		if(isset($rule_data['check_virus']) && $rule_data['check_virus'] == 2) return TRUE;
		
		$report;	
		$status = Antivirus::scan_file($temp_file,$report);
		// If infected, remove and warn the user
		if (!$status) {
			unlink($temp_file);
			return FALSE;
		}

		return TRUE;
	}//end evaluate()


	/**
	* writes JavaScript code for client side validation
	* in this case, it will display a indicator saying 'virus check is underway, please wait..'
	*
	* @param object form_question	&$q_asset		reference to the question asset
	* @param string					$operator		the operator being used to compare
	* @param mixed					&$comparison	either a value or a form_question asset (based on
	*												comparison question id), being used to compare
	*
	* @access public
	* @return boolean
	*/
	function generateJSCode(&$q_asset, $rule_data)
	{
		// Generate a customisable indicator DIV
		
		// if disabled
		if(isset($rule_data['check_virus']) && $rule_data['check_virus'] == 2) return '';
		
		$content = isset($rule_data['indicator_content']) && !empty ($rule_data['indicator_content']) ? $rule_data['indicator_content'] : translate('core_form_rule_file_virus_check_indicator_content_default');
		$class = isset($rule_data['indicator_class']) && !empty ($rule_data['indicator_class']) ? $rule_data['indicator_class'] : translate('core_form_rule_file_virus_check_indicator_class_default');
		ob_start();
		?>
		var newdiv = document.createElement('div');
		var divIdName = 'file_upload_virus_check_indicator';
		if(document.getElementById(divIdName) == null ){
			var indicator_content = '<?php echo $content; ?>';
			newdiv.setAttribute('id',divIdName);
			newdiv.setAttribute('class','<?php echo $class; ?>');
			newdiv.innerHTML = indicator_content;
			form.appendChild(newdiv);
		}
		<?php
		$contents = ob_get_contents();
		ob_end_clean();
		return $contents;

	}//end generateJSCode()


	/**
	* returns a default error message for this rule
	*
	* @param object Form_Question	&$q_asset	the question asset that failed
	* @param array					$rule_data	the rule data
	*
	* @return string	the error message
	* @access public
	*/
	function defaultError(&$q_asset, $rule_data)
	{
		return translate('core_form_rule_file_virus_check_error_true', $q_asset->getValue(), $q_asset->attr('name'));

	}//end defaultError()


}//end class
?>