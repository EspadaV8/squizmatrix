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
* $Id: edit_table_cell_props.php,v 1.6 2005/05/11 06:04:49 lwright Exp $
*
*/

/**
* Table Cell Properties Pop-Up
*
* Purpose
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
header("Cache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0");
header("Pragma: no-cache");
header("Expires: ". gmdate("D, d M Y H:i:s",time()-3600) . " GMT");

include(dirname(__FILE__)."/header.php");
?>
<script language="JavaScript" type="text/javascript">

	function popup_init() {

		var data = owner.bodycopy_current_edit["data"]["attributes"];
		available_types = owner.bodycopy_current_edit["data"]["available_types"];

		var f = document.main_form;
		f.width.value   = (data['width']  == null) ? "" : data['width'];
		f.height.value  = (data['height'] == null) ? "" : data['height'];
		f.colspan.value = (data['colspan'] == null) ? "" : data['colspan'];
		f.bgcolor.value = (data['bgcolor'] == null) ? "" : data['bgcolor'];

		owner.highlight_combo_value(f.align,  data['align']);
		owner.highlight_combo_value(f.valign, data['valign']);
		owner.highlight_combo_value(f.nowrap, data['nowrap']);

		// remove the existing values
		for(var i = f.type.options.length - 1; i >= 0; i--) {
			f.type.options[i] = null;
		}
		var i = 0;
		for(var key in available_types) {
			if (available_types[key] == null) continue;
			if(available_types[key]["name"] != null) {
				f.type.options[i] = new Option(available_types[key]["name"], key);
				i++;
			}
		}

		owner.highlight_combo_value(f.type, data["content_type"]);

	}// end popup_init()

	function save_props(f) {

		var data = new Object();
		data["width"]    = owner.form_element_value(f.width);
		data["height"]   = owner.form_element_value(f.height);
		data["colspan"]  = owner.form_element_value(f.colspan);
		data["bgcolor"]  = owner.form_element_value(f.bgcolor);
		data["align"]    = owner.form_element_value(f.align);
		data["valign"]   = owner.form_element_value(f.valign);
		data["nowrap"]   = owner.form_element_value(f.nowrap);
		data["type"]     = owner.form_element_value(f.type);
		owner.bodycopy_save_table_cell_properties(data);
	}
</script>

<div class="title">Table Cell Properties</div>

<form name="main_form">
<table width="100%" border="0">
	<tr>
		<td>
			<table width="100%" cellspacing="0" cellpadding="0">
				<tr>
					<td valign="top" width="50%">
						<fieldset>
						<legend><b><?php echo translate('layout'); ?></b></legend>
						<table style="width:100%">
							<tr>
								<td class="label"><?php echo translate('width'); ?>:</td>
								<td><input type="text" name="width" value="" size="5"></td>
							</tr>
							<tr>
								<td class="label"><?php echo translate('height'); ?>:</td>
								<td><input type="text" name="height" value="" size="5"></td>
							</tr>
							<tr>
								<td class="label"><?php echo translate('colspan'); ?>:</td>
								<td><input type="text" name="colspan" value="" size="5"></td>
							</tr>
						</table>
						</fieldset>
					</td>
					<td>&nbsp;</td>
					<td valign="top" width="50%">
						<fieldset>
						<legend><b><?php echo translate('alignment'); ?></b></legend>
						<table style="width:100%">
							<tr>
								<td class="label"><?php echo translate('horizontal'); ?>:</td>
								<td>
								<select name="align">
									<option value="">
									<option value="left"  ><?php echo translate('left'); ?>
									<option value="center"><?php echo translate('centre'); ?>
									<option value="right" ><?php echo translate('right');?>
								</select>
								</td>
							</tr>
							<tr>
								<td class="label"><?php echo translate('vertical'); ?>:</td>
								<td>
								<select name="valign">
									<option value="">
									<option value="middle"  ><?php echo translate('middle'); ?>
									<option value="top"     ><?php echo translate('top'); ?>
									<option value="bottom"  ><?php echo translate('bottom'); ?>
								</select>
								</td>
							</tr>
						</table>
						</fieldset>
					</td>
				</tr>
			</table>
		</td>
	</tr>
	<tr>
		<td>
			<fieldset>
			<legend><b><?php echo translate('cell_styles-colours'); ?></b></legend>
			<table style="width:100%">
				<tr>
					<td class="label"><?php echo translate('background_colour'); ?>:</td>
					<td><?php colour_box('bgcolor', '', true, '*',true, false, false);?></td>
				</tr>
				<tr>
					<td class="label"><?php echo translate('no_text_wrap'); ?>:</td>
					<td>
					<select name="nowrap">
						<option value=""><?php echo translate('off'); ?>
						<option value="on"><?php echo translate('on'); ?>
					</select>
					</td>
				</tr>
			</table>
			</fieldset>
		</td>
	</tr>
	<tr>
		<td>
			<fieldset>
			<legend><b><?php echo translate('cell_type'); ?></b></legend>
			<table style="width:100%">
				<tr>
					<td class="label"><?php echo translate('cell_type'); ?>:</td>
					<td>
					<select name="type">
						<option value="">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</option>
						<option value="">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</option>
						<option value="">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</option>
					</select>
					</td>
				</tr>
			</table>
			</fieldset>
		</td>
	</tr>
	<tr>
		<td>
			<div style="text-align: right;">
			<button type="button" name="ok" onClick="javascript: save_props(this.form)"><?php echo translate('ok'); ?></button>
			&nbsp;
			<button type="button" name="cancel" onClick="javascript: popup_close();"><?php echo translate('cancel'); ?></button>
			</div>
		</td>
	</tr>
</table>
</form>

<?php include(dirname(__FILE__)."/footer.php"); ?>