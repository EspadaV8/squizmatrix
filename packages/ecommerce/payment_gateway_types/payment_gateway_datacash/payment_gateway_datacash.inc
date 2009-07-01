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
* $Id: payment_gateway_datacash.inc,v 1.1 2009/06/22 09:27:23 ata Exp $
*
*/


require_once dirname(__FILE__).'/../../payment_gateway/payment_gateway.inc';
require_once dirname(__FILE__).'/datacash_processor.inc';


/**
* Payment_Gateway_Datacash
*
* Interface for online credit card processing using Datacash payment gateway
*
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage ecommerce
*/
class Payment_Gateway_Datacash extends Payment_Gateway
{

	
	/**
	 * The response from Datacash with Card Holder Verification redirect information such as Access Control Server (ACS) URL, Payment Authentication Request (PAReq) and Datacash reference 
	 * @var array
	 */
	private $_redirect_info = NULL;
	
	/**
	 * The formatted last 4 digits of card number string, e.g. ****-****-****-1234
	 * @var string
	 */
	private $_formatted_card_no = '';
	
	
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
	* Perform any additional processing required during the creation of this asset
	*
	* Datacash Payment Gateway creates a bodycopy asset when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	function _createAdditional(&$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;

		return ($this->_createBodycopy('display_format', translate('default_bodycopy_content')) && $this->_createBodycopy('card_holder_verification', translate('card_holder_verification_bodycopy_content')));

	}//end _createAdditional()


	/**
	* Create Bodycopy
	*
	* @param string	$link_value		the link value of the link between this Datacash Payment Gateway asset and the new bodycopy
	* @param string	$content		the content to put into the bodycopy
	*
	* @return string
	* @access private
	*/
	function _createBodycopy($link_value, $content)
	{
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
		$bodycopy = new Bodycopy();
		$copy_link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1, 'value' => $link_value);

