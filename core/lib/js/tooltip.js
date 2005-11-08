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
* $Id: tooltip.js,v 1.7 2005/11/08 02:53:09 lwright Exp $
*
*/

/**
* Tooltip class. Constructor
*
* @return null
* @access public
*/
function ToolTip()
{
  this.normal_font  = "10px Tahoma,Arial";
  this.title_font   = "bold 12px Tahoma,Arial";
  this.title_align  = "left";
  this.normal_bg    = "#F4F4EB";
  this.title_bg     = "#594165";
  this.border       = "solid 1px #000000";
  this.normal_color = "#000000";
  this.title_color  = "#FFFFFF";
  this.showing      = false;

  this.print = tt_print;
  this.show = tt_show;
  this.hide = tt_hide;
  this.paint = tt_paint;

  this.set = tt_set;
  this.get = tt_get;

}//end ToolTip()


/**
* Add tooltip block to the page, if it is not exists.
*
* @return void
* @access public
*/
function tt_print()
{
  if (!document.getElementById("ToolBox"))
  {
	output = '<iframe scrolling="no" border="0" frameborder="0" id="hider" style="position:absolute;top:-200px;left:-110px;width:10px; height:30px;progid:DXImageTransform.Microsoft.Alpha(style=0, opacity=0)" src="about:blank"></iframe>';
	output += '<table cellspacing="0" cellpadding="0" border="0" id="ToolBox" style="font:' + this.normal_font +
			 ';border:' + this.border +
			 ';color:' + this.normal_color +
			 ';background:' + this.normal_bg +
			 ';position:absolute;top:0px;left:0px;visibility:hidden;filter:progid:DXImageTransform.Microsoft.Fade(duration=0.5)"><tr><td id="ToolBoxTitle" style="padding:0px"></td></tr><tr><td id="ToolBoxContent" style="padding:2px"></td></tr></table>';
	if (document.body.insertAdjacentHTML) document.body.insertAdjacentHTML('beforeEnd', output);
	else document.body.innerHTML += output;
  }

}//end tt_print()


/**
* Attach tooltip to corresponding object and make it visible.
*
* @param	obj	HTML object which position we would like to know
*
* @return void
* @access private
*/
function findPosX(obj)
{
	var curleft = 0;
	if (obj.offsetParent)
	{
		while (obj.offsetParent)
		{
			curleft += obj.offsetLeft
			obj = obj.offsetParent;
		}
	}
	else if (obj.x)
		curleft += obj.x;
	return curleft;

}//end findPosX()


/**
* Attach tooltip to corresponding object and make it visible.
*
* @param	obj	HTML object which position we would like to know
*
* @return void
* @access private
*/
function findPosY(obj)
{
	var curtop = 0;
	if (obj.offsetParent)
	{
		while (obj.offsetParent)
		{
			curtop += obj.offsetTop
			obj = obj.offsetParent;
		}
	}
	else if (obj.y)
		curtop += obj.y;
	return curtop;

}//end findPosY()


/**
* Attach tooltip to corresponding object and make it visible.
*
* @param	obj	HTML object which will be base for tooltip
* @param	text	content of the tooltip (could be HTML)
* @param	title	title of the tooltip [optional]
*
* @return
* @access
*/
function tt_show(obj, text, title, close_button)
{
  if (obj == null) return;

  this.print();

  var top = findPosY(obj);
  top += obj.offsetHeight;
  var left = findPosX(obj);
  this.paint(top, left, text, title, close_button);

  this.showing = true;

}//end tt_show()


/**
* Make tooltip invisible.
*
* @return void
* @access public
*/
function tt_hide()
{
  var tool_box = document.getElementById("ToolBox");

  if (tool_box.filters) tool_box.filters[0].Apply();
  tool_box.style.visibility 	= "hidden";
  document.getElementById("hider").style.visibility = "hidden";
  if (tool_box.filters) tool_box.filters[0].Play();

  this.showing = false;

}//end tt_hide()


