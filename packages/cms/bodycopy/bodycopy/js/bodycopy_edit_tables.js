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
* $Id: bodycopy_edit_tables.js,v 1.7 2003/11/26 00:51:25 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

function bodycopy_insert_table(bodycopy_name, tableid, before) {
	bodycopy_current_edit["bodycopy_name"]          = bodycopy_name;
	bodycopy_current_edit["data"]                   = new Object();
	bodycopy_current_edit["data"]["containerid"]    = tableid;
	bodycopy_current_edit["data"]["container_type"] = 'table';
	bodycopy_current_edit["data"]["before"]         = before;
	bodycopy_show_popup("insert_table.php", 300, 220);
}// end bodycopy_insert_table()

function bodycopy_save_insert_table(cols, rows, attributes) {
	bodycopy_current_edit["data"]["num_cols"]   = cols;
	bodycopy_current_edit["data"]["num_rows"]   = rows;
	bodycopy_current_edit["data"]["attributes"] = attributes;
	bodycopy_hide_popup();
	bodycopy_submit("insert_container", bodycopy_current_edit["bodycopy_name"], bodycopy_current_edit["data"]);
}// end bodycopy_save_insert_table()

function bodycopy_delete_table(bodycopy_name, tableid) {
	if (confirm('Are you sure you want to delete this table?') && confirm('Really Sure? This is irreversible.')) {
		var data = new Object();
		data["containerid"] = tableid;
		bodycopy_submit("delete_container", bodycopy_name, data);
	}
}// end bodycopy_delete_table()

function bodycopy_edit_table_properties(bodycopy_name, tableid) {
	bodycopy_current_edit["bodycopy_name"]   = bodycopy_name;
	bodycopy_current_edit["data"]            = new Object();
	bodycopy_current_edit["data"]["tableid"] = tableid;
	var data = get_bodycopy_current_table_data(bodycopy_name, tableid);
	bodycopy_current_edit["data"]["attributes"] = var_unserialise(data["attributes"]);
	bodycopy_show_popup("edit_table_props.php", 320, 330);
}// end bodycopy_edit_table_properties()

function bodycopy_save_table_properties(attributes) {
	bodycopy_current_edit["data"]["attributes"] = attributes;
	bodycopy_hide_popup();
	var id = bodycopy_current_edit["bodycopy_name"] + '_table_' + bodycopy_current_edit["data"]["tableid"]; 
	bodycopy_chgColor(id);
	serialise_table(bodycopy_current_edit["bodycopy_name"], bodycopy_current_edit["data"], bodycopy_current_edit["data"]["tableid"],null,null);
}// end bodycopy_edit_table_properties()

function bodycopy_insert_table_col(bodycopy_name, tableid, colid, before) {
	var data = new Object();
	data["tableid"] = tableid;
	data["colid"]   = colid;
	data["before"]  = before;
	bodycopy_submit("insert_table_column", bodycopy_name, data);
}// end bodycopy_insert_table()

function bodycopy_delete_table_col(bodycopy_name, tableid, colid) {
	if (confirm('Are you sure you want to delete this column?') && confirm('Really Sure? This is irreversible.')) {
		var data = new Object();
		data["tableid"] = tableid;
		data["colid"]   = colid;
		bodycopy_submit("delete_table_column", bodycopy_name, data);
	}
}// end bodycopy_delete_table_col()

function bodycopy_insert_table_row(bodycopy_name, tableid, rowid, before) {
	var data = new Object();
	data["tableid"] = tableid;
	data["rowid"]   = rowid;
	data["before"]  = before;
	bodycopy_submit("insert_table_row", bodycopy_name, data);
}// end bodycopy_insert_table_row()

function bodycopy_delete_table_row(bodycopy_name, tableid, rowid) {
	if (confirm('Are you sure you want to delete this table row?') && confirm('Really Sure? This is irreversible.')) {
		var data = new Object();
		data["tableid"] = tableid;
		data["rowid"]   = rowid;
		bodycopy_submit("delete_table_row", bodycopy_name, data);
	}
}// end bodycopy_delete_table_row()

function bodycopy_edit_table_row_properties(bodycopy_name, tableid, rowid) {
	bodycopy_current_edit["bodycopy_name"] = bodycopy_name;
	bodycopy_current_edit["data"] = new Object();
	bodycopy_current_edit["data"]["tableid"] = tableid;
	bodycopy_current_edit["data"]["rowid"]   = rowid;
	var data = get_bodycopy_current_table_data(bodycopy_name, tableid, rowid);
	bodycopy_current_edit["data"]["attributes"] = var_unserialise(data["attributes"]);
	bodycopy_show_popup("edit_table_row_props.php", 270, 170);
}// end bodycopy_edit_table_row_properties()

