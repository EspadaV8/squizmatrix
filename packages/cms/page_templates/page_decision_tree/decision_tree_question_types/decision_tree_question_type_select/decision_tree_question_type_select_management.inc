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
* $Id: decision_tree_question_type_select_management.inc,v 1.2 2012/08/30 00:58:08 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Decision_Tree_Question_Type_Select_Management
*
* @see Asset_Management
*
* @author  Mohamed Haidar <mhaidar@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Decision_Tree_Question_Type_Select_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Decision_Tree_Question_Type_Select_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'style'		=> Array(
											'added'			=> '0.0.1',
											'type'			=> 'selection',
											'default'		=> 'DROP_DOWN',
											'parameters'	=> Array(
																'options'	=> Array(
																				'DROP_DOWN'	=> 'Drop Down',
																				'RADIO'		=> 'Radio',
																			   ),
																'allow_empty'	=> FALSE,
																'multiple'		=> FALSE
															   ),
										   ),
						'options'		=> Array(
											'added'		=> '0.0.1',
											'type'		=> 'serialise',
											'default'	=> Array(),
										   ),
						'extra'			=> Array(
											'added'		=> '0.0.1',
											'type'		=> 'text',
											'default'	=> '',
										   ),
						'empty_key'		=> Array(
											'added'		=> '0.0.1',
											'type'		=> 'text',
											'default'	=> 'EMPTY',
										   ),
						'empty_text'	=> Array(
											'added'		=> '0.0.1',
											'type'		=> 'text',
											'default'	=> 'Please Select',
										   ),
					  );

	}//end constructor


}//end class
?>