/**
* Change all tooltip attributes and repaint tooltip.
*
* @param	top	top position of the tooltip
* @param	left	left position of the tooltip
* @param	text	content of the tooltip (could be HTML)
* @param	title	title of the tooltip [optional]
*
* @return
* @access
*/
function tt_paint(top, left, text, title, close_button)
{
  var tool_box = document.getElementById("ToolBox");

  if (tool_box.filters) tool_box.filters[0].Apply();

  if ((typeof(title) != "undefined" && title != "") || (typeof(top) == "undefined"))
  {
	document.getElementById("ToolBoxTitle").style.textAlign = this.title_align;
	document.getElementById("ToolBoxTitle").style.font = this.title_font;
	document.getElementById("ToolBoxTitle").style.background = this.title_bg;
	document.getElementById("ToolBoxTitle").style.color = this.title_color;
	document.getElementById("ToolBoxTitle").style.padding = "2px";
	if (typeof(title) != "undefined") document.getElementById("ToolBoxTitle").innerHTML = unescape(title);

	if (closeElement = document.getElementById("ToolBoxClose")) {
		closeElement.parentNode.removeChild(closeElement);
	}

	if (typeof close_button != 'undefined') {
		closeElement = document.createElement('th');
		closeElement.style.textAlign = 'right';
		closeElement.style.padding = '2px';
		closeElement.style.backgroundColor = this.title_bg;
		closeElement.id = 'ToolBoxClose';

		if (close_button == true) {
			close_button = 'X';
		}

		closeElement.innerHTML = '<a href="#" style="text-decoration: none; color: ' + this.title_color + '" onclick="tooltip.hide(); return false;">' + close_button + '</a>';
		document.getElementById("ToolBoxTitle").parentNode.appendChild(closeElement);
		document.getElementById("ToolBoxContent").colSpan = 2;
	}
  }
  else
  {
	document.getElementById("ToolBoxTitle").style.background = this.normal_bg;
	document.getElementById("ToolBoxTitle").style.padding = "0px";
	document.getElementById("ToolBoxTitle").innerHTML = "";
  }
  if ((typeof(text) != "undefined" && text != "") || (typeof(top) == "undefined"))
  {
	if(typeof(text) != "undefined") document.getElementById("ToolBoxContent").innerHTML = unescape(text);
	document.getElementById("ToolBoxContent").style.padding = "2px";
	document.getElementById("ToolBoxContent").style.background = this.normal_bg;
  }
  else
  {
	document.getElementById("ToolBoxContent").innerHTML = "";
	document.getElementById("ToolBoxContent").style.padding = "0px";
	document.getElementById("ToolBoxContent").style.background = this.title_bg;
  }

  if ((typeof(top) != "undefined") && (typeof(left) != "undefined"))
  {
	var win_width = (window.innerWidth)?window.innerWidth:document.body.offsetWidth;

	if (tool_box.offsetWidth + left > win_width) left -= (tool_box.offsetWidth + left) - win_width;

	tool_box.style.top 		= top + "px";
	tool_box.style.left		= left + "px";

	if (window.event)
	{
	  var hider = document.getElementById("hider");
	  hider.style.top = top;
	  hider.style.left = left;
	  hider.style.width = tool_box.offsetWidth;
	  hider.style.height = tool_box.offsetHeight;
	  hider.style.visibility = "visible";
	}
  }
  tool_box.bgColor 		= this.background;
  tool_box.style.font 		= this.normal_font;
  tool_box.style.color 		= this.color;
  tool_box.style.border 	= this.border;
  tool_box.style.background 	= this.normal_bg;
  tool_box.style.visibility 	= "visible";

  if (tool_box.filters) tool_box.filters[0].Play();

}//end tt_paint()


/**
* Return value of the object's attribute.
*
* @param	attr	attribute name
*
* @return string
* @access public
*/
function tt_get(attr)
{
  if (typeof(eval("this." + attr)) == "undefined") return false;
  return eval("this." + attr);

}//end tt_get()


/**
* Set value of the object's attribute.
*
* @param	attr	attribute name
* @param	value	new value of the attribute
*
* @return
* @access
*/
function tt_set(attr, value)
{
  if (typeof(eval("this." + attr)) == "undefined") return false;
  eval("this." + attr + " = '" + value + "'");
  if (document.getElementById("ToolBox").style.visibility == "visible") this.paint();

}//end tt_set()


//default tooltip object for all pages
var tooltip = new ToolTip();
