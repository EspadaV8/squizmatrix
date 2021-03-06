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
* $Id: condition_user_type_edit_fns.inc,v 1.6 2012/08/30 01:09:10 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/conditions/condition/condition_edit_fns.inc';

/**
* Condition_User_Type_Edit_Fns
*
* Edit functions for the user type condition
*
* @see Asset_Edit_Fns
*
* @author  Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition_User_Type_Edit_Fns extends Condition_Edit_Fns
{


	/**
	* Constructor
	*
	* @see Asset_Edit_Fns::__construct()
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* Paint the backend editing interface for this condition
	*
	* @param array				&$condition_data	Array of settings for this condition
	* @param Backend_Outputter	&$o					Backend outputter to use
	* @param string				$prefix				Prefix for form elements
	* @param boolean			$write_access		Whether we have access to edit the field values
	*
	* @return void
	* @access public
	*/
	public static function paintEditInterface(Array $condition_data, Backend_Outputter $o, $prefix, $write_access)
	{
		$user_types = array_get_index($condition_data, 'user_types', '');

		$type_list = Array();
		if (!empty($user_types)) {
			$user_types = str_replace(' ', '', $user_types);
			$type_list = explode(',', $user_types);
		}
		?>

		<table class="sq-backend-table">
			<tr>
				<th class="sq-conditions-options-th"><?php echo translate('User Types') ?></th>

				<td>
				<?php
				if ($write_access) {
					asset_type_chooser($prefix.'[type_list]', TRUE, $type_list, NULL, FALSE, FALSE, Array('user'));
				} elseif (!empty($type_list)) {
					$type_names = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_list, 'name');
					echo '<ul class="sq-asset-id-list">';
					foreach ($type_list as $type) {
						echo '<li>';
							echo get_asset_type_icon($type);
							echo '<strong>'.$type_names[$type].'</strong>';
						echo '</li>';
					}
					echo '</ul>';
				} else {
					echo '<em>'.translate('No user types selected.').'</em>';

				}
				?>
				</td>
			</tr>
		</table>

		<?php

	}//end paintEditInterface()


	/**
	* Process the backend editing interface for this condition
	*
	* @param Backend_Outputter	$o		Backend outputter to use
	* @param string				$prefix	Prefix for form elements
	*
	* @return array
	* @access public
	*/
	public static function processEditInterface(Backend_Outputter $o, $prefix)
	{
		if (!isset($_POST[$prefix]['type_list']) || empty($_POST[$prefix]['type_list'])) {
			return Array();
		}

		// construct and return data required by this condition
		// as found in the post data, array required consists of
		// 'match' and 'condition_data'
		$results = Array(
					'user_types'	=> trim(implode(', ', $_POST[$prefix]['type_list']),', '),
				   );

		return $results;

	}//end processEditInterface()


}//end class

?>