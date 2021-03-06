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
* $Id: content_type_code_prefs.inc,v 1.9 2012/08/30 00:58:07 ewang Exp $
*
*/


require_once SQ_LIB_PATH.'/config/prefs.inc';
require_once SQ_ATTRIBUTES_PATH.'/css_dimension/css_dimension.inc';

/**
* Content_Type_Code_Prefs
*
* Purpose
*
*    Looks after the creation the Raw HTML preferences
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.9 $
* @package MySource_Matrix
*/
class Content_Type_Code_Prefs extends Prefs
{

	var $pref_vars = Array (
						'SQ_CODE_EDITOR_SIZE'			=> Array(
															'default'		=> Array('width' => '400', 'height' => '400'),
															'protected'		=> FALSE,
														   ),
						'SQ_CODE_EDITOR_HTML_CLASSES'	=> Array(
															'default'		=> Array(
																				'attribute_names'	=> 'html_attribute',
																				'attribute_value'	=> 'html_attribute_values',
																				'tag_brackets'		=> 'html_tag_brackets',
																				'tag_name'			=> 'html_tag_names',
																				'html_comment'		=> 'html_comment',
																			   ),
															'protected'		=> FALSE,
														   ),
						'SQ_CODE_EDITOR_PHP_CLASSES'	=> Array(
															'default'		=> Array(
																				'background'	=> 'php_background',
																				'default'		=> 'php_default',
																				'html'			=> 'php_html',
																				'comment'		=> 'php_comment',
																				'keyword'		=> 'php_keyword',
																				'string'		=> 'php_string',
																			   ),
															'protected'		=> FALSE,
														   ),
						'SQ_CODE_EDITOR_JS_CLASSES'		=> Array(
															'default'		=> Array(
																				'reserved'	=> 'js_reserved',
																				'function'	=> 'js_function',
																				'brackets'	=> 'js_brackets',
																				'comment'	=> 'js_comment',
																				'default'	=> 'js_default',
																				'string'	=> 'js_string',
																			   ),
															'protected'		=> FALSE,
														   ),
						'SQ_CODE_EDITOR_LINE_NUMBERS'	=> Array(
															'default'		=> Array(
																				'use_line_numbers'	=> FALSE,
																				'line_number_left'	=> TRUE,
																				'line_number_style'	=> 'span',
																				'line_number_class'	=> 'line_numbers',
																			   ),
															'protected'		=> FALSE,
														   ),
					 );
	var $css_dimension_width;
	var $css_dimension_height;


