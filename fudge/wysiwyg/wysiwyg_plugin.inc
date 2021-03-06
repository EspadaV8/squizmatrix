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
* $Id: wysiwyg_plugin.inc,v 1.22 2012/08/30 00:56:51 ewang Exp $
*
*/
require_once dirname(__FILE__).'/../../core/include/init.inc';

/**
* WYSIWYG Plugin
*
* Purpose
*     A class used to create a plugin for the WYSIWYG
*     editor. Must bee added using the add_plugin function
*     of wysiwyg.inc
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.22 $
* @package Fudge
* @subpackage wysiwyg
*/
class wysiwyg_plugin
{

	/**
	* An array of buttons that the plugin wants printed on the toolbar
	*
	* @var array
	*/
	var $buttons = Array();

	/**
	* Show this plugin in the toolbar or not
	* Some plugins will be just for modifying HTML on submit or something
	* like that so not all of them actually have buttons
	*
	* @var array
	*/
	var $_show_in_toolbar = TRUE;

	/**
	* A reference to the WYSIWYG object that created this plugin
	*
	* @var object WYSIWYG
	*/
	var $wysiwyg = NULL;


	/**
	* Constructor
	*
	* @param object wysiwyg	$wysiwyg	A reference to the WYSIWYG that
	*									this plugin is being created for
	*
	* @return object WYSIWYG_Plugin
	* @access public
	*/
	function WYSIWYG_Plugin(&$wysiwyg)
	{
		$this->wysiwyg =& $wysiwyg;

	}//end constructor


	/**
	* Adds a printable toolbar button to this plugin
	*
	* @param string	$id			A unique ID for this button
	* @param string	$cmd		The command to run when the button is clicked
	*							This command will be used to switch on action of
	*							a pressed button to determine the functionality
	* @param string	$tooltip	The tooltip for this button (appears on hover)
	* @param string	$textmode	Should this button be available in text mode?
	*							Value is either 'true' or 'false' (with quotes)
	* @param string	$position	The position of the button in the toolbar image
	*							This is zero indexed (first button is id zero)
	* @param string	$context	Enable this button if outside this element
	*
	* @return void
	* @access private
	*/
	function _add_button($id, $cmd, $tooltip, $textmode, $position, $context='')
	{
		$this->buttons[$id]['command']  = $cmd;
		$this->buttons[$id]['tooltip']  = $tooltip;
		$this->buttons[$id]['textmode'] = $textmode;
		$this->buttons[$id]['position'] = $position;
		$this->buttons[$id]['context']  = $context;

	}//end _add_button()


	/**
	* Opens a drop down menu to group buttons
	*
	* @param string	$id			A unique ID for this menu
	* @param string	$tooltip	The tooltip for the menu button (appears on hover)
	* @param string	$textmode	Should this menu be available in text mode?
	*							Value is either 'true' or 'false' (with quotes)
	* @param string	$position	The position of the menu button in the toolbar image
	*							This is zero indexed (first button is id zero)
	* @param string	$context	Enable this menu if outside this element
	*
	* @return void
	* @access private
	*/
	function _open_menu($id, $tooltip, $textmode, $position, $context='')
	{
		$this->buttons[$id]['command']    = 'htmlarea-menu-'.$id;
		$this->buttons[$id]['tooltip']    = $tooltip;
		$this->buttons[$id]['textmode']   = $textmode;
		$this->buttons[$id]['position']   = $position;
		$this->buttons[$id]['context']    = $context;
		$this->buttons[$id]['menu_start'] = TRUE;

	}//end _open_menu()


	/**
	* Closes an open menu of grouped buttons
	*
	* @param string	$id	A unique ID of the menu
	*
	* @return void
	* @access private
	*/
	function _close_menu($id)
	{
		$this->buttons['menu_end_'.$id]['menu_end'] = TRUE;

	}//end _close_menu()


	/**
	* Prints any JS variable needed by the WYSIWYG editor
	* This code is output in the HTMLArea construtor - do not use script tags
	*
	* @return void
	* @access public
	*/
	function print_plugin_vars()
	{
		return;

	}//end print_plugin_vars()


	/**
	* Prints shortcut keys for the buttons of this plugin
	* This code is a case statement - do not use script tags
	*
	* @return void
	* @access public
	*/
	function print_plugin_shortcuts()
	{
		return;

	}//end print_plugin_shortcuts()


