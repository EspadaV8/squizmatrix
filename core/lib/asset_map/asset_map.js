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
* $Id: asset_map.js,v 1.12 2005/02/18 05:30:32 mmcintyre Exp $
*
*/


var SQ_REFRESH_ASSETIDS = "";


/**
* Initalises the asset map
*
* Checks the browser flavour and version and make some desicions based on that information
* about whether communication should be stable between the javascript and the asset map
*/
function init_asset_map()
{
	if (matches = navigator.userAgent.match(/MSIE ([0-9.]+)/)) {

		if (matches[1] < '6.0') {
			alert('You need to use Internet Explorer 6.0 or above for the communication between the Asset Map and the Javascript');
		}

	} else {

		var firebird_re = /(Firebird)\/([0-9.]+)/;
		var safari_re   = /(Safari)\/([0-9.]+)/;
		var moz_re      = /^Mozilla\/5\.0.*rv:([^)]+)\)/;

		if ((matches = navigator.userAgent.match(firebird_re)) || (matches = navigator.userAgent.match(safari_re)) || (matches = navigator.userAgent.match(moz_re))) {
			if (matches[1] == 'Firebird' && matches[2] < '0.6.1') {
				alert('You need to use Firebird 0.6.1 or above for the communication between the Asset Map and the Javascript');
			} else if (matches[1] == 'Safari' && parseFloat(matches[2]) < 125.1) {
				// Safari 1.2.1 reports as build number '125.1'
				alert('You need to use Safari 1.2.1 (build 125.1) or above for the communication between the Asset Map and the Javascript');
			} else if (matches[1] == 'Mozilla' && matches[2] < '1.4') {
				alert('You need to use Mozilla 1.4 or above for the communication between the Asset Map and the Javascript');
			}
		// we don't know about this browser, ah well may as well give it a go ...
		} else {
			alert('You are using an untested browser there is no guarantee that the communication between the Asset Map and the Javascript will be successful');
		}

	}//end if

}//end init_asset_map()


/**
* Opens a new window with a HIPO job in it
*
*/
function open_hipo(url)
{
	window.open(url, 'hipo_job', 'width=650,height=400,scrollbars=1,toolbar=0,menubar=0,location=0,resizable=1');

}//end open_hipo()


/**
* Returns the java applet object
*
* @return &object the java applet object
*/
function get_java_applet_object()
{
	return document.sq_asset_map;

}//end get_java_applet_object()


/**
* Add the passed list of assetids to the array of IDs to refresh
*
*/
function reload_assets(assetids)
{
	if (SQ_REFRESH_ASSETIDS != "") {
		SQ_REFRESH_ASSETIDS += ",";
	}
	SQ_REFRESH_ASSETIDS += assetids;

}//end reload_assets()


/*
* Add the passed assetid to the array of IDs to refresh
*
*/
function reload_asset(assetid)
{
	if (SQ_REFRESH_ASSETIDS != "") {
		SQ_REFRESH_ASSETIDS += ",";
	}
	SQ_REFRESH_ASSETIDS += assetid;

}//end reload_asset()


/**
* Makes an extenal call to java
*
* @param object		$asset_mapObj	the java applet
* @param string		$type			the type of request
* @param String		$command		the command
* @param Array		$params			the params to pass to java
*/
function jsToJavaCall(asset_mapObj, type, command, params)
{
	
	var params_str = '';
	for (var i = 0; i < params.length; i++) {
		params_str += params[i];
		if (params.length - 1 != i) {
			params_str += ',';
		}
	}
	asset_mapObj.jsToJavaCall(type, command, params_str);

}//end jsToJavaCall();




  //////////////////////////////////
 //   Asset Finder Functions     //
//////////////////////////////////

var ASSET_FINDER_FIELD_NAME = null;
var ASSET_FINDER_FIELD_SAFE_NAME = null;
var ASSET_FINDER_DONE_FUNCTION = null;
var ASSET_FINDER_OBJ = null;

/**
* set the finder object that initiated the asset finder
*
* @param finder the finder that initiated the asset finder
*/
function set_finder(finder)
{
	ASSET_FINDER_OBJ = finder;

}//end set_finder()


