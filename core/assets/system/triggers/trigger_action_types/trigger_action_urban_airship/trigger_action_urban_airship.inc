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
* $Id: trigger_action_urban_airship.inc,v 1.1 2013/04/05 05:06:40 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';
require_once SQ_SYSTEM_ROOT.'/core/attributes/parameter_map/parameter_map.inc';
require_once SQ_SYSTEM_ROOT.'/core/attributes/option_list/option_list.inc';
require_once dirname(__FILE__).'/lib/urbanairship.php';

/**
* Trigger_Action_Urban_Airship
*
* A trigger action that sends a redirect header with a user-configurable URL
*
* @author  Nic Hubbard <nic@zedsaid.com>
* @version $Revision: 1.1.1 $
* @package MySource_Matrix_Packages
* @subpackage zed_said
*/
class Trigger_Action_Urban_Airship extends Trigger_Action
{


	/**
	* Execute this action
	*
	* Returns an array of data about what it did, or false on error
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return mixed array|boolean
	* @access public
	*/
	public static function execute($settings, &$state)
	{

		// Static values
		$static_app_key = $settings['airship_app_key'];
		$static_app_token = $settings['airship_device_token'];
		$static_app_master_secret = $settings['airship_app_master_secret'];
		$static_push_message = $settings['airship_message'];
		$static_push_sound = $settings['airship_sound'];
		$static_push_badge = $settings['airship_badge'];
		$static_push_key1 = $settings['airship_key1'];
		$static_push_key2 = $settings['airship_key2'];
		$static_push_key3 = $settings['airship_key3'];
		$static_push_value1 = $settings['airship_value1'];
		$static_push_value2 = $settings['airship_value2'];
		$static_push_value3 = $settings['airship_value3'];

		// check required settings
		if (!isset($static_app_key) || empty($static_app_key)) {
			return FALSE;
		}
		if (!isset($static_app_master_secret) || empty($static_app_master_secret)) {
			return FALSE;
		}
		if (!isset($static_push_message) || empty($static_push_message)) {
			return FALSE;
		}

		// Make sure we either have a device token or we are broadcasting
		$should_broadcast = array_get_index($settings, 'airship_broadcast', 0) ? TRUE : FALSE;
		if ((!isset($static_app_token) || empty($static_app_token)) && !$should_broadcast) {
			return FALSE;
		}

		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}
		if (is_null($state['asset'])) return FALSE;

		// Param Map
		$parameter_map_value = array_get_index($settings, 'value_map', serialize(Array()));
		$atr_parameter_map = new Asset_Attribute_Parameter_Map(0, $parameter_map_value);
		$param_map_token = $atr_parameter_map->getParameterValue('value_map_token');
		$param_map_badge = $atr_parameter_map->getParameterValue('value_map_badge');
		$param_map_sound = $atr_parameter_map->getParameterValue('value_map_sound');
		$param_map_message = $atr_parameter_map->getParameterValue('value_map_message');

		// Dynamic Replacements
		if (!empty($param_map_token)) {
			$static_app_token = $param_map_token;
		}
		if (!empty($param_map_badge)) {
			$static_push_badge = $param_map_badge;
		}
		if (!empty($param_map_sound)) {
			$static_push_sound = $param_map_sound;
		}
		if (!empty($param_map_message)) {
			$static_push_message = $param_map_message;
		}

		// replace keywords
		$static_push_badge_replaced = $state['asset']->replaceKeywordsInString($static_push_badge);
		$static_push_sound_replaced = $state['asset']->replaceKeywordsInString($static_push_sound);
		$static_push_message_replaced = $state['asset']->replaceKeywordsInString($static_push_message);

		// Only values should be replaced, not keys
		$static_push_value1_replaced = $state['asset']->replaceKeywordsInString($static_push_value1);
		$static_push_value2_replaced = $state['asset']->replaceKeywordsInString($static_push_value2);
		$static_push_value3_replaced = $state['asset']->replaceKeywordsInString($static_push_value3);


