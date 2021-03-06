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
* $Id: content_type_image_prefs.inc,v 1.9 2012/08/30 00:58:07 ewang Exp $
*
*/


require_once SQ_LIB_PATH.'/config/prefs.inc';
require_once SQ_ATTRIBUTES_PATH.'/css_dimension/css_dimension.inc';

/**
* Content_Type_Structure_Tree_Prefs
*
* Purpose
*
*    Looks after the creation the Image Content Type Preferences
*
* @author  Darren McKee <dmckee@squiz.net
* @version $Revision: 1.9 $
* @package MySource_Matrix
*/
class Content_Type_Image_Prefs extends Prefs
{

	var $pref_vars = Array (
						'SQ_CONTENT_TYPE_IMAGE_DEFAULTS'	=> Array(
																'name'			=> 'image_defaults_title',
																'description'	=> 'This preference allows you to change the default values',
																'default'		=> Array(
																					'caption'				=> 'Caption',
																					'caption_position'		=> 'bottom',
																					'use_caption_attr'		=> FALSE,
																					'width'					=> '400',
																					'height'				=> '400',
																					'fit_dimensions'		=> TRUE,
																					'constrain_proportions'	=> TRUE,
																					'on_click'				=> 'none',
																					'html_layout'			=> 'table',
																				   ),
																'protected'		=> FALSE,
															   ),
						'SQ_CONTENT_TYPE_IMAGE_FIELDS'		=> Array(
																'name'			=> 'image_fields_title',
																'description'	=> 'This preference allows you to specify which fields are displayed',
																'default'		=> Array(
																					'caption_options'	=> TRUE,
																					'caption_position'	=> TRUE,
																					'dimension_options'	=> TRUE,
																					'on_click'			=> TRUE,
																					'html_layout'		=> TRUE,
																				   ),
																'protected'		=> FALSE,
															   ),
						'SQ_CONTENT_TYPE_IMAGE_CLASSES'		=> Array(
																'name'			=> 'image_css_classes',
																'description'	=> 'This preference allows you to specify how the image is displayed',
																'default'		=> Array(
																					'surrounding_container'	=> 'image_table',
																					'image_table_cell'		=> 'image_table_cell',
																					'caption_table_cell'	=> 'caption_table_cell',
																					'image_tag'				=> 'inline_image',
																					'image_div'				=> 'image_div',
																					'caption_div'			=> 'caption_div',
																				   ),
																'protected'		=> FALSE,
															   ),
						'SQ_CONTENT_TYPE_IMAGE_PREVIEW'		=> Array(
																'name'			=> 'image_preview_css_classes',
																'description'	=> 'This preference allows you to specify how the image is displayed',
																'default'		=> Array(
																					'surrounding_container'	=> 'preview_container',
																					'preview_image_table'	=> 'preview_image_table',
																					'image_table_cell'		=> 'image_table_cell',
																					'image_tag'				=> 'preview_image',
																					'close_preview_cell'	=> 'close_cell',
																					'image_div'				=> 'preview_image_div',
																					'close_preview_div'		=> 'close_preview_div',
																				   ),
																'protected'		=> FALSE,
															   ),
					 );


	/**
	* Constructor
	*
	*/
	function Content_Type_Image_Prefs($pref_file='')
	{
        $this->Prefs($pref_file);

        $this->pref_vars['SQ_CONTENT_TYPE_IMAGE_DEFAULTS']['name'] = translate('Image Content Type Value Defaults');
        $this->pref_vars['SQ_CONTENT_TYPE_IMAGE_FIELDS']['name']   = translate('Image Fields to Show');
        $this->pref_vars['SQ_CONTENT_TYPE_IMAGE_CLASSES']['name']  = translate('Image CSS Classes');
        $this->pref_vars['SQ_CONTENT_TYPE_IMAGE_PREVIEW']['name']  = translate('Image Preview CSS Classes');

	}//end constructor


	/**
	* Paints the backend interface to edit preferences
	*
	* @param object		&$o			reference to the backend outputter
	* @param boolean	$have_lock	do we have the lock to edit these preferences or not
	* @param string		$pref		a particular preference to paint
	*
	* @return boolean
	* @access public
	*/
	function paintBackend(&$o, $have_lock, $pref=NULL)
	{
	    $is_root   = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin  = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());
        $have_lock = ($have_lock && $is_admin);