	/**
	* Prints any JS code needed to get the options array for any select boxes in this plugin
	* This code is a case statement - do not use script tags
	*
	* @return void
	* @access public
	*/
	function print_plugin_create_select()
	{
		return;

	}//end print_plugin_create_select()


	/**
	* Paints any JS functions needed by this plugin on a per WYSIWYG basis
	* This code is printed outside of script tags - print script tags yourself
	*
	* @return void
	* @access public
	*/
	function paint()
	{
		return;

	}//end paint()


	/**
	* Processes and returns the submitted HTML from the editor
	* This allows plugins to modify the html as they see fit.
	*
	* @param string	&$html	the HTML that was submitted by the editor
	*
	* @return string
	* @access public
	*/
	function process(&$html)
	{
		return $html;

	}//end process()


	/**
	* Sets the initial contents of the WYSIWYG editor
	*
	* @param string	&$contents	Some HTML content
	*
	* @return void
	* @access public
	*/
	function set_contents(&$contents)
	{
		return $contents;

	}//end set_contents()


	/**
	* Paints any JS generic functions needed by this plugin
	* These functions can be used by all WYSIWYGs on the page
	* This code is printed outside of script tags - print script tags yourself
	*
	* @return void
	* @access public
	*/
	function paint_generic()
	{
		return;

	}//end paint_generic()


	/**
	* Prints JS executed when a button of this plugin is clicked
	* This code is a case statement - do not use script tags
	*
	* @return void
	* @access public
	*/
	function print_plugin_button_click()
	{
		return;

	}//end print_plugin_button_click()


	/**
	* Prints the JS executed on select for any select boxes in this plugin
	* This code is a case statement - do not use script tags
	*
	* @return void
	* @access public
	*/
	function print_plugin_combo_selected()
	{
		return;

	}//end print_plugin_combo_selected()


	/**
	* Prints any JS the plugin wants executed before the WYSIWYG has its way
	* This code is a case statement - do not use script tags
	*
	* @return void
	* @access public
	*/
	function print_plugin_exec_command()
	{
		return;

	}//end print_plugin_exec_command()


	/**
	* Prints the JS executed by buttons of this plugin when the toolbar is updated
	* This code is a case statement - do not use script tags
	*
	* @return void
	* @access public
	*/
	function print_plugin_update_toolbar()
	{
		return;

	}//end print_plugin_update_toolbar()


	/**
	* Prints JS to create special types of buttons for this plugin
	* This code is a case statement - do not use script tags
	*
	* @return void
	* @access public
	*/
	function print_plugin_button_type()
	{
		return;

	}//end print_plugin_button_type()


	/**
	* Prints any JS the plugin wants executed on returned HTML from getHTML
	* This code is a function - do not use script tags
	*
	* @return void
	* @access public
	*/
	function print_plugin_get_html()
	{
		return;

	}//end print_plugin_get_html()


	/**
	* Prints any JS the plugin wants executed before getHTML runs
	* This code is a function - do not use script tags
	*
	* @return void
	* @access public
	*/
	function print_plugin_pre_get_html()
	{
		return;

	}//end print_plugin_pre_get_html()


	/**
	* Prints any JS the plugin wants executed before submitting the form
	* This code is a function - do not use script tags
	*
	* @return void
	* @access public
	*/
	function print_plugin_form_submit()
	{
		return;

	}//end print_plugin_form_submit()


	/**
	* Prints an JS the plugin wants executed when an event is fired in the wysiwyg's iframe or div
	* This code is in a function - do not use script tags
	* You can reference the variables 'editor' and 'ev' ( = the event fired)
	* from within this code
	*
	* @return void
	* @access public
	*/
	function print_plugin_event_handler()
	{
		return;

	}//end print_plugin_event_handler()


	/**
	* Returns a path to a file in this plugin's directory
	* This value can be passed to the _popupDialog function of the
	* WYSIWYG object
	*
	* @param $filename
	* @param $classname
	*
	* @return string
	* @access public
	*/
	function get_popup_href($filename, $classname='')
	{
		if (empty($classname)) $classname = get_class_lower($this);
		return $classname.'/'.$filename;

	}//end get_popup_href()


	/**
 	* Check whether this plugin is ready to be used.
 	*
 	* Return a TRUE if ok or a error message.
 	*
 	* @return mixed boolean|string
 	* @access public
 	* @static
	*/
	public static function check_usable()
	{
		return TRUE;

	}//end check_usable()


}//end class

?>