function bodycopy_save_table_row_properties(attributes) {
	bodycopy_current_edit["data"]["attributes"] = attributes;
	bodycopy_hide_popup();
	var id = bodycopy_current_edit["bodycopy_name"] + '_row_' + bodycopy_current_edit["data"]["tableid"] + '_' + bodycopy_current_edit["data"]["rowid"]; 
	bodycopy_chgColor(id);
	serialise_table(bodycopy_current_edit["bodycopy_name"], bodycopy_current_edit["data"], bodycopy_current_edit["data"]["tableid"], bodycopy_current_edit["data"]["rowid"],null);
}// end bodycopy_save_table_row_properties()

function bodycopy_edit_table_row_order(bodycopy_name, tableid) {
	bodycopy_current_edit["bodycopy_name"] = bodycopy_name;
	bodycopy_current_edit["data"] = new Object();
	bodycopy_current_edit["data"]["tableid"] = tableid;
	var table = get_bodycopy_current_table_data(bodycopy_name, tableid);
	var row_order = new Array();
	for(var i = 0; i < table["num_rows"]; i++) {
		row_order[i] = 'Row ' + i + ' : ';
	}
	bodycopy_current_edit["data"]["order_type"] = "row";
	bodycopy_current_edit["data"]["row_order"]  = row_order;
	bodycopy_show_popup("edit_table_orderer.php", 300, 300);
}// end bodycopy_edit_table_row_order()

function bodycopy_save_table_row_order(row_order) {
	bodycopy_current_edit["data"]["row_order"] = row_order;
	bodycopy_hide_popup();
	bodycopy_submit("edit_table_row_order", bodycopy_current_edit["bodycopy_name"], bodycopy_current_edit["data"]);
}// end bodycopy_save_table_row_order()

function bodycopy_edit_table_col_order(bodycopy_name, tableid) {
	bodycopy_current_edit["bodycopy_name"] = bodycopy_name;
	bodycopy_current_edit["data"] = new Object();
	bodycopy_current_edit["data"]["tableid"] = tableid;
	var table = get_bodycopy_current_table_data(bodycopy_name, tableid);
	var col_order = new Array();
	for(var i = 0; i < table["num_cols"]; i++) {
		col_order[i] = 'Column ' + i + ' : ';
	}
	bodycopy_current_edit["data"]["order_type"] = "col";
	bodycopy_current_edit["data"]["col_order"] = col_order;
	bodycopy_show_popup("edit_table_orderer.php", 300, 300);
}// end bodycopy_edit_table_col_order()

function bodycopy_save_table_col_order(col_order) {
	bodycopy_current_edit["data"]["col_order"] = col_order;
	bodycopy_hide_popup();
	bodycopy_submit("edit_table_col_order", bodycopy_current_edit["bodycopy_name"], bodycopy_current_edit["data"]);
}// end bodycopy_save_table_col_order()

function bodycopy_edit_table_cell_properties(bodycopy_name, tableid, rowid, cellid) {
	bodycopy_current_edit["bodycopy_name"] = bodycopy_name;
	bodycopy_current_edit["data"] = new Object();
	bodycopy_current_edit["data"]["tableid"] = tableid;
	bodycopy_current_edit["data"]["rowid"]   = rowid;
	bodycopy_current_edit["data"]["cellid"]  = cellid;
	bodycopy_current_edit["data"]["available_types"] = get_bodycopy_available_content_types();
	var data = get_bodycopy_current_table_data(bodycopy_name, tableid, rowid, cellid);
	bodycopy_current_edit["data"]["attributes"] = var_unserialise(data["attributes"]);
	bodycopy_show_popup("edit_table_cell_props.php", 310, 310);
}// end bodycopy_edit_table_cell_properties()

function bodycopy_save_table_cell_properties(attributes) {
	var data = get_bodycopy_current_table_data(bodycopy_current_edit["bodycopy_name"], bodycopy_current_edit["data"]["tableid"], bodycopy_current_edit["data"]["rowid"], bodycopy_current_edit["data"]["cellid"]);
	for (var key in data) { if (typeof(data[key]) == "string") { data[key] = var_unserialise(data[key]); }}
	data['attributes'] = attributes;
	bodycopy_hide_popup();
	var id = bodycopy_current_edit["bodycopy_name"] + '_cell_' + bodycopy_current_edit["data"]["tableid"] + '_' + bodycopy_current_edit["data"]["rowid"] + '_' + bodycopy_current_edit["data"]["cellid"]; 
	bodycopy_chgColor(id);
	serialise_table(bodycopy_current_edit["bodycopy_name"], data, bodycopy_current_edit["data"]["tableid"], bodycopy_current_edit["data"]["rowid"], bodycopy_current_edit["data"]["cellid"]);
}// end bodycopy_save_table_cell_properties()