		// Images for readonly mode on checkboxes
		$tick = '<img src="'.sq_web_path('lib').'/web/images/tick.gif" alt="Yes" style="position: relative; right: 0px;"/>';
		$cross = '<img src="'.sq_web_path('lib').'/web/images/cross.gif" alt="Yes" />';

		$this->paintField($o, $have_lock, $pref, 'SQ_CONTENT_TYPE_IMAGE_DEFAULTS', function($value_defaults) use ($have_lock, $tick, $cross) {
            $boolean_options = Array(
                0 => translate('No'),
                1 => translate('Yes'),
            );
            ?>
            <table class="sq-backend-table" style="width: 350px;">
                <tr>
                    <td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate('Caption'); ?></td>

                    <td class="sq-backend-table-cell" nowrap="nowrap">
                        <?php
                            if ($have_lock) {
                                text_box('SQ_CONTENT_TYPE_IMAGE_DEFAULTS[caption]', array_get_index($value_defaults, 'caption', ''), '30');
                            } else {
                                echo $value_defaults['caption'];
                            }
                        ?>
                    </td>
                </tr>
                <tr>
                    <td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate('Caption Position'); ?></td>

                    <td class="sq-backend-table-cell" nowrap="nowrap">
                        <?php
                            if ($have_lock) {
                                combo_box('SQ_CONTENT_TYPE_IMAGE_DEFAULTS[caption_position]', Array('bottom' => 'Bottom', 'top' => 'Top'), FALSE, array_get_index($value_defaults, 'caption_position', ''));
                                } else {
                                    echo ucwords($value_defaults['caption_position']);
                                }
                        ?>
                    </td>
                </tr>
                <tr>
                    <td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate('Use Caption Attribute'); ?></td>

                    <td class="sq-backend-table-cell" nowrap="nowrap">
                        <?php
                            if ($have_lock) {
                                combo_box('SQ_CONTENT_TYPE_IMAGE_DEFAULTS[user_caption_attr]', $boolean_options, FALSE, array_get_index($value_defaults, 'user_caption_attr', FALSE));
                            } else {
                                echo $boolean_options[array_get_index($value_defaults, 'user_caption_attr', FALSE)];
                            }
                        ?>
                    </td>
                </tr>
                <tr>
                    <td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate('Width'); ?></td>

                    <td class="sq-backend-table-cell" nowrap="nowrap">
                        <?php
                            if ($have_lock) {
                                text_box('SQ_CONTENT_TYPE_IMAGE_DEFAULTS[width]', array_get_index($value_defaults, 'width', ''), '10');
                            } else {
                                echo $value_defaults['width'];
                            }
                        ?>
                    </td>
                </tr>
                <tr>
                    <td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate('Height'); ?></td>

                    <td class="sq-backend-table-cell" nowrap="nowrap">
                        <?php
                            if ($have_lock) {
                                text_box('SQ_CONTENT_TYPE_IMAGE_DEFAULTS[height]', array_get_index($value_defaults, 'height', ''), '10');
                            } else {
                                echo $value_defaults['height'];
                            }
                        ?>
                    </td>
                </tr>
                <tr>
                    <td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate('Constrain Proportions'); ?></td>

                    <td class="sq-backend-table-cell" nowrap="nowrap">
                        <?php
                            if ($have_lock) {
                                combo_box('SQ_CONTENT_TYPE_IMAGE_DEFAULTS[constrain_proportions]', $boolean_options, FALSE, array_get_index($value_defaults, 'constrain_proportions', FALSE));
                            } else {
                                echo $boolean_options[array_get_index($value_defaults, 'constrain_proportions', FALSE)];
                            }
                        ?>
                    </td>
                </tr>
                <tr>
                    <td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate('Shrink/Stretch to Dimensions'); ?></td>

                    <td class="sq-backend-table-cell" nowrap="nowrap">
                        <?php
                            if ($have_lock) {
                                combo_box('SQ_CONTENT_TYPE_IMAGE_DEFAULTS[fit_dimensions]', $boolean_options, FALSE, array_get_index($value_defaults, 'fit_dimensions', FALSE));
                            } else {
                                echo $boolean_options[array_get_index($value_defaults, 'fit_dimensions', FALSE)];
                            }
                        ?>
                    </td>
                </tr>
                <tr>
                    <td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate('On Click'); ?></td>

                    <td class="sq-backend-table-cell" nowrap="nowrap">
                        <?php
                            if ($have_lock) {
                                $GLOBALS['SQ_SYSTEM']->am->includeAsset('content_type_image');
                                $dummy_asset = new Content_Type_Image();
                                $onclick = $dummy_asset->getAttribute('on_click');
                                $onclick->value = $value_defaults['on_click'];
                                $onclick->paint('content_type_image_on_click');
                            } else {
                                echo ucwords($value_defaults['on_click']);
                            }
                        ?>
                    </td>
                </tr>
                <tr>
                    <td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate('HTML Layout'); ?></td>

                    <td class="sq-backend-table-cell" nowrap="nowrap">
                        <?php
                            if ($have_lock) {
                                combo_box('SQ_CONTENT_TYPE_IMAGE_DEFAULTS[html_layout]', Array('table' => 'Table', 'div' => 'Div'), FALSE, array_get_index($value_defaults, 'html_layout', ''));
                            } else {
                                echo ucwords($value_defaults['html_layout']);
                            }
                        ?>
                    </td>
                </tr>
            </table>
            <?php
        });//end closure (SQ_CONTENT_TYPE_IMAGE_DEFAULTS)

