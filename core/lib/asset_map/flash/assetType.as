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
* $Id: assetType.as,v 1.10 2003/11/26 00:51:12 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


// Create the Class
function AssetType(type_code, name, menu_path, version, instantiable, allowed_access, parent_type, edit_screens) 
{
//	trace (this._target + "::AssetType.(constructor)(" + type_code +", " + name + ", " + menu_path + ")");

	this.type_code		= type_code;
	this.name			= name;
	this.menu_path		= menu_path;
	this.version		= version;
	this.instantiable	= instantiable;
	this.allowed_access	= allowed_access;
	this.parent_type	= parent_type;
	this.edit_screens	= edit_screens;

	this.sub_types		= new Array();

}

AssetType.prototype.toString = function()
{

	var edit_opts = "{";
	for(var i = 0; i < this.edit_screens.length; i++) {
		var c = this.edit_screens[i].code_name;
		var n = this.edit_screens[i].name;
		edit_opts += "(" + c + ": " + n + ")";
	}
	edit_opts += "}";


	return "Type Code: " + this.type_code + 
			", Name: " + this.name +
			", Menu Path: " + this.menu_path +
			", Version: " + this.version + 
			", Sys Only: " + this.allowed_access + 
			", Parent Type: " + this.parent_type +
			", Edit Opts: " + edit_opts +
			", Sub Types: " + this.sub_types;
}

/**
* Whether or not this asset type can be created by the user
*
* @return boolean
*/
AssetType.prototype.createable = function()
{
	var user_type = _root.asset_manager.asset_types[_root.asset_manager.assets[_root.asset_manager.current_userid].type_code];
	return (this.instantiable && user_type.isType(this.allowed_access));
}

/**
* Returns true if this asset type or one of it's ancestors is the passed type
*
* @return boolean
*/
AssetType.prototype.isType = function(type)
{
	if (this.type_code == type) return true;
	var parent_type = this.type_code;

	while(_root.asset_manager.asset_types[parent_type].parent_type != 'asset') {
		var parent_type = _root.asset_manager.asset_types[parent_type].parent_type;
		if (parent_type == type) return true;
		// this should NEVER happen, if it does DIE
		if (_root.asset_manager.asset_types[parent_type] == undefined) return false;
	}
	return false;
}

/**
* Returns the iconID of the asset type.
*
* @return string
*/

AssetType.prototype.getIconID = function()
{
	return "mc_asset_type_" + this.type_code + "_icon";
}

