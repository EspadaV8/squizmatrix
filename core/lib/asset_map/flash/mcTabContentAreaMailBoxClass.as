#include "mcMailBoxHeadingClass.as"
#include "mcMailBoxClass.as"


/**
* TabContentAreaMailBox
*
* Looks after the displaying of the mail msgs
*
*/

// Create the Class
function mcTabContentAreaMailBoxClass()
{
	super();

	this.attachMovie("mcMailBoxSubHeaderID", "sub_header", 4);
	this.sub_header._y = 0;
	this.sub_header.back._width = this._width;
	
	this.sub_header.onRelease = function() {
		trace ('noo');
		getURL(_root.inbox_path, _root.url_frame);
	};


	this.attachMovie("FScrollPaneSymbol", "scroll_pane", 2);
	this.scroll_pane.setHScroll(false);
	this.scroll_pane.setVScroll(true);
	this.scroll_pane._x = 0;
	this.scroll_pane._y = this.sub_header._height;

	// Now the msgs container
	this.attachMovie("mcMailBoxID", "msgs_container", 3);

	// Attach the container on to the scroll pane
	this.scroll_pane.setScrollContent(this.msgs_container);
	this.msgs_container._x = 0;
	this.msgs_container._y = this.sub_header._y + this.sub_header._height;

	// Because the scroll pane inherits from some other place 
	// we need to manually set it up for nesting
	makeNestedMouseMovieClip(this.scroll_pane,               true, NestedMouseMovieClip.NM_ON_PRESS);
	makeNestedMouseMovieClip(this.scroll_pane.hScrollBar_mc, true, NestedMouseMovieClip.NM_ON_PRESS);
	makeNestedMouseMovieClip(this.scroll_pane.vScrollBar_mc, true, NestedMouseMovieClip.NM_ON_PRESS);

}// end constructor()

// Make it inherit from Tab Content Area
mcTabContentAreaMailBoxClass.prototype = new mcTabContentAreaClass();

/**
* Set the size of the tabs
*
* @param int	w	the width of the tabs
* @param int	h	the height of the tabs
*
*/
mcTabContentAreaMailBoxClass.prototype.setSize = function(w, h)
{
	super.setSize(w, h);
	this.sub_header.back._width = w;
	this.scroll_pane.setSize(w, h);
	this.msgs_container.refresh();

}// setSize()

/**
* Fired when we are pressed and then release
*
* @access public
*/
mcTabContentAreaMailBoxClass.prototype.onRelease = function() 
{
	return super.onRelease(); // Fucking flash see SUPER_METHOD_EG.as
}// end


Object.registerClass("mcTabContentAreaMailBoxID", mcTabContentAreaMailBoxClass);