		$this->paintField($o, $have_lock, $pref, 'SQ_CONTENT_TYPE_IMAGE_FIELDS', function($fields) use ($have_lock, $tick, $cross) {
            ?>
			<table class="sq-backend-table" style="width: 200px;">
				<?php
				$option_names = Array(
                    'caption_options'   => translate('Caption Options'),
                    'caption_position'  => translate('Caption Position'),
                    'dimension_options' => translate('Dimension Options'),
                    'on_click'          => translate('On Click'),
                    'html_layout'       => translate('HTML Layout'),
                );
            $tick = '<img src="'.sq_web_path('lib').'/web/images/tick.gif" alt="Yes" style="position: relative; right: 0px;"/>';
            $cross = '<img src="'.sq_web_path('lib').'/web/images/cross.gif" alt="No" />';
            ?>
            <table class="sq-backend-table" style="width: 350px">
                <col width="80%" />
                <col width="20%" />
                <?php
                foreach ($option_names as $key => $display_name) {
                    ?><tr><?php
                    if ($have_lock) {
                        $option_id = 'SQ_CONTENT_TYPE_IMAGE_FIELDS['.$key.']';
                        ?><th><?php
                        label($display_name, 'image_content_'.$key);
                        ?></th><td style="text-align: center"><?php
                        check_box($option_id, '1', $fields[$key], '', 'id="image_content_'.$key.'"');
                        ?></td><?php
                    } else {
                        ?><th><?php
                        echo $display_name;
                        ?></th><td style="text-align: center"><?php
                        echo ($fields[$key] == TRUE) ? $tick : $cross;
                        ?></td><?php
                    }
                    ?></tr><?php
                }
                ?>
            </table>
			<?php
		});//end closure (SQ_CONTENT_TYPE_IMAGE_FIELDS)

		$this->paintField($o, $have_lock, $pref, 'SQ_CONTENT_TYPE_IMAGE_CLASSES', function($classes) use ($have_lock) {
			?>
			<table class="sq-backend-table" style="width: 350px;">
				<?php
                $translated_names = Array(
                    'surrounding_container' => translate('Surrounding Container'),
                    'image_table_cell'      => translate('Image Table Cell'),
                    'caption_table_cell'    => translate('Caption Table Cell'),
                    'image_tag'             => translate('Image Tag'),
                    'image_div'             => translate('Image Div'),
                    'caption_div'           => translate('Caption Div'),
                );
			    foreach ($classes as $name => $class) {
					?>
					<tr>
						<td class="sq-backend-table-header"><?php echo $translated_names[$name]; ?></td>
						<td class="sq-backend-table-cell" nowrap="nowrap">
							<?php
								if ($have_lock) {
									text_box('SQ_CONTENT_TYPE_IMAGE_CLASSES['.$name.']', $class);
								} else {
									echo $class;
								}
							?>
						</td>
					</tr>
				<?php
				}
				?>
			</table>
			<?php
		});//end closure (SQ_CONTENT_TYPE_IMAGE_CLASSES)