		if (!empty($static_push_message_replaced)) {

			// Create Airship object
			$airship = new Airship($static_app_key, $static_app_master_secret);

			// Message
			$message_data = array('alert'=> $static_push_message_replaced);
			if (!empty($static_push_sound_replaced)) {
				$message_data['sound'] = $static_push_sound_replaced;
			}
			if (!empty($static_push_badge_replaced)) {
				$message_data['badge'] = intval($static_push_badge_replaced);
			}
			$message = array('aps' => $message_data);

			// Custom payload
			if (!empty($static_push_key1) && !empty($static_push_value1_replaced)) {
				$message[$static_push_key1] = $static_push_value1_replaced;
			}
			if (!empty($static_push_key2) && !empty($static_push_value2_replaced)) {
				$message[$static_push_key2] = $static_push_value2_replaced;
			}
			if (!empty($static_push_key3) && !empty($static_push_value3_replaced)) {
				$message[$static_push_key3] = $static_push_value3_replaced;
			}

			// If we should push or broadcast
			if ($should_broadcast) {
				$airship->broadcast($message);
			} else {
				// Check to see if we have multiple device tokens
				if (strpos($static_app_token, ',')) {
					$static_app_token = explode(',', $static_app_token);
				}
				error_log(print_r($message, TRUE), 0);
				$airship->push($message, $static_app_token);
			}//end

			return Array(
					'assetid'	=> $state['asset']->id,
				   );
		} else {
			return FALSE;
		}

	}//end execute()


	/**
	* Get the HTML editing interface for this action. Returns the string instead of printing it
	*
	* @param array		$settings		settings that this condition saves in processInterface()
	* @param string		$prefix			unique prefix
	* @param boolean	$write_access	determines whether the interface is editable
	*
	* @return boolean
	* @access public
	*/
	public static function getInterface($settings, $prefix, $write_access=FALSE)
	{
		ob_start();
		?>

		<p class="sq-backend-section-subheading"><?php echo translate('Urbain Airship API Details').' '; ?></p>


			<table class="sq-backend-table" >
				<tr>
					<td width="100"><p><?php echo translate('App Key').' '; ?></p></td>

					<td><p>
					<?php
						$app_key = array_get_index($settings, 'airship_app_key', '');
						if ($write_access) {
							text_box($prefix.'[airship_app_key]', $app_key, 45);
						} else {
							echo str_repeat('&#9679;', strlen($app_key));
						}
					?></p></td>
				</tr>
				<tr>
					<td width="100"><p><?php echo translate('Master Secret').' '; ?></p></td>

					<td><p>
					<?php
						$app_master_secret = array_get_index($settings, 'airship_app_master_secret', '');
						if ($write_access) {
							text_box($prefix.'[airship_app_master_secret]', $app_master_secret, 45);
						} else {
							echo str_repeat('&#9679;', strlen($app_master_secret));
						}
					?></p></td>
				</tr>
				<tr>
					<td width="100"><p><?php echo translate('Device Tokens').' '; ?></p></td>

					<td>
					<?php
						$device_token = array_get_index($settings, 'airship_device_token', '');
						if ($write_access) {
							echo '<p>';
							$option_list = new Asset_Attribute_Option_List(0, $device_token, ',');
							$option_list->setEditParam('width', 85);
							$option_list->paint('airship_device_token');
							echo '</p>';
							echo '<p>'.translate('The token of the devices as generated by the Urban Airship API.').'</p>';

						} else {
							echo '<p>'.str_replace(',', '<br />', $device_token).'</p>';
						}
					?></td>
				<tr>
					<td width="100"><p><?php echo translate('Broadcast').' '; ?></p></td>

					<td>
					<?php
						$airship_broadcast = array_get_index($settings, 'airship_broadcast', 0);
						if ($write_access) {
							echo '<p>';
							check_box($prefix.'[airship_broadcast]', 1, $airship_broadcast);
							echo '</p>';
							echo '<p class="sq-backend-warning">'.translate('This option will broadcast the push notification to ALL users associated with the App Key. If this option is used device token will be ignored.').'</p>';

						} else {
							$tick_type = ($airship_broadcast == '1') ? 'tick' : 'cross';
							echo '<img class="sq-icon" width="16" height="16" src="'.sq_web_path('lib').'/web/images/'.$tick_type.'.gif" alt="" style="border: 0;" />';
						}
					?></td>
				</tr>

			</table><br />

			<p class="sq-backend-section-subheading"><?php echo translate('Push Notification Details').' '; ?></p>

			<table class="sq-backend-table" >
				<tr>
					<td width="100"><p><?php echo translate('Badge').' '; ?></p></td>

					<td><p>
					<?php
						$airship_badge = array_get_index($settings, 'airship_badge', '');
						if ($write_access) {
							echo '<p>';
							text_box($prefix.'[airship_badge]', $airship_badge,  5);
							echo '</p>';
							echo '<p>'.translate('A number to set as the app icon badge number. Keywords are supported.').'</p>';

						} else {
							echo '<p>'.$airship_badge.'</p>';
						}
					?></p></td>
				</tr>
				<tr>
					<td width="100"><p><?php echo translate('Sound').' '; ?></p></td>

					<td><p>
					<?php
						$airship_sound = array_get_index($settings, 'airship_sound', '');
						if ($write_access) {
							echo '<p>';
							text_box($prefix.'[airship_sound]', $airship_sound, 25);
							echo '</p>';
							echo '<p>'.translate('The name of the sound to be played when the notification is recieved. Use the word <b>default</b> for the default message sound. <br />You can specify a custom sound that iOS plays when it presents the remote notification for an application. The sound files must be in the main bundle of the client application. Keywords are supported.').'</p>';

						} else {
							echo '<p>'.$airship_sound.'</p>';
						}
					?></p></td>
				</tr>
				<tr>
					<td width="100"><p><?php echo translate('Alert').' '; ?></p></td>

					<td><p>
					<?php
						$airship_message = array_get_index($settings, 'airship_message', '');
						if ($write_access) {
							echo '<p>';
							text_box($prefix.'[airship_message]', $airship_message, 95);
							echo '</p>';
							echo '<p>'.translate('The maximum size allowed for a notification payload is 256 bytes. Message will be automatically trimmed if too long. Keywords are supported.').'</p>';

						} else {
							echo '<p>'.$airship_message.'</p>';
						}
					?></p></td>
				<tr>
					<td width="100"><p><?php echo translate('Custom parameters'); ?></p></td>

					<td><p>
					<?php
						$airship_key1 = array_get_index($settings, 'airship_key1', '');
						$airship_key2 = array_get_index($settings, 'airship_key2', '');
						$airship_key3 = array_get_index($settings, 'airship_key3', '');
						$airship_value1 = array_get_index($settings, 'airship_value1', '');
						$airship_value2 = array_get_index($settings, 'airship_value2', '');
						$airship_value3 = array_get_index($settings, 'airship_value3', '');
						if ($write_access) {
							echo '<p>';
							text_box($prefix.'[airship_key1]', $airship_key1, 15);
							echo '&nbsp; &nbsp;';
							text_box($prefix.'[airship_value1]', $airship_value1, 65);
							echo '</p>';
							echo '<p>';
							text_box($prefix.'[airship_key2]', $airship_key2, 15);
							echo '&nbsp;&nbsp;';
							text_box($prefix.'[airship_value2]', $airship_value2, 65);
							echo '</p>';
							echo '<p>';
							text_box($prefix.'[airship_key3]', $airship_key3, 15);
							echo '&nbsp;&nbsp;';
							text_box($prefix.'[airship_value3]', $airship_value3, 65);
							echo '</p>';
							echo '<p>'.translate('Additional key value pairs that will be sent as part of the payload. Keywords are supported for values.').'</p>';

						} else {
							echo '<p><ul>';
							echo '<li>Key: '.$airship_key1.' Value: '.$airship_value1.'</li>';
							echo '<li>Key: '.$airship_key2.' Value: '.$airship_value2.'</li>';
							echo '<li>Key: '.$airship_key3.' Value: '.$airship_value3.'</li>';
							echo '</ul></p>';
						}
					?></p></td>
				</tr>
			</table>

         <?php

		// Print the parameter map
		if ($write_access) {

			echo '<br /><div>';
			$new_prefix = str_replace(array('[',']'), '_', $prefix);
			hidden_field($prefix.'[new_prefix]', $new_prefix);

			$parameter_map_value = array_get_index($settings, 'value_map', serialize(Array()));
			$atr_parameter_map = new Asset_Attribute_Parameter_Map(0, $parameter_map_value);

			// Param Map
			$atr_parameter_map->setParameter('value_map_token', 'Device Token');
			$atr_parameter_map->setParameter('value_map_badge', 'Badge');
			$atr_parameter_map->setParameter('value_map_sound', 'Sound');
			$atr_parameter_map->setParameter('value_map_message', 'Message');
			echo $atr_parameter_map->paint($new_prefix.'_parameter_map');
			echo '</div>';
		}//end

		return ob_get_clean();

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	*
	* @param array	&$settings		a container for any data the action might want to save
	* @param string	$request_data	array of data corresponding to this action as
	*								specified by the $prefix in getInterface.
	*
	* @return boolean
	* @access public
	*/
	public static function processInterface(&$settings, $request_data)
	{
		$settings['airship_app_key'] = trim(array_get_index($request_data, 'airship_app_key', ''));
		$settings['airship_app_master_secret'] = trim(array_get_index($request_data, 'airship_app_master_secret', ''));
		$settings['airship_message'] = trim(array_get_index($request_data, 'airship_message', ''));
		$settings['airship_sound'] = trim(array_get_index($request_data, 'airship_sound', ''));
		$settings['airship_broadcast'] = trim(array_get_index($request_data, 'airship_broadcast', 0));
		$settings['airship_badge'] = trim(array_get_index($request_data, 'airship_badge', 0));
		$settings['airship_key1'] = trim(array_get_index($request_data, 'airship_key1', 0));
		$settings['airship_key2'] = trim(array_get_index($request_data, 'airship_key2', 0));
		$settings['airship_key3'] = trim(array_get_index($request_data, 'airship_key3', 0));
		$settings['airship_value1'] = trim(array_get_index($request_data, 'airship_value1', 0));
		$settings['airship_value2'] = trim(array_get_index($request_data, 'airship_value2', 0));
		$settings['airship_value3'] = trim(array_get_index($request_data, 'airship_value3', 0));

		$settings['value_map'] = '';
		$new_prefix = str_replace(array('[',']'), '_', array_get_index($request_data, 'new_prefix', ''));
		if ($new_prefix) {
			$atr_parameter_map = new Asset_Attribute_Parameter_Map();
			$atr_parameter_map->process($new_prefix.'_parameter_map');
			$settings['value_map'] = $atr_parameter_map->value;
		}

		$option_list = new Asset_Attribute_Option_List(0, NULL, ',');
		$option_list->process('airship_device_token');
		$settings['airship_device_token'] = $option_list->value;

		return FALSE;

	}//end processInterface()


	/**
	* Get the list of locks that the action needs acquire before executing
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return array
	* @access public
	*/
	public static function getLocks($settings, &$state)
	{
		return Array();

	}//end getLocks()


}//end class

?>
