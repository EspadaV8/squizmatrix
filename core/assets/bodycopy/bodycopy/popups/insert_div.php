<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |

* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: insert_div.php,v 1.7 2004/12/06 14:38:07 brobertson Exp $
*
*/

/**
* Insert DIV Pop-Up
*
* Purpose
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
header("Cache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0");
header("Pragma: no-cache");
header("Expires: ". gmdate("D, d M Y H:i:s",time()-3600) . " GMT");

include(dirname(__FILE__)."/header.php");
?>
<script type="text/javascript" language="javascript" src="<?php echo sq_web_path('lib')?>/js/general.js"></script>
<script language="JavaScript" type="text/javascript">

	function popup_init() {
		var f = document.main_form;
	}// end popup_init()

	function popup_save(f) {
		var data = new Object();
		data["identifier"]  = owner.form_element_value(f.identifier);
		data["layout_type"] = owner.form_element_value(f.layout_type);
		data["css_class"]   = owner.form_element_value(f.css_class);
		owner.bodycopy_save_insert_div(data);
	}

</script>

<div class="title" style="text-align: right;">Insert DIV</div>

<form name="main_form">
<input type="hidden" name="bodycopy_name" value="">
<input type="hidden" name="divid" value="">
<table width="100%" border="0">
	<tr>
		<td>
		<fieldset>
			<legend><b>Identification</b></legend>
			<table style="width:100%">
				<tr>
					<td class="label">Name:</td>
					<td><input type="text" name="identifier" value="" size="15"></td>
				</tr>
			</table>
		</fieldset>
		<fieldset>
			<legend><b>Style Information</b></legend>
			<table style="width:100%">
				<tr>
					<td class="bodycopy-popup-heading">Presentation:</td>
					<td>
						<select name="layout_type">
							<option value="div" >Block-level</option>
							<option value="span">Inline</option>
							<option value="none">Raw HTML</option>
						</select>
					</td>
				</tr>
				<tr>
					<td class="label">Class:</td>
					<td><input type="text" name="css_class" value="" size="15"></td>
				</tr>
			</table>
		</fieldset>
		</td>
	</tr>
	<tr>
		<td>
		<div style="text-align: right;">
		<button type="button" name="ok" onClick="javascript: popup_save(this.form)">OK</button>
		&nbsp;
		<button type="button" name="cancel" onClick="javascript: popup_close();">Cancel</button>
		</div>
		</td>
	</tr>
</table>
</form>

<?php include(dirname(__FILE__)."/footer.php"); ?>