		$bodycopy->setAttrValue('name', ucwords(str_replace('_',' ', $link_value)));
		$args = Array('content' => $content);
		if ($bodycopy->create($copy_link, $args)) {
			$child_bodycopy = $GLOBALS['SQ_SYSTEM']->am->getLink($bodycopy->id, SQ_LINK_TYPE_2, 'bodycopy_div');
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($child_bodycopy['minorid'], 'attributes');
			return TRUE;
		}
		return FALSE;

	}//end _createBodycopy()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowedLinks()
	*/
	function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		$page_links[SQ_LINK_TYPE_2]['bodycopy'] = Array('card' => 2,   'exclusive' => TRUE);
		return $page_links;

	}//end _getAllowedLinks()


	/**
	* Return the available keywords
	*
	* The return value is in the form:
	* <pre>
	* Array(
	*     'keyword' => 'name',
	* )
	* </pre>
	*
	* @return array
	* @access public
	*/
	function getAvailableKeywords()
	{
		$keywords = parent::getAvailableKeywords();

		$keywords['card_number'] = 'Card Number';
		$keywords['card_start_month'] = 'Card Start Month';
		$keywords['card_start_year'] = 'Card Start Year';
		$keywords['card_expiry_month'] = 'Card Expiry Month';
		$keywords['card_expiry_year'] = 'Card Expiry Year';
		$keywords['card_issue_number'] = 'Card Issue Number';
		$keywords['transaction_amount'] = 'Transaction Amount';
		$keywords['processing_error'] = 'Processing Error (empty if no error)';
		$keywords['submit_button'] = 'Submit Button';
		$keywords['cancel_button'] = 'Cancel Button';
		$keywords['reset_button'] = 'Reset Button';
		$keywords['card_holder_verification_form'] = 'Card Holder Verification Form';
		
		return $keywords;

	}//end getAvailableKeywords()


	/**
	* Add valid keywords for this asset to an array of keywords when asked
	*
	* @param object	&$broadcaster	the asset that triggered the event
	* @param array	$vars			the vars that get submitted by the broadcaster
	*								we add keywords to the $vars['keywords'] array
	*
	* @return boolean
	* @access private
	*/
	function onRequestKeywords(&$broadcaster, $vars=Array())
	{
		$keywords = $this->getAvailableKeywords();
		$vars['keywords'] = array_merge($vars['keywords'], $keywords);

	}//end onRequestKeywords()


	/**
	* Basic output function. root of output process
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		//this page is redirected to by the child iframe with unknown reference error
		if (array_get_index($_GET, 'unknown_referer', FALSE)) {
			trigger_error('Unknown caller reference');
			exit;
		}
		
		//this page is redirected to by the child iframe with an error message
		if (array_get_index($_GET, 'error_message', FALSE)) {
			$this->_setError($_GET['error_message']);
		}
		
		$success = FALSE;
		$is_in_frame = FALSE; //indicates whether this page is in the iframe after it is redirected to by the ACS
		
		if (empty($_POST['PaRes']) || empty($_POST['MD'])) {
			//THIS PAGE IS REDIRECTED TO BY DELIVERY METHOD OR FORM ECOMMERCE ASSET NOT BY THE ACS
			$amount = array_get_index($_SESSION,'SQ_ECOM_AMOUNT', 0);
			$card_info = array_get_index($_REQUEST, $this->getPrefix().'_card', FALSE);
			$action = array_get_index($_REQUEST, $this->getPrefix().'_action', FALSE);
			
			if ($action == 'Cancel') {
				$this->returnToCaller('CANCEL');
	
			} else if (empty($amount)) {
				unset($_SESSION['SQ_ECOM_AMOUNT']);
				$this->returnToCaller('SUCCESS');
	
			} else if ($card_info) {
				$success = $this->processPayment($card_info);
				//if success = 2, redirect to the Access Control Server (ACS) for Card Holder Verification
				if ($success === 2) {
					//store the Datacash reference number to authorize payment after the card holder verify himself
					$_SESSION['SQ_ECOM_TRANSACTION_REF_NO'] = $this->_redirect_info['TRANSACTION'];
					//store the last 4 digits of card number to display it later in Form Ecommerce or Delivery Method through the Session variable SQ_ECOM_RESPONSE
					$_SESSION['SQ_ECOM_CARD_NUMBER'] = $this->_formatted_card_no;
					//print the Card Holder Verification bodycopy
					$this->printBodycopy('card_holder_verification');
					return;
				}
			}
			
		} else if (!empty($_SESSION['SQ_ECOM_TRANSACTION_REF_NO']) && ($_POST['MD'] == md5($_SESSION['SQ_ECOM_REF_NO']))) {
			//THIS PAGE IS REDIRECTED TO BY THE ACCESS CONTROL SERVER (ACS) OF ISSUEING BANK AFTER CARD HOLDER VERIFICATION PROCESS
			$is_in_frame = TRUE;
			$datacash_ref = $_SESSION['SQ_ECOM_TRANSACTION_REF_NO'];
			$this->_formatted_card_no = array_get_index($_SESSION, 'SQ_ECOM_CARD_NUMBER', '');
			unset($_SESSION['SQ_ECOM_TRANSACTION_REF_NO']);
			unset($_SESSION['SQ_ECOM_CARD_NUMBER']);
			$success = $this->authorizePayment($datacash_ref, $_POST['PaRes']);
			if ($success === FALSE) {
				//transaction fails, redirect the parent document of the iframe and display error
				$this->_redirectParent($this->getURL().'?error_message='.urlencode($this->_tmp['error_message'][0]));
			}
		}

		//if transaction is accepted, redirect the user to the success page
		if ($success === 1) {
			unset($_SESSION['SQ_ECOM_AMOUNT']);
			$this->returnToCaller('SUCCESS', $is_in_frame);
		}
		
		//print default bodycopy
		$this->printBodycopy('display_format');
		
	}//end printBody()


	/**
	* Prints the front-end interface for this asset
	*
	* @return void
	* @access public
	*/
	function printBodycopy($link_value)
	{
		$bodycopy = $this->getBodycopy($link_value);
		if (is_null($bodycopy)) return;

		$keywords = $bodycopy->getKeywords();
		$replacements = Array();
		foreach ($keywords as $word) {
			$replacements[$word] = $this->getKeywordReplacement($word);
		}
		$bodycopy->setKeywordReplacements($replacements);

		//OUTPUT HERE
		if ($link_value == 'display_format') {
			//the default body copy with card input
			$datapath = sq_web_path('data').'/asset_types/payment_gateway_datacash/files';
			$card = $this->getPrefix().'_card';
	
			if ($this->attr('test_mode')) {
				echo '<h2 class="gateway-test-mode" style="color: white; background-color: #C00; padding: 5px">TEST MODE</h2>';
			}
	
			?>
			<script src="<?php echo $datapath; ?>/loader.js"></script>
			<script>
				var Loader = new Loader('<?php echo $card; ?>','#FFFFFF','Processing Transaction...','<?php echo $datapath; ?>/loader.gif');
				Loader.print();
			</script>
			<form method="post" action="<?php echo $this->getURL(); ?>" onsubmit="Loader.show();">
			<?php
			$bodycopy->printBody();
			echo '</form>';
		} else {
			//the card holder verification bodycopy
			$bodycopy->printBody();
		}

	}//end printBodycopy()


	/**
	* Return the bodycopy linked to this page
	*
	* @return object
	* @access public
	*/
	function getBodycopy($link_value)
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, $link_value);
		if (empty($link)) return NULL;
		$bodycopy = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
		return $bodycopy;

	}//end getBodycopy()


	/**
	* Process credit card details to gateway
	*
	* @param array	$card_info	Submitted credit card information
	*
	* @return mixed	return FALSE if the card is not valid or transaction fails, return 1 if SUCCESS, return 2 if need to REDIRECT in 3-D Secure check
	* @access public
	*/
	function processPayment($card_info)
	{
		if (!$card_info) return FALSE;

		$amount = $_SESSION['SQ_ECOM_AMOUNT'];

		$processor = new Datacash_Processor($this->attr('client'), $this->attr('password'), $this->attr('datacash_api_path'), $this->attr('cardinfo_dir_path'), $this->attr('timeout'), $this->attr('test_mode'));

		//SET AMOUNT
		if (empty($amount)) {
			$this->_setError(translate('datacash_amount_not_specified'));
		} else {
			$processor->setAmount($amount, $this->attr('currency'));
		}
		
		//SET MERCHANT REFERENCE
		//merchant reference number should be generated here instead of inside default_delivery_method.inc or form_ecommerce.inc
		//because the customer can try to pay many times until his card is accepted. So each time a payment transaction
		//is sent to Datacash, a new merchant reference number should be used because using the old reference number can
		//cause the "duplicate reference" error. We can see that Datacash reference number is different in every response
		$processor->setMerchantReference(md5(session_id().time()));
		//get the merchant reference number back, it was changed because Datacash requires that merchant reference number length must be between 6 and 30 alphanumeric characters
		$_SESSION['SQ_ECOM_REF_NO'] = $processor->getMerchantReference();
		
		//SET CARD NUMBER
		if (empty($card_info['number'])) {
			$this->_setError(translate('datacash_card_number_not_specified'));
		} else {
			$card_info['number'] = str_replace(Array(' ', "\t"), '', $card_info['number']);
			$processor->setCardNumber($card_info['number']);
		}
		
		//SET CARD EXPIRY DATE
		if ($this->_isValidDate(array_get_index($card_info, 'expiry_month', ''), array_get_index($card_info, 'expiry_year', ''))) {
			$processor->setCardExpiryDate($card_info['expiry_month'], $card_info['expiry_year']);
		} else {
			$this->_setError(translate('datacash_expiry_date_invalid'));
		}
		
		//SET CARD START DATE IF SPECIFIED BECAUSE START DATE IS ONLY REQUIRED FOR SOME CARD TYPES, E.G. SOLO
		if (!empty($card_info['start_month']) || !empty($card_info['start_year'])) {
			if ($this->_isValidDate(array_get_index($card_info, 'start_month', ''), array_get_index($card_info, 'start_year', ''))) {
				$processor->setCardStartDate($card_info['start_month'], $card_info['start_year']);
			} else {
				$this->_setError(translate('datacash_start_date_invalid'));
			}
		}
		
		//SET CARD ISSUE NUMBER, IF SPECIFIED
		if (!empty($card_info['issue_number'])) {
			if ($this->_isValidIssueNumber($card_info['issue_number'])) {
				$processor->setCardIssueNumber($card_info['issue_number']);
			} else {
				$this->_setError(translate('datacash_issue_number_invalid'));
			}
		}
		
		//SET TRANSACTION TYPE AS "AUTH" (ONE STAGE PROCESSING)
		$processor->setTransactionTypeAuth();
		
		//SET 3-D SECURE PARAMETERS
		$nonthreed_card_types = $this->attr('nonthreed_card_types');
		$processor->setThreeDSecureParams($this->attr('merchant_url'), $this->attr('purchase_desc'), time(), empty($nonthreed_card_types)? Array() : explode('|', $nonthreed_card_types));

		//CHECK IF THERE IS ERROR BEFORE SENDING DATA TO DATACASH SERVER, RETURN FALSE
		if ($this->isError()) {
			return FALSE;
		}
		
		$success = $processor->process();

		//GET RESPONSE
		$response = $processor->getResponse();
		
		//if success (1 or 2), save the last 4 digits of card number to output later
		if ($success) {
			$this->_formatted_card_no = $this->_getFormattedCardNumber($card_info['number']);
		}
		
		//if $success = 2, redirect user for card holder verification
		if ($success === 2) {
			$this->_redirect_info = $response;
		} else {
			//success = 1 or FALSE
			$this->_setTransactionResult($success, $response);
		}

		return $success;

	}//end processPayment()
	
	
	/**
	 * Authorize the payment when the user is redirected from the Access Control Server (ACS) after completing the Card Holder Verification process
	 * 
	 * @param string $datacash_ref		the Datacash reference number
	 * @param string $pares				the Payment Authentication Response (PARes) returned by the Access Control Server (ACS)
	 * 
	 * @return mixed		return 1 if SUCCESS; otherwise, return FALSE
	 * @access public
	 */
	function authorizePayment($datacash_ref, $pares)
	{
		$processor = new Datacash_Processor($this->attr('client'), $this->attr('password'), $this->attr('datacash_api_path'), $this->attr('cardinfo_dir_path'), $this->attr('timeout'), $this->attr('test_mode'));
		
		$success = $processor->authorize($datacash_ref, $pares);

		//get response
		$response = $processor->getResponse();
		
		$this->_setTransactionResult($success, $response);
		
		return $success;
		
	}//end authorizePayment()
	
	
	/**
	 * Set the result of the transaction. If transaction fails, set its error message. If transaction success, set the Session variable SQ_ECOM_RESPONSE
	 * 
	 * @param boolean	$success		the reason of the transaction. It is either 1 or FALSE		
	 * @param array		$response		the response array from the Datacash Processor
	 * 
	 * @return void
	 * @access private
	 */
	private function _setTransactionResult($success, $response)
	{
		if ($success === FALSE) {
			$this->_setError($response['STATUS']);
		} else { //success = 1
			if ($this->attr('test_mode')) {
				$response['STATUS'] = '!!!ATTENTION!!! TEST MODE (transaction not performed) -- '.$response['STATUS'];
			}

			$response['CARDNO'] = $this->_formatted_card_no;
			$response['AMOUNT'] = $_SESSION['SQ_ECOM_AMOUNT'];
			
			$_SESSION['SQ_ECOM_RESPONSE']   = $response;
		}
		
	}//end _setTransactionResult()
	

	/**
	 * Get the formatted last 4 digits of the card, e.g. ****-****-****-1234
	 * 
	 * @param string $card_number		the card number to get last 4 digits
	 * 
	 * @return string
	 * @access private
	 */
	private function _getFormattedCardNumber($card_number)
	{
		$card_no = $card_number;
		$card_len = strlen($card_no);
		if ($card_len == 16) {
			$card_no = '****-****-****-'.substr($card_no, -4);
		} else {
			$card_no = str_pad(substr($card_no, -4), $card_len, '*', STR_PAD_LEFT);
		}
		
		return $card_no;
		
	}//end _getFormattedCardNumber()

	
	/**
	* Returns back to e-commerce form, which called gateway
	*
	* @param string	$state	state to return
	*
	* @return void
	* @access public
	*/
	function returnToCaller($state='SUCCESS', $is_in_frame = FALSE)
	{
		$back_url = NULL;

		if ($state == 'SUCCESS') {
			$back_url = array_get_index($_SESSION, 'SQ_ECOM_SUCCESS_URL');
		} else if ($state == 'CANCEL') {
			$back_url = array_get_index($_SESSION, 'SQ_ECOM_CANCEL_URL');
		}

		unset($_SESSION['SQ_ECOM_SUCCESS_URL']);
		unset($_SESSION['SQ_ECOM_CANCEL_URL']);
			
		if (is_null($back_url)) {
			if ($is_in_frame) {
				$this->_redirectParent($this->getURL().'?unknown_referer=1');
			} else {
				trigger_error('Unknown caller reference');
				exit;
			}
		} else {
			if ($is_in_frame) {
				$this->_redirectParent($back_url);
			}else {
				header('Location: '.$back_url);
				exit;
			}
		}
		
	}//end returnToCaller()
	
	
	/**
	 * Redirect the parent of the iframe that this page is loaded in
	 * 
	 * @param string $url	the URL to redirect the parent to using Javascript
	 * 
	 * @return void
	 * @access private
	 */
	private function _redirectParent($url)
	{
		$string = <<<HEREDOC
<script type="text/javascript">
	if (parent != self) {
		parent.location = "{$url}";
	}
</script>
HEREDOC;
				
		echo $string;
		exit;
		
	}//end _redirectParent()


	/**
	* Card Number keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCardNumberKeywordReplacement()
	{
		$prefix = $this->getPrefix();
		$card = $prefix.'_card';

		// set variables for test mode
		if ($this->attr('test_mode')) {
			$test_card_numbers = Datacash_Processor::getTestCardNumbers();
			$card_number = '<select name="'.$card.'[number]">';
			foreach ($test_card_numbers as $number => $label) {
				$card_number .= '<option value="'.$number.'" >'.$label.'</option>';
			}
			$card_number .= '</select>';
		} else {
			$card_number = '<input name="'.$card.'[number]" />';
		}

		return $card_number;

	}//end getCardNumberKeywordReplacement()


	/**
	* Card Start Month keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCardStartMonthKeywordReplacement()
	{
		$card = $this->getPrefix().'_card';

		$string = '<input name="'.$card.'[start_month]" size="2" />';
		
		return $string;

	}//end getCardStartMonthKeywordReplacement()


	/**
	* Card Start Year keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCardStartYearKeywordReplacement()
	{
		$card = $this->getPrefix().'_card';

		$string = '<input name="'.$card.'[start_year]" size="2" />';
		
		return $string;

	}//end getCardStartYearKeywordReplacement()


	/**
	* Card Expiry Month keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCardExpiryMonthKeywordReplacement()
	{
		$card = $this->getPrefix().'_card';

		$string = '<input name="'.$card.'[expiry_month]" size="2" />';
		
		return $string;

	}//end getCardExpiryMonthKeywordReplacement()


	/**
	* Card Expiry Year keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCardExpiryYearKeywordReplacement()
	{
		$card = $this->getPrefix().'_card';

		$string = '<input name="'.$card.'[expiry_year]" size="2" />';
		
		return $string;

	}//end getCardExpiryYearKeywordReplacement()


	/**
	* Card Issue Number keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCardIssueNumberKeywordReplacement()
	{
		$card = $this->getPrefix().'_card';

		$string = '<input name="'.$card.'[issue_number]" size="4" />';
		
		return $string;

	}//end getCardIssueNumberKeywordReplacement()


	/**
	* Transaction Amount keyword replacement
	*
	* @return string
	* @access public
	*/
	function getTransactionAmountKeywordReplacement()
	{
		$amount = array_get_index($_SESSION,'SQ_ECOM_AMOUNT', 0);

		return $amount;

	}//end getTransactionAmountKeywordReplacement()


	/**
	* Processing Error keyword replacement
	*
	* @return string
	* @access public
	*/
	function getProcessingErrorKeywordReplacement()
	{
		if ($this->isError()) {
			return '<span class="payment-gateway-transaction-error">'.$this->getErrorMessage().'</span>';
		}

		return '';

	}//end getProcessingErrorKeywordReplacement()


	/**
	* Submit Button keyword replacement
	*
	* @return string
	* @access public
	*/
	function getSubmitButtonKeywordReplacement()
	{
		$button_text = $this->attr('submit_text');
		if (empty($button_text)) $button_text = 'Submit';
		
		return '<input type="submit" value="'.$button_text.'" />';

	}//end getSubmitButtonKeywordReplacement()


	/**
	* Reset Button keyword replacement
	*
	* @return string
	* @access public
	*/
	function getResetButtonKeywordReplacement()
	{
		$button_text = $this->attr('reset_text');
		if (empty($button_text)) $button_text = 'Reset';
		
		return '<input type="reset" value="'.$button_text.'" />';

	}//end getResetButtonKeywordReplacement()


	/**
	* Cancel Button keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCancelButtonKeywordReplacement()
	{
		$button_text = $this->attr('cancel_text');
		if (empty($button_text)) $button_text = 'Cancel';
		
		return '<input type="submit" name="'.$this->getPrefix().'_action" value="'.$button_text.'" />';

	}//end getCancelButtonKeywordReplacement()
	

	/**
	* Card Holder Verification Form keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCardHolderVerificationFormKeywordReplacement()
	{
		$md = md5($_SESSION['SQ_ECOM_REF_NO']);
		
		$string = <<<HEREDOC
<form name="PARequestForm" action="{$this->_redirect_info['ACS_URL']}" method="POST">
	<input type="hidden" name="PaReq" value="{$this->_redirect_info['PAREQ_MESSAGE']}">
	<input type="hidden" name="TermUrl" value="{$this->getURL()}">
	<input type="hidden" name="MD" value="{$md}">
</form>
<iframe name="ACSframe" width="390" height="400" frameborder="0">
	<b>Your browser does not support iframes</b>
</iframe>
<script type="text/javascript">
	window.onload = function()
	{
		document.PARequestForm.target = "ACSframe";
		document.PARequestForm.submit();
	}
</script>
HEREDOC;
		
		return $string;

	}//end getCardHolderVerificationFormKeywordReplacement()
	
	
	/**
	 * Set error for the transaction so that the getProcessingErrorKeywordReplacement() method will return the error message
	 * 
	 * @param string $err_message	the error message to set
	 * @return void
	 * @access private
	 */
	private function _setError($err_message)
	{
		$this->_tmp['is_error'] = TRUE;
		$this->_tmp['error_message'][] = $err_message;
		
	}//end _setError()
	
	
	/**
	 * Check if the input date is valid
	 * 
	 * @param string $month		the month of the input date
	 * @param string $year		the year of the input date
	 * @return boolean			return TRUE if the date is valid; otherwise, return FALSE
	 * @access private
	 */
	private function _isValidDate($month, $year)
	{
		$valid = FALSE;
		
		$two_digits_pattern = '/^\d{2}$/';
		//month and year must have 2 digit pattern mm/yy
		if (preg_match($two_digits_pattern, $month) && preg_match($two_digits_pattern, $year)) {
			//month must be in 1 and 12
			if ((0 < $month) && ($month < 13)) {
				$valid = TRUE;
			}
		}
		
		return $valid;
		
	}//end _isValidDate()
	
	
	/**
	 * Check if the issue number is valid
	 * 
	 * @param string $issue_no		the input issue number to test
	 * 
	 * @return boolean				return TRUE if the date is valid; otherwise, return FALSE
	 * @access private
	 */
	private function _isValidIssueNumber($issue_no)
	{
		$valid = FALSE;
		
		$pattern = '/^\d{1,2}$/';
		//issue number must be one or two digits long
		if (preg_match($pattern, $issue_no)) {
			$valid = TRUE;
		}
		
		return $valid;
		
	}//end _isValidIssueNumber()
	
	
	/**
	 * Check if the transaction has error
	 * 
	 * @return boolean
	 * @access public
	 */
	public function isError()
	{
		return array_get_index($this->_tmp, 'is_error', FALSE);
		
	}//end isError()
	
	
	/**
	 * Get the error message under HTML format
	 * 
	 * @return string		the HTML error message
	 * @access public
	 */
	public function getErrorMessage()
	{
		$err_message = '';
		$errors = array_get_index($this->_tmp, 'error_message', Array());
		foreach ($errors as $error) {
			$err_message .= "<li>$error </li>";
		}
		
		return '<ul>'.$err_message.'</ul>';
		
	}//end getErrorMessage()
	

}//end class

?>