/**
* Activated by the pressing of the "Change" button to start the asset finder mode in the flash menu
*
* @param string	$name			the name of the hidden field
* @param string	$safe_name		the name prefix for all the other form elements associated with the
* @param string	$type_codes		the typecodes seperated by a pipe
*
* @access public
*/
function asset_finder_change_btn_press(name, safe_name, type_codes, done_fn)
{
	ASSET_FINDER_DONE_FUNCTION = done_fn;

	if (ASSET_FINDER_FIELD_NAME != null && ASSET_FINDER_FIELD_NAME != name) {
		alert('The asset finder is currently in use');
		return;
	}

	// no name ? we must be starting the asset finder
	if (ASSET_FINDER_FIELD_NAME == null) {
		ASSET_FINDER_FIELD_NAME = name;
		ASSET_FINDER_FIELD_SAFE_NAME = safe_name;

		asset_finder_start('asset_finder_done', type_codes);

		ASSET_FINDER_OBJ.set_button_value(ASSET_FINDER_FIELD_SAFE_NAME + '_change_btn', 'Cancel');

	// else we must be cancelling the asset finder
	} else {
		asset_finder_cancel();
		ASSET_FINDER_OBJ.set_button_value(ASSET_FINDER_FIELD_SAFE_NAME + '_change_btn', 'Change');
		ASSET_FINDER_FIELD_NAME = null;
		ASSET_FINDER_FIELD_SAFE_NAME = null;
	}

}//end asset_finder_change_btn_press()


/**
* Call-back fns that stops the asset finder
*
* @param Array params the params array
* @param string label the label to give this asset
* @param string url the url of this asset
*
* @access public
*/
function asset_finder_done(params, label, url)
{
	var win = ASSET_FINDER_OBJ.window;
	win.focus();

	if (ASSET_FINDER_FIELD_NAME == null) return;

	var assetid = params;
	// we are in java, params is actually the assetid
	if (assetid != -1) {
		ASSET_FINDER_OBJ.set_hidden_field(ASSET_FINDER_FIELD_NAME + '[assetid]',assetid);
		ASSET_FINDER_OBJ.set_hidden_field(ASSET_FINDER_FIELD_NAME + '[url]', url);
		ASSET_FINDER_OBJ.set_text_field(ASSET_FINDER_FIELD_SAFE_NAME + '_label', (assetid == 0) ? '' : label + ' (Id : #' + assetid + ')');

	}

	ASSET_FINDER_OBJ.set_button_value(ASSET_FINDER_FIELD_SAFE_NAME + '_change_btn', 'Change');
	ASSET_FINDER_FIELD_NAME = null;
	ASSET_FINDER_FIELD_SAFE_NAME = null;
	if (ASSET_FINDER_DONE_FUNCTION !== null) ASSET_FINDER_DONE_FUNCTION();

}//end asset_finder_done()


/**
* Starts the asset finder and lets the asset map know that we are now in asset finder mode
*
* @param String		fn				the function to call when done
* @param String		type_codes		the type codes to restrict the asset finder to
*
* @access public
*/
var ASSET_FINDER_CALL_BACK = null;
function asset_finder_start(fn, type_codes)
{
	var params = type_codes.split('|');
	var asset_mapObj = get_java_applet_object();
	jsToJavaCall(asset_mapObj, 'asset_finder', 'assetFinderStarted', params);

}//end asset_finder_start()


/**
* Alerts the asset map that asset finder mode has been canceled
*
* @access public
*/
function asset_finder_cancel() {

	var asset_mapObj = get_java_applet_object();
	params = new Array();
	jsToJavaCall(asset_mapObj, 'asset_finder', 'assetFinderStopped', params);

}//end asset_finder_cancel()


/**
* Activated by the pressing of the "Clear" button
*
* @param string	$name			the name of the hidden field
* @param string	$safe_name		the name prefix for all the other form elements associated with the
*
* @access public
*/
function asset_finder_clear_btn_press(name, safe_name)
{
	ASSET_FINDER_OBJ.set_hidden_field(name + '[assetid]', 0);
	ASSET_FINDER_OBJ.set_hidden_field(name + '[url]', '');
	ASSET_FINDER_OBJ.set_text_field(safe_name + '_label', '');

}//end asset_finder_clear_btn_press()


/**
* Activated by the pressing of the "Reset" button
*
* @param string	$name			the name of the hidden field
* @param string	$safe_name		the name prefix for all the other form elements associated with the
*
* @access public
*/
function asset_finder_reset_btn_press(name, safe_name, assetid, label)
{
	ASSET_FINDER_OBJ.set_hidden_field(name + '[assetid]', assetid);
	ASSET_FINDER_OBJ.set_text_field(safe_name + '_label', label);

}//end asset_finder_reset_btn_press()


/**
* Activated by on an unload event to cancel the asset finder if we are currently looking
*
* @access public
*/
function asset_finder_onunload()
{
	// got a name ? we must be finding assets, cancel it
	if (ASSET_FINDER_FIELD_NAME != null) {
		asset_finder_cancel();
	}

}//end asset_finder_onunload()


ASSET_FINDER_OTHER_ONUNLOAD = (window.onunload) ? window.onunload : new Function;
window.onunload = asset_finder_onunload;