		$this->paintField($o, $have_lock, $pref, 'SQ_CONTENT_TYPE_IMAGE_PREVIEW', function($classes) use ($have_lock) {
			?>
			<table class="sq-backend-table" style="width: 350px;">
				<?php
                $translated_names = Array(
                    'surrounding_container' => translate('Surrounding Container'),
                    'preview_image_table'   => translate('Preview Image Table'),
                    'image_table_cell'      => translate('Image Table Cell'),
                    'image_tag'             => translate('Image Tag'),
                    'close_preview_cell'    => translate('Close Preview Cell'),
                    'image_div'             => translate('Image Div'),
                    'close_preview_div'     => translate('Close Preview Div'),
                );
                foreach ($classes as $name => $class) {
					?>
					<tr>
						<td class="sq-backend-table-header"><?php echo $translated_names[$name]; ?></td>
						<td class="sq-backend-table-cell" nowrap="nowrap">
							<?php
								if ($have_lock) {
									text_box('SQ_CONTENT_TYPE_IMAGE_PREVIEW['.$name.']', $class);
								} else {
									echo $class;
								}
							?>
						</td>
					</tr>
				<?php
				}
				?>
			</table>
			<?php
		});//end closure (SQ_CONTENT_TYPE_IMAGE_PREVIEW)

		return TRUE;

	}//end paintBackend()


	/**
	* Processes the backend interface to edit preferences
	*
	* @param object		&$o			reference to the backend outputter
	* @param boolean	$have_lock	do we have the lock to edit these preferences or not
	* @param string		$pref		a particular preference to process
	*
	* @return boolean
	* @access public
	*/
	function processBackend(&$o, $have_lock, $pref=NULL)
	{
		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());
		if (!$is_admin) return FALSE;

		if (is_null($pref) || $pref == 'SQ_CONTENT_TYPE_IMAGE_DEFAULTS') {
			$posted_values = $_POST['SQ_CONTENT_TYPE_IMAGE_DEFAULTS'];
			if (!isset($posted_values['use_caption_attr'])) {
				$posted_values['use_caption_attr'] = FALSE;
			}
			if (!isset($posted_values['fit_dimensions'])) {
				$posted_values['fit_dimensions'] = FALSE;
			}
			if (!isset($posted_values['constrain_proportions'])) {
				$posted_values['constrain_proportions'] = FALSE;
			}
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('content_type_image');
			$dummy_asset = new Content_Type_Image();
			$onclick = $dummy_asset->getAttribute('on_click');
			$onclick->process('content_type_image_on_click');
			$posted_values['on_click'] = $onclick->value;
			$this->pref_vars['SQ_CONTENT_TYPE_IMAGE_DEFAULTS']['default'] = $posted_values;
		}
		if (is_null($pref) || $pref == 'SQ_CONTENT_TYPE_IMAGE_FIELDS') {
			foreach ($this->pref_vars['SQ_CONTENT_TYPE_IMAGE_FIELDS']['default'] as $name => $val) {
				$this->pref_vars['SQ_CONTENT_TYPE_IMAGE_FIELDS']['default'][$name] = isset($_POST['SQ_CONTENT_TYPE_IMAGE_FIELDS'][$name]);
			}
		}
		if (is_null($pref) || $pref == 'SQ_CONTENT_TYPE_IMAGE_CLASSES') {
			$this->pref_vars['SQ_CONTENT_TYPE_IMAGE_CLASSES']['default'] = $_POST['SQ_CONTENT_TYPE_IMAGE_CLASSES'];
		}
		if (is_null($pref) || $pref == 'SQ_CONTENT_TYPE_IMAGE_PREVIEW') {
			$this->pref_vars['SQ_CONTENT_TYPE_IMAGE_PREVIEW']['default'] = $_POST['SQ_CONTENT_TYPE_IMAGE_PREVIEW'];
		}
		return TRUE;

	}//end processBackend()


}//end class

?>
