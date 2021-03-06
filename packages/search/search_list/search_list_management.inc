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
* $Id: search_list_management.inc,v 1.10 2012/08/30 00:59:15 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Search_List_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage search
*/
class Search_List_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Search_List_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars['default_format'] = Array(
											'added'		=> '0.1',
											'type'		=> 'wysiwyg',
											'default'	=> '%search_term% ( %number_of_occurrences% )',
										);

		$this->vars['search_field']   = Array(
											'added'		=> '0.1',
											'type'		=> 'text',
											'default'	=> '',
										);
		
		$this->vars['group_by'] = Array(
									'added'			=> '0.1',
									'type'			=> 'selection',
									'default'		=> 'number',
									'parameters'	=> Array(
														'multiple'		=> FALSE,
														'allow_empty'	=> FALSE,
														'options'		=> Array(
																			'number'		=> 'Standard',
																			'random'		=> 'Random',
																		   ),
													   ),
								  );

	}//end constructor


	/**
	* Return an array with information about the events that this page is listening for
	*
	* @return boolean
	* @access public
	*/
	function getEventList()
	{
		return Array(
				Array(
					'event_name'					=> 'requestKeywords',
					'broadcast_type_code'			=> 'content_type',
					'broadcast_strict_type_code'	=> FALSE,
					'options'						=> Array(
														'side_of_link'	=> 'major',
														'indirect'		=> TRUE,
														'is_exclusive'	=> NULL,
														'is_dependant'	=> 1,
														'value'			=> '',
														'link_type'		=> SQ_LINK_TYPE_2 | SQ_LINK_TYPE_1,
													   ),
				),
			   );

	}//end getEventList()

}//end class

?>