	/**
	* Constructor
	*
	*/
	function Content_Type_Code_Prefs($pref_file='')
	{
		$this->Prefs($pref_file);
		$this->css_dimension_width = new Asset_Attribute_CSS_Dimension();
		$this->css_dimension_height = new Asset_Attribute_CSS_Dimension();

		$this->pref_vars['SQ_CODE_EDITOR_SIZE']['name']         = translate('Code Editor Size');
		$this->pref_vars['SQ_CODE_EDITOR_HTML_CLASSES']['name'] = translate('HTML Code Highlighting Classes');
		$this->pref_vars['SQ_CODE_EDITOR_PHP_CLASSES']['name']  = translate('PHP Code Highlighting Classes');
		$this->pref_vars['SQ_CODE_EDITOR_JS_CLASSES']['name']   = translate('JavaScript Code Highlighting Classes');
		$this->pref_vars['SQ_CODE_EDITOR_LINE_NUMBERS']['name'] = translate('Line Numbers');

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

        $css_dimension_width = $this->css_dimension_width;
        $css_dimension_height = $this->css_dimension_height;
		$this->paintField($o, $have_lock, $pref, 'SQ_CODE_EDITOR_SIZE', function($code_editor_size) use ($have_lock, $css_dimension_width, $css_dimension_height) {
            ?>
            <table class="sq-backend-table" style="width: 350px;">
                <tr>
                    <td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate('Width') ?></td>

                    <td class="sq-backend-table-cell" style="width: 100%;">
                    <?php
                    if ($have_lock) {
                        $css_dimension_width->paint('code_width', $have_lock, $code_editor_size['width']);
                    } else {
                        echo $code_editor_size['width'];
                    }
                    ?>
                    </td>
                </tr>
                <tr>
                    <td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate('Height') ?></td>

                    <td class="sq-backend-table-cell" style="width: 100%;">
                    <?php
                    if ($have_lock) {
                        $css_dimension_height->paint('code_height', $have_lock, $code_editor_size['height']);
                    } else {
                        echo $code_editor_size['height'];
                    }
                    ?>
                    </td>
                </tr>
            </table>
            <?php
        });//end closure (SQ_CODE_EDITOR_SIZE)

		// HTML Classes
		$this->paintField($o, $have_lock, $pref, 'SQ_CODE_EDITOR_HTML_CLASSES', function($html_classes) use ($have_lock) {
			?>
			<table class="sq-backend-table" style="width: 350px;">
			<?php
			$translated_names = Array(
			    'attribute_names' => translate('Attribute Names'),
			    'attribute_value' => translate('Attribute Value'),
			    'tag_brackets'    => translate('Tag Brackets'),
			    'tag_name'        => translate('Tag Name'),
			    'html_comment'    => translate('HTML Comment'),
			);
			foreach ($html_classes as $class_name => $choice) {
				?>
				<tr>
					<td class="sq-backend-table-header" nowrap="nowrap" style="width: 100px;">
						<?php echo $translated_names[$class_name]; ?>
					</td>
					<td class="sq-backend-table-cell" style="width: 100%;">
					<?php
					if ($have_lock) {
						text_box('SQ_CODE_EDITOR_HTML_CLASSES['.$class_name.']', $choice);
					} else {
						echo $choice;
					}
					?>
					</td>
				</tr>
				<?php
			}
				?>
			</table>
			<?php
		});//end closure (SQ_CODE_EDITOR_HTML_CLASSES)

		// Classes for PHP code
		$this->paintField($o, $have_lock, $pref, 'SQ_CODE_EDITOR_PHP_CLASSES', function($php_classes) use ($have_lock) {
			?>
			<table class="sq-backend-table" style="width: 350px;">
			<?php
			$translated_names = Array(
			    'background' => translate('Background'),
			    'default'    => translate('Default'),
			    'html'       => translate('HTML'),
			    'comment'    => translate('Comment'),
			    'keyword'    => translate('Keyword'),
			    'string'     => translate('String'),
			);
			foreach ($php_classes as $class_name => $choice) {
				?>
				<tr>
					<td class="sq-backend-table-header" nowrap="nowrap" style="width: 100px;">
						<?php echo $translated_names[$class_name]; ?>
					</td>
					<td class="sq-backend-table-cell" style="width: 100%;">
					<?php
					if ($have_lock) {
						text_box('SQ_CODE_EDITOR_PHP_CLASSES['.$class_name.']', $choice);
					} else {
						echo $choice;
					}
					?>
					</td>
				</tr>
				<?php
			}
				?>
			</table>
			<?php
		});//end closure (SQ_CODE_EDITOR_PHP_CLASSES)

		// Classes for JavaScript code
		$this->paintField($o, $have_lock, $pref, 'SQ_CODE_EDITOR_JS_CLASSES', function($js_classes) use ($have_lock) {
			?>
			<table class="sq-backend-table" style="width: 350px;">
			<?php
			$translated_names = Array(
			    'reserved'   => translate('Reserved'),
			    'function'   => translate('Function'),
			    'brackets'   => translate('Brackets'),
			    'comment'    => translate('Comment'),
			    'default'    => translate('Default'),
			    'string'     => translate('String'),
			);
			foreach ($js_classes as $class_name => $choice) {
				?>
				<tr>
					<td class="sq-backend-table-header" nowrap="nowrap" style="width: 100px;">
						<?php echo $translated_names[$class_name]; ?>
					</td>
					<td class="sq-backend-table-cell" style="width: 100%;">
					<?php
					if ($have_lock) {
						text_box('SQ_CODE_EDITOR_JS_CLASSES['.$class_name.']', $choice);
					} else {
						echo $choice;
					}
					?>
					</td>
				</tr>
				<?php
			}
				?>
			</table>
			<?php
		});//end closure (SQ_CODE_EDITOR_JS_CLASSES)

		// Line Number settings.
		$this->paintField($o, $have_lock, $pref, 'SQ_CODE_EDITOR_LINE_NUMBERS', function($line_prefs) use ($have_lock) {
			?>
			<table class="sq-backend-table" style="width: 350px;">
				<tr>
					<td class="sq-backend-table-header" nowrap="nowrap" style="width: 100px;">
						<?php echo translate('Show Line Numbers'); ?>

					</td>
					<td class="sq-backend-table-cell" style="width: 100%;">
					<?php
					$options = Array(
					    0 => translate('Hide line numbers'),
					    1 => translate('Show line numbers'),
					);

					if ($have_lock) {
					    combo_box('SQ_CODE_show_line_numbers', $options, FALSE, $line_prefs['use_line_numbers']);
					} else {
                        $options = Array(
                            0 => translate('Line numbers hidden'),
                            1 => translate('Line numbers shown'),
                        );

						echo $options[$line_prefs['use_line_numbers']];
					}
					?>
					</td>
				</tr>
				<tr>
					<td class="sq-backend-table-header" nowrap="nowrap" style="width: 100px;">
						<?php echo translate('Line Number Orientation'); ?>

					</td>
					<td class="sq-backend-table-cell" style="width: 100%;">
					<?php
					$options = Array(
					    1 => translate('Line numbers on left'),
					    0 => translate('Line numbers on right'),
					);

					if ($have_lock) {
					    combo_box('SQ_CODE_line_number_left', $options, FALSE, $line_prefs['line_number_left']);
					} else {
						echo $options[$line_prefs['line_number_left']];
					}
					?>
					</td>
				</tr>
				<tr>
					<td class="sq-backend-table-header" nowrap="nowrap" style="width: 100px;">
						<?php echo translate('Line Number Style'); ?>

					</td>
					<td class="sq-backend-table-cell" style="width: 100%;">
					<?php
					$options = Array(
					    'span' => translate('Span'),
					    'list' => translate('List'),
					);

					if ($have_lock) {
					    combo_box('SQ_CODE_line_style', $options, FALSE, $line_prefs['line_number_style']);
					} else {
						echo $options[$line_prefs['line_number_style']];
					}
					?>
					</td>
				</tr>
				<tr>
					<td class="sq-backend-table-header" nowrap="nowrap" style="width: 100px;">
						<?php echo translate('Line Number Class'); ?>

					</td>
					<td class="sq-backend-table-cell" style="width: 100%;">
					<?php
					if ($have_lock) {
						text_box('SQ_CODE_line_class', $line_prefs['line_number_class']);
					} else {
						echo $line_prefs['line_number_class'];
					}
					?>
					</td>
				</tr>
			</table>
			<?php
		});//end closure (SQ_CODE_EDITOR_LINE_NUMBERS)

		return TRUE;

	}//end paintBackend()


	/**
	* Processes the backend interface to edit preferences
	*
	* @param object		$o			reference to the backend outputter
	* @param boolean	$have_lock	do we have the lock to edit these preferences or not
	* @param string		$pref		a particular preference to process
	*
	* @return boolean
	* @access public
	*/
	function processBackend($o, $have_lock, $pref=NULL)
	{

		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());
		if (!$is_admin) return FALSE;

		if (is_null($pref) || $pref == 'SQ_CODE_EDITOR_SIZE') {
			$this->css_dimension_width->process('code_width');
			$this->css_dimension_height->process('code_height');
			$this->pref_vars['SQ_CODE_EDITOR_SIZE']['default']['width'] = $this->css_dimension_width->getContent();
			$this->pref_vars['SQ_CODE_EDITOR_SIZE']['default']['height'] = $this->css_dimension_height->getContent();
		}

		if(isset($_POST['SQ_CODE_EDITOR_HTML_CLASSES'])) {
			if (is_null($pref) || $pref == 'SQ_CODE_EDITOR_HTML_CLASSES') {
				$this->pref_vars['SQ_CODE_EDITOR_HTML_CLASSES']['default'] = $_POST['SQ_CODE_EDITOR_HTML_CLASSES'];
			}
		}

		if(isset($_POST['SQ_CODE_EDITOR_PHP_CLASSES'])) {
			if (is_null($pref) || $pref == 'SQ_CODE_EDITOR_PHP_CLASSES') {
				$this->pref_vars['SQ_CODE_EDITOR_PHP_CLASSES']['default'] = $_POST['SQ_CODE_EDITOR_PHP_CLASSES'];
			}
		}

		if(isset($_POST['SQ_CODE_EDITOR_JS_CLASSES'])) {
			if (is_null($pref) || $pref == 'SQ_CODE_EDITOR_JS_CLASSES') {
				$this->pref_vars['SQ_CODE_EDITOR_JS_CLASSES']['default'] = $_POST['SQ_CODE_EDITOR_JS_CLASSES'];
			}
		}

		if (is_null($pref) || $pref == 'SQ_CODE_EDITOR_LINE_NUMBERS') {
        	$line_prefs = $this->pref_vars['SQ_CODE_EDITOR_LINE_NUMBERS']['default'];
			$line_prefs['use_line_numbers'] = isset($_POST['SQ_CODE_show_line_numbers']);
			if(isset($_POST['SQ_CODE_line_style'])) {
				$line_prefs['line_number_style'] = $_POST['SQ_CODE_line_style'];
			}
			if(isset($_POST['SQ_CODE_line_class'])) {
				$line_prefs['line_number_class'] = $_POST['SQ_CODE_line_class'];
			}
			if(isset($_POST['SQ_CODE_line_number_left'])) {
				$line_prefs['line_number_left'] = $_POST['SQ_CODE_line_number_left'];
			}
			$this->pref_vars['SQ_CODE_EDITOR_LINE_NUMBERS']['default'] = $line_prefs;
		}
		return TRUE;

	}//end processBackend()


}//end class

?>
