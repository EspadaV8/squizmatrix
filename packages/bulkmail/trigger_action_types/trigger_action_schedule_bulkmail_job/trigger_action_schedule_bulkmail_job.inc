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
* $Id: trigger_action_schedule_bulkmail_job.inc,v 1.2 2012/08/30 00:57:14 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';

/**
* Trigger_Action_Schedule_Bulkmail_Job
*
*
* @author  Ash Karelia <akarelia@squiz.com.au>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage bulkmail
*/
class Trigger_Action_Schedule_Bulkmail_Job extends Trigger_Action
{


	/**
	* Execute this action
	*
	* Returns true on success, or false on error
	* The settings used by this action are in the form:<BR />
	* <PRE>
	* Array(
	*	'assetid' => int    // id of the root node
	*	'emails' => string  // list of users emails
	* );
	* </PRE>
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return boolean
	* @access public
	*/
	public static function execute($settings, &$state)
	{
		$bulkmail_job_id = array_get_index($settings, 'bulkmail_job_id', 0);

		if ($bulkmail_job_id == 0) return FALSE;

		$asset_to_send   = array_get_index($settings, 'asset_to_send', 0);
		$current_asset   = array_get_index($settings, 'current_asset', FALSE);


		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		if (is_null($state['asset'])) return FALSE;

		if ($current_asset) {
			$asset_to_send = $state['asset']->id;
		}

		$am = $GLOBALS['SQ_SYSTEM']->am;
		$bm = $am->getSystemAsset('bulkmail_manager');
		$job_asset = $am->getasset($bulkmail_job_id);

		// if we don't ahve the asset to send yet then
		// pick up the asset that is configured int he job itself
		if ($asset_to_send == 0) {
			$asset_to_send = $job_asset->attr('content_id');
		}

		$subscriptions   = array_get_index($settings, 'subscriptions', Array());
		$sub_status      = array_get_index($settings, 'subscriptions_status', FALSE);
		$recipient       = array_get_index($settings, 'recipient', Array());


		$post_office = $job_asset->getPostOffice();

		// have all the informatin we want?
		// add a ad-hoc bulkmail job
		$is_a_bm        = get_class($post_office) == 'Bulkmail_Manager';
		$from           = $job_asset->attr('from');
		$recipient      = empty($recipient) ? $job_asset->attr('recipients') : $recipient;
		$gen_as         = $job_asset->attr('content_gen_as');
		$po_headers     = $job_asset->attr('use_post_office_header') ? TRUE : FALSE;
		$headers        = $po_headers ? $post_office->attr('header_details') : $job_asset->attr('header_details');
		$subject        = $po_headers ? $post_office->attr('subject') : $job_asset->attr('subject');
		$server_headers = $is_a_bm ? Array() : $post_office->attr('server_details');
		$design         = $job_asset->attr('content_design') ? $job_asset->attr('content_design') : 0;
		$layout         = $job_asset->attr('content_layout') ? $job_asset->attr('content_layout') : 0;

		// if the subscription is turned on but no categories are added then borrow from the job
		if ($sub_status && empty($subscriptions)) {
			$subscriptions = $job_asset->attr('subscriptions');
		}

		if (!$bm->createAdHocJob($from, $recipient, $subject, $asset_to_send, $gen_as, $headers, 0, $server_headers, $design, $layout, $subscriptions))
			return FALSE;


		return Array(
				'job_id' => $job_asset->id,
			   );

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
		$bulkmail_job_id = array_get_index($settings, 'bulkmail_job_id', 0);
		$subscriptions   = array_get_index($settings, 'subscriptions', Array());
		$sub_status      = array_get_index($settings, 'subscriptions_status', FALSE);
		$asset_to_send   = array_get_index($settings, 'asset_to_send', 0);
		$current_asset   = array_get_index($settings, 'current_asset', FALSE);
		$recipient       = array_get_index($settings, 'recipient', Array());

		$option_type = Array(
						'matches'		=> 'Matches',
						'contains'		=> 'Contains',
					   );

		$new_rule_type = Array(
							'0'			=> '-- Add New Rule --',
							'metadata'	=> 'Asset Metadata field',
						 );


		ob_start();

		?><table>
			<tr>
				<td>
				<?php
					echo translate('asset_to_send');
				?>
				</td>
				<td>
				<?php
					if ($write_access) {
						asset_finder($prefix.'[asset_to_send]', $asset_to_send);
						check_box($prefix.'[current_asset]', '1', $current_asset, '', 'id="'.$prefix.'_current_asset"');
						label(translate('use_current_asset'), $prefix.'_current_asset');
					} else {
						if ($asset_to_send) {
							echo get_asset_tag_line($asset_to_send);
							echo "<br />";
						}
						echo translate('use_current_asset');
						echo '&nbsp;&nbsp;<img src="'.sq_web_path('lib').'/web/images/'.($current_asset ? 'tick' : 'cross').'.gif" alt="'.($current_asset ? translate('yes') : translate('no')).'" /> ';

					}
				?>
				</td>
			</tr>
			<tr>
				<td>
				<?php
					echo translate('bulkmail_job');
				?>
				</td>
				<td>
				<?php
					if ($write_access) {
						asset_finder($prefix.'[bulkmail_job_id]', $bulkmail_job_id, Array('bulkmail_job' => 'I'));
					} else {
						if ($bulkmail_job_id) {
							echo get_asset_tag_line($bulkmail_job_id);
						} else {
							echo translate('not_configured');
						}
					}
				?>
				</td>
			</tr>
			<tr>
				<td valign="top">
				<?php
					echo translate('recipients');
				?>
				</td>
				<td>
				<?php
					if ($write_access) {
						multiple_asset_finder($prefix.'[recipient]', $recipient, Array('user' => 'D', 'user_group' => 'D'));
					} else {
						if ($recipient) {
							foreach ($recipient as $res) {
								echo get_asset_tag_line($res);
								echo "<br />";
							}
						} else {
							echo translate('not_configured');
						}
					}
				?>
				</td>
			</tr>
			<tr>
				<td>
				<?php
					echo translate('enable_subscription_filter');
				?>
				</td>
				<td>
				<?php
					if ($write_access) {
						check_box($prefix.'[subscriptions_status]', '1', $sub_status);
					} else {
						echo '&nbsp;&nbsp;<img src="'.sq_web_path('lib').'/web/images/'.($sub_status ? 'tick' : 'cross').'.gif" alt="'.($sub_status ? translate('yes') : translate('no')).'" /> ';
					}
				?>
				</td>
			</tr>
			<?php
				if ($sub_status) {
			?>
			<tr>
				<td valign="top">
				<?php
					echo translate('filters');
				?>
				</td>
				<td>
				<?php
				echo '<table class ="sq-backend-table" border="1" style="margin-bottom: 7px">';
				echo '<tr><td rowspan =2 width="12%">Name</td><td colspan="3">Rules</td></tr>';
				echo '<tr><td width="15%">Type</td><td>Details</td><td>Delete?</td></tr>';

				foreach ($subscriptions as $name => $subscription) {
					hidden_field($prefix.'[subscriptions]['.$name.']', '');
					$tr = FALSE;
					echo "<tr>";
					$rowspan = ($write_access) ? (count($subscription) + 1) : count($subscription);
					if (empty($subscription)) $rowspan++;
					echo '<th rowspan='.$rowspan.'>'.$name.'</th>';
					if (!empty($subscription)) {
						foreach ($subscription as $index => $rules) {
							if ($tr) echo "<tr>";
							$type_data = array_keys($rules);
							echo '<td>'.$new_rule_type[$type_data[0]].'</td>';
							echo '<td>';
							if ($write_access) {
								asset_finder($prefix.'[subscriptions]['.$name.']['.$index.']['.$type_data[0].'][assetid]', $rules[$type_data[0]]['assetid'], Array('metadata_field' => 'D'));
							} else {
								echo get_asset_tag_line($rules[$type_data[0]]['assetid']);
							}
							echo '&nbsp;&nbsp;&nbsp;';
							if ($write_access) {
								combo_box($prefix.'[subscriptions]['.$name.']['.$index.']['.$type_data[0].'][rule_cond]', $option_type, FALSE, $rules[$type_data[0]]['rule_cond']);
							} else {
								echo $rules[$type_data[0]]['rule_cond'];
							}
							echo '&nbsp;&nbsp;&nbsp;';
							if ($write_access) {
								text_box($prefix.'[subscriptions]['.$name.']['.$index.']['.$type_data[0].'][rule_text]', $rules[$type_data[0]]['rule_text'], 30);
							} else {
								echo $rules[$type_data[0]]['rule_text'];
							}

							echo '</td><td>';
							if ($write_access) check_box($prefix.'[subscriptions]['.$name.']['.$index.'][delete]');
							echo "</td></tr>";

							$tr = TRUE;
						}//end foreach
					} else {
						echo '<td colspan=3></td></tr>';
					}
					if ($write_access) {
						?>
							<tr>
								<td colspan=4>
									<?php
										combo_box($prefix.'[subscriptions]['.$name.'][new_rule]', $new_rule_type, FALSE);
									?>
								</td>
							</tr>
						<?php
					}
				}//end foreach
				echo "</table>";
				if ($write_access) {
					label(translate('new_category'), $prefix.'[new_category_name]');
					echo '&nbsp;';
					text_box($prefix.'[new_category_name]', '', 15);
				}

				?>
				</td>
			</tr>
			<?php
				}
			?>

		</table>
		<?php

		$html = ob_get_clean();

		return $html;

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
		$settings['bulkmail_job_id']      = array_get_index($request_data['bulkmail_job_id'], 'assetid', 0);
		$settings['subscriptions_status'] = array_get_index($request_data, 'subscriptions_status', FALSE);
		$settings['asset_to_send']        = array_get_index($request_data['asset_to_send'], 'assetid', 0);
		$settings['current_asset']        = array_get_index($request_data, 'current_asset', FALSE);


		$raw_userids = $request_data['recipient'];
		$userids     = Array();
		foreach ($raw_userids as $user) {
			if (!empty($user['assetid'])) {
				$userids[] = $user['assetid'];
			}
		}
		$settings['recipient'] = $userids;

		if ($settings['current_asset']) $settings['asset_to_send'] = 0;

		if ($settings['subscriptions_status']) {
			$new_subscriptions = Array();
			$subscriptions     = array_get_index($request_data, 'subscriptions', Array());
			$new_category_name = array_get_index($request_data, 'new_category_name', '');
			if (!empty($subscriptions)) {
					foreach ($subscriptions as $sub_name => $subscription) {
						foreach ($subscription as $index => $subscription_data) {
							if ($index === 'new_rule') {
								if ($subscription['new_rule'] !== '0')  {
									$new_subscriptions[$sub_name][][$subscription['new_rule']] = Array(
																									'assetid'	=> 0,
																									'rule_cond'	=> 'contains',
																									'rule_text'	=> '',
																								);
								}
								continue;
							}

							if (!isset($subscription_data['delete'])) {
								$category = array_keys($subscription_data);
								$new_subscriptions[$sub_name][][$category[0]] = Array(
																					'assetid'	=> $subscription_data[$category[0]]['assetid']['assetid'],
																					'rule_cond'	=> array_get_index($subscription_data[$category[0]], 'rule_cond', 'contains'),
																					'rule_text'	=> array_get_index($subscription_data[$category[0]], 'rule_text',''),
																				);
							}
						}
					}
				}

				if (!empty($new_category_name)) {
					$new_subscriptions[$new_category_name] = Array();
				}
				$settings['subscriptions'] = $new_subscriptions;
		}

		return FALSE;

	}//end processInterface()


}//end class

?>