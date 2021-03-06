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
* $Id: content_type_structure_tree_prefs.inc,v 1.8 2012/08/30 00:58:07 ewang Exp $
*
*/


require_once SQ_LIB_PATH.'/config/prefs.inc';
require_once SQ_ATTRIBUTES_PATH.'/css_dimension/css_dimension.inc';

/**
* Content_Type_Structure_Tree_Prefs
*
* Purpose
*
*    Looks after the creation the Structure Tree
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix
*/
class Content_Type_Structure_Tree_Prefs extends Prefs
{

	var $pref_vars = Array (
						'SQ_STRUCTURE_TREE_FORMATTING'		=> Array(
																'default'		=> Array(
																					'bold'			=> TRUE,
																					'underline'		=> TRUE,
																					'italic'		=> TRUE,
																					'delete'		=> TRUE,
																					'insert'		=> TRUE,
																					'connections'	=> TRUE,
																				   ),
																'protected'		=> FALSE,
															   ),
						'SQ_STRUCTURE_TREE_NODE_CLASSES'	=> Array(
																'default'		=> Array(
																					'node_text'		=> 'tree_node_text',
																					'node_branches'	=> 'tree_node_branches',
																				   ),
																'protected'		=> FALSE,
															   ),
					 );


	/**
	* Constructor
	*
	*/
	function __construct($pref_file='')
	{
		parent::__construct($pref_file);

		$this->pref_vars['SQ_STRUCTURE_TREE_FORMATTING']['name']   = translate('Structured Tree Formatting Options');
		$this->pref_vars['SQ_STRUCTURE_TREE_NODE_CLASSES']['name'] = translate('Structured Tree CSS Classes');

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
        $is_admin  = ($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());
	    $have_lock = $have_lock && $is_admin;

	    $this->paintField($o, $have_lock, $pref, 'SQ_STRUCTURE_TREE_FORMATTING', function($format_options) use ($have_lock) {
	        $option_names = Array(
	            'bold'        => translate('Bold'),
	            'underline'   => translate('Underline'),
	            'italic'      => translate('Italic'),
	            'delete'      => translate('Delete'),
	            'insert'      => translate('Insert'),
	            'connections' => translate('Connections'),
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
                        $option_id = 'SQ_STRUCTURE_SHOW_'.strtoupper($key);
                        ?><th><?php
                        label($display_name, $option_id);
                        ?></th><td style="text-align: center"><?php
                        check_box($option_id, '1', $format_options[$key]);
                        ?></td><?php
                    } else {
                        ?><th><?php
                        echo $display_name;
                        ?></th><td style="text-align: center"><?php
                        echo ($format_options[$key] == TRUE) ? $tick : $cross;
                        ?></td><?php
                    }
                    ?></tr><?php
                }
                ?>
            </table>
            <?php
		});//end SQ_STRUCTURE_TREE_FORMATTING

		$this->paintField($o, $have_lock, $pref, 'SQ_STRUCTURE_TREE_NODE_CLASSES', function($classes) use ($have_lock) {
            ?>
            <table class="sq-backend-table" style="width: 350px;">
            <?php
                $class_text = Array(
                               'node_text'     => translate('Node Text'),
                               'node_branches' => translate('Node Branches'),
                              );
                foreach ($classes as $name => $class) {
            ?>
                    <tr>
                        <td class="sq-backend-table-header" nowrap="nowrap"><?php echo $class_text[$name]; ?></td>
                        <td class="sq-backend-table-cell" style="width: 100%;">
                        <?php
                            if ($have_lock) {
                                text_box('SQ_STRUCTURE_TREE_NODE_CLASSES['.$name.']', $class, '20');
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
		});//end if

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

		if (is_null($pref) || $pref == 'SQ_STRUCTURE_TREE_FORMATTING') {
			$this->pref_vars['SQ_STRUCTURE_TREE_FORMATTING']['default']['bold'] = isset($_POST['SQ_STRUCTURE_SHOW_BOLD']);
			$this->pref_vars['SQ_STRUCTURE_TREE_FORMATTING']['default']['underline'] = isset($_POST['SQ_STRUCTURE_SHOW_UNDERLINE']);
			$this->pref_vars['SQ_STRUCTURE_TREE_FORMATTING']['default']['italic'] = isset($_POST['SQ_STRUCTURE_SHOW_ITALIC']);
			$this->pref_vars['SQ_STRUCTURE_TREE_FORMATTING']['default']['delete'] = isset($_POST['SQ_STRUCTURE_SHOW_DELETE']);
			$this->pref_vars['SQ_STRUCTURE_TREE_FORMATTING']['default']['insert'] = isset($_POST['SQ_STRUCTURE_SHOW_INSERT']);
			$this->pref_vars['SQ_STRUCTURE_TREE_FORMATTING']['default']['connections'] = isset($_POST['SQ_STRUCTURE_SHOW_CONNECTIONS']);
		}
		if (is_null($pref) || $pref == 'SQ_STRUCTURE_TREE_NODE_CLASSES') {
			$this->pref_vars['SQ_STRUCTURE_TREE_NODE_CLASSES']['default'] = $_POST['SQ_STRUCTURE_TREE_NODE_CLASSES'];
		}
		return TRUE;

	}//end processBackend()


}//end class

?>