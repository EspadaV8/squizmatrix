 /////////////////////////////////////////////////////////////////////////////
// NOTE: the list items in this container are just stored as
// normal attributes and not in there own array (as I would have liked)
// this is due to the attachMovie() fn not being able to accept arrays
// elements as the instance name for the new movie

// Create the Class
function mcListItemContainerClass()
{
	// holds some information on all the list items, in the vertical order that they appear in the list
	// eg li_2_1_3 would be the mean this element exists here
	this.items_order = new Array();

	// the number of items that ever been created (used for depth when attaching movies)
	this.num_items = 0;

	// an array of assetid => Array(list item names)
	this.asset_list_items = new Object();

	// if an attr starts with this then it's a list item
	this.item_prefix = "li";

	// reference to the currently selected item
	this.selected_item = null;

	// some current settings/flags
	this.action      = "";

	// a temp object that can hold any run-time data
	this.tmp = new Object();

	// Set ourselves up as a listener on the asset types, so we know when they have been loaded
	_root.asset_manager.addListener(this);

	// Set ourselves up as a broadcaster
    ASBroadcaster.initialize(this);

}// end constructor

// Make it inherit from Nested Mouse Movements MovieClip
mcListItemContainerClass.prototype = new NestedMouseMovieClip(false, NestedMouseMovieClip.NM_ON_PRESS | NestedMouseMovieClip.NM_ON_RELEASE);

/**
* Event fired when the Asset Types object has finished recieving all the asset types from the server
* We can then run init()
*/
mcListItemContainerClass.prototype.onAssetTypesLoaded = function()
{
	this.init();
}

/**
* Loads the list container, by getting the root folder and showing it's kids
*
* @access public
*/
mcListItemContainerClass.prototype.init = function() 
{
	
	_root.asset_manager.loadAssets([1], this, "initLoaded", {}, true);
	
}// end init()


/**
* continuation of init() after the root folder has been loaded
*
* @access private
*/
mcListItemContainerClass.prototype.initLoaded = function(params, new_assets, old_assets) 
{
	this.showKids('1', this.item_prefix);
}// end initLoaded()


/**
* Called by the mcListItem's on the pressing of the plus button
* Calls the root getAssetKids() which gets the XML data then 
* calls displayKids() below
*
* @param int		parent_assetid	the assetid of the asset who's kids to show
* @param string		parent_name		the list item name of the parent that the kids are attached to
* @param boolean	dont_refresh	prevent the refreshing of the list
*
*/
mcListItemContainerClass.prototype.showKids = function(parent_assetid, parent_name, dont_refresh) 
{

	if (dont_refresh != true) dont_refresh = false;

	// we don't know anything about this or it's got no kids, bugger off
	if (_root.asset_manager.assets[parent_assetid] == undefined || !_root.asset_manager.assets[parent_assetid].links.length) return;

	var params = {parent_assetid: parent_assetid, parent_name: parent_name, dont_refresh: dont_refresh};
	_root.asset_manager.loadAssets(_root.asset_manager.assets[parent_assetid].getLinkAssetids(), this, "showKidsLoaded", params);

}// end showKids()


/**
* A continuation of showKids() after the assets have been loaded
*/
mcListItemContainerClass.prototype.showKidsLoaded = function(params) 
{
	var parent_assetid = params.parent_assetid;
	var parent_name	   = params.parent_name;

//	trace('Show Kids Loaded    : ' + parent_assetid + ', ' + parent_name);

	// Now create the kids list items, if they don't already exist
	for(var j = 0; j < _root.asset_manager.assets[parent_assetid].links.length; j++) {
		var linkid    = _root.asset_manager.assets[parent_assetid].links[j];
		var item_name = parent_name + "_" + linkid;
		this._createItem(parent_name, item_name, linkid);

	}// end for

	// see if we can find this parent in the list
	var parent_i = (parent_assetid > 1) ? this[parent_name].pos : -1;

	this._recurseDisplayKids(parent_assetid, parent_name, parent_i);

	if (parent_assetid > 1) {
		// Because we only want to change the collapse sign for the parent asset
		// we do it here rather than in the recusion
		 this[parent_name].setKidState("minus");
	}

	if (!params.dont_refresh) {
		this.refresh(parent_i);
	}

}// end showKidsLoaded()

/**
* Splices the items order array recursively, adding the kids of the passed parent
*
*/
mcListItemContainerClass.prototype._recurseDisplayKids = function(parent_assetid, parent_name, parent_i) 
{

	var i = parent_i + 1;

	// Now add the kids into the items order array in the correct spot
//	trace('Kids of ' + parent_assetid + ' : ' + _root.asset_manager.assets[parent_assetid].links);
	for(var j = 0; j < _root.asset_manager.assets[parent_assetid].links.length; j++) {
		var name = parent_name + "_" + _root.asset_manager.assets[parent_assetid].links[j];
		if (this[name] == undefined) continue;
		this._insertItem(i, name, (j == _root.asset_manager.assets[parent_assetid].links.length - 1));
		if (this[name].expanded()) {
			i = this._recurseDisplayKids(this[name].assetid, name, i);
		} else {
			i++;
		}
	}

	return i;

}// end _recurseDisplayKids()


/**
* Creates a list item mc
*
* @param string		parent_name		The parent list item name
* @param string		item_name		The item name for the new item
* @param int		linkid			The linkid for which this item is the minor party
*
*/
mcListItemContainerClass.prototype._createItem = function(parent_name, item_name, linkid) 
{
	if (this[item_name] == undefined) {

		this.num_items++;
		var indent = (_root.asset_manager.asset_links[linkid].majorid > 1) ? this[parent_name].indent + 1 : 0;
		var assetid = _root.asset_manager.asset_links[linkid].minorid;

		this.attachMovie("mcListItemID", item_name, this.num_items);
		this[item_name]._visible = false;

		this[item_name].setParent(parent_name);
		this[item_name].setLinkId(linkid);
		this[item_name].setAsset(_root.asset_manager.assets[assetid]);
		this[item_name].setIndent(indent);

		if (this.asset_list_items[assetid] == undefined) this.asset_list_items[assetid] = new Array();
		this.asset_list_items[assetid].push(item_name);

	}// end if

}// end _createItem()

/**
* Deletes a list item mc
*
* @param string		item_name		The item name for the new item
*
*/
mcListItemContainerClass.prototype._deleteItem = function(item_name) 
{
	if (this[item_name] == undefined) return;

	this.asset_list_items[this[item_name].assetid].remove_element(item_name);
	this._removeItem(item_name);
	this[item_name].removeMovieClip();

}// end _deleteItem()

/**
* Inserts an item into the items_order array
*
* @param int		pos			The position to insert to
* @param string		name		The name of the list item
* @param boolean	end_branch	Whether this is an end branch or not
*
*/
mcListItemContainerClass.prototype._insertItem = function(pos, name, end_branch) 
{
	this.items_order.splice(pos, 0, {name: name, branch_count: 0, end_branch: end_branch});
}// end _insertItem()

/**
* Removes an item from the items_order array
*
* @param string		item_name		The name of the list item
*
*/
mcListItemContainerClass.prototype._removeItem = function(item_name) 
{
	if (this[item_name] == undefined) return;

	// if the one we are removing is the currently selected item, un select it
	if (this.selected(this[item_name])) {
		this.unselectItem();
	}

	for(var x = 0; x < this.items_order.length; x++) {
		if (this.items_order[x].name == item_name) {
			this.items_order.splice(x, 1);
			break;
		}
	}

}// end _removeItem()

/**
* Called by the mcListItem's on the pressing of the minus button
* Hiding the children of the passed 
*
* @param int		parent_assetid	the assetid of the asset who's kids to hide
* @param string		parent_name		the list item name of the parent that the kids are attached to
* @param boolean	dont_refresh	prevent the refreshing of the list
*
*/
mcListItemContainerClass.prototype.hideKids = function(parent_assetid, parent_name, dont_refresh) 
{
	if (dont_refresh != true) dont_refresh = false;

	var parent_i = (parent_assetid > 1) ? this[parent_name].pos : -1;
	
	var num_to_remove = this._recurseHideKids(parent_assetid, parent_name);

	// Now remove the kids from the items order
	this.items_order.splice(parent_i + 1, num_to_remove);

	if (parent_assetid > 1) {
		// Because we only want to change the collapse sign for the parent asset
		// we do it here rather than in the recusion
		 this[parent_name].setKidState("plus");
	}

	if (!dont_refresh) {
		this.refresh(parent_i);
	}

}// end hideKids()

mcListItemContainerClass.prototype._recurseHideKids = function(parent_assetid, parent_name) 
{

	var num_kids = _root.asset_manager.assets[parent_assetid].links.length;
	
	// loop through all the kids and make them invisible, also recursivly remove their kids
	for(var j = 0; j < _root.asset_manager.assets[parent_assetid].links.length; j++) {
		var name = parent_name + "_" + _root.asset_manager.assets[parent_assetid].links[j];
		if (this[name] == undefined) continue;
		if (this[name].expanded()) {
			num_kids += this._recurseHideKids(this[name].assetid, name);
		}
		this[name]._visible = false;
	}

	return num_kids;

}// end _recurseHideKids()

/**
* Event fired by asset manager
*
* @param int			assetids
* @param object Asset	new_assets	The new version of the assets
* @param object Asset	old_assets	The old version of the assets
*
* @access public
*/
mcListItemContainerClass.prototype.onAssetsReload = function(assetids, new_assets, old_assets) 
{

	for(var j = 0; j < assetids.length; j++) {

		var assetid = assetids[j];

		var deletes = old_assets[assetid].links.diff(new_assets[assetid].links);
		var inserts = new_assets[assetid].links.diff(old_assets[assetid].links);

		// if it's the root folder
		if (assetid == 1) {
			this._reloadAssetListItem(assetid, this.item_prefix, -1, true, deletes, inserts, new_assets[assetid].links);

		// else it's just a normal asset
		} else {

			for(var i = 0; i < this.asset_list_items[assetid].length; i++) {
				var item_name = this.asset_list_items[assetid][i];
				// we need to the get the pos this way because the this[item_name].pos will be out 
				// of wack until we call the refresh below
				var pos  = 0; // outside for scope reasons
				for(; pos < this.items_order.length; pos++) {
					if (this.items_order[pos].name == item_name) break;
				}
				if (pos >= this.items_order.length) continue; // we didn't find it

				trace("Reload List Item : " + item_name + ", expanded : " + this[item_name].expanded());

				this._reloadAssetListItem(assetid, item_name, pos, this[item_name].expanded(), deletes, inserts, new_assets[assetid].links);

			}// end for

		}// end if

	}// end for


	// OK to make sure that every thing is in the correct order we hide everything
	// but DON'T refresh, then we show every thing again will all expanded assets 
	// being displayed
	this.hideKids(1, this.item_prefix, true);
	this.showKids(1, this.item_prefix);

	// Now if there has been a asset move and we know where to select, then do so
	if (this.tmp.move_asset.new_select_name != undefined) {
		this.select(this[this.tmp.move_asset.new_select_name]);
		delete this.tmp.move_asset.new_select_name;
	}

}// end onAssetReload()


/**
* Called by onAssetReload() to reload a single asset list item
*
* @param int		assetid
* @param string		item_name	the list item we are reloading
* @param int		pos			the current pos of the list item
* @param boolean	expanded	whether this list item is expanded or not
* @param Array		deletes		linkids that have been removed
* @param Array		inserts		linkids that have been added
* @param Array		all_links	all the linkids for the asset
*
* @access private
*/
mcListItemContainerClass.prototype._reloadAssetListItem = function(assetid, item_name, pos, expanded, deletes, inserts, all_kids) 
{

	// delete any child items that aren't needed any more
	for(var j = 0; j < deletes.length; j++) {
		var kid_name = item_name + "_" + deletes[j];
		this._deleteItem(kid_name);
	}// end for

	// the reset of this is only relevant if the item is expanded
	if (expanded) {
		trace("EXPANDED -> " + all_kids);

		// insert any child items that have just been added
		for(var j = 0; j < inserts.length; j++) {
			var kid_name = item_name + "_" + inserts[j];
			this._createItem(item_name, kid_name, inserts[j]);
			this._insertItem(pos + 1, kid_name, false);

		}// end for

	}// end if expanded

}// end _reloadAssetListItem()


/**
* Refreshes the display of this container's items, and inform the list container (our parent)
* of the refresh
*
* @param int start_i   the index in the items_order array to start the refresh from 
*
* @access public
*/
mcListItemContainerClass.prototype.refresh = function(start_i) 
{
	this.refreshDisplay(start_i);
	this._parent.refreshDisplay();
}
	
/**
* Refreshes the display of the items, from a certain position onwards
* don't inform our parent
*
* @param int start_i   the index in the items_order array to start the refresh from 
*
* @access public
*/
mcListItemContainerClass.prototype.refreshDisplay = function(start_i) 
{

	if (start_i == undefined || start_i < 0) start_i = 0;

	// now cycle through every item from the parent down and reset their positions
	var branch_count = (start_i >= 0) ? this.items_order[start_i].branch_count : 0;
	for(var i = start_i; i < this.items_order.length; i++) {
		// set for future use
		this.items_order[i].branch_count = branch_count;
		this[this.items_order[i].name].setPos(i);
		this[this.items_order[i].name]._visible = true;

		// if we have come across an end branch,
		// and if we aren't at the last item
		// and if this element has no kids, then we add the branch gap
		if (this.items_order[i].end_branch) {
			if (i < this.items_order.length - 1) {
				var this_indent = this[this.items_order[i].name].indent;
				var next_indent = this[this.items_order[i + 1].name].indent;
				if (this_indent != (next_indent - 1)) {
					branch_count++;
				}
			}
		}
	}// end for

}// end refreshDisplay()

/**
* Returns the position of the item in the items_order array that is under the 
* x,y co-ordinates passed into the fn. These co-ords MUST be relative to
* this containers axis, not the Stage's
*
* Returns -1 if the y co-ord is before any clips 
* and the length of the items array if after all clips
*
* @param float x 
* @param float y
* @param boolean [bleed_gaps] default=false, if the co-ords are over a branch gap returns the pos above the gap
*
* @return int | Object
* @access public
*/
mcListItemContainerClass.prototype.getItemPos = function(x, y, bleed_gaps) 
{

	if (bleed_gaps != true) bleed_gaps = false;

	var pos    = -1;
	var in_gap = false;

	if (y > 0) {

		var last_pos = this[this.items_order[this.items_order.length - 1].name]._y + _root.LIST_ITEM_POS_INCREMENT + ((bleed_gaps) ? _root.LIST_ITEM_END_BRANCH_GAP : 0);

		// if we are past the last item in the list return the length of the items_order array
		if (y > last_pos) {
			pos = this.items_order.length;
		} else {

			// OK the biggest problem we have here is the bloody end branch gaps
			// so what we can do is find the maximum position number that these co-ords
			// would produce by ignoring the gaps
			var max_pos = Math.floor(y / _root.LIST_ITEM_POS_INCREMENT);

			// make sure the number is valid
			if (max_pos < 0) {
				return -1;
			} else if (max_pos >= this.items_order.length) {
				max_pos = this.items_order.length - 1;
			} 

			// Now we get a minimum position number, by using the branch count at the max pos
			var min_pos = max_pos - this.items_order[max_pos].branch_count;

			while(min_pos <= max_pos) {

				var i = min_pos + Math.round((max_pos - min_pos) / 2);

				var start_y = this[this.items_order[i].name]._y;
				var end_y   = this[this.items_order[i].name]._y  + _root.LIST_ITEM_POS_INCREMENT + ((bleed_gaps) ? _root.LIST_ITEM_END_BRANCH_GAP : 0);

				// if the mouse is before this element make the one above us the new max
				if (y < start_y) {
					max_pos = i - 1;

				// if the mouse is after this element make the one below us the new min
				} else if (y > end_y) {
					min_pos = i + 1;

				// else mouse is in this element 
				} else {
					pos    = i;
					if (bleed_gaps) {
						// we are in the gap if we are past the end of the item proper
						in_gap = (y > (this[this.items_order[i].name]._y  + _root.LIST_ITEM_POS_INCREMENT));
					}
					break;

				}// end if

			}//end while

		}// end if

	}// end if

	return (bleed_gaps) ? {pos: pos, in_gap: in_gap} : pos;

}// end getItemPos()


/**
* Returns the assetid and the position in it's kids array that a pos in the items_order array refers to
*
* @param int	pos
* @param string	where	where in relation to pos are we talking about (before | after | child)
*
* @return Object
* @access public
*/
mcListItemContainerClass.prototype.posToAssetInfo = function(pos, where) 
{

	trace('posToAssetInfo(' + pos + ', ' + where + ')');
	trace('Items Order : ' + this.items_order[pos].name);
	trace('Parent  : ' + this[this.items_order[pos].name].parent_item_name);

	var pos_item = this[this.items_order[pos].name];
	var parent_assetid = 0;
	var relative_pos   = 0;

	// if we are after a child pos, then this is easy, the parent is the current item 
	// and the relative pos is the top of the list
	if (where == "child") {
		parent_assetid = pos_item.assetid;
		relative_pos   = 0;

	} else {

		parent_assetid = pos_item.getParentAssetid();

		// if we are after this pos (and this pos is an end branch)
		// then our pos is the length of the kids array
		if (where == "after" && this.items_order[pos].end_branch) {
			relative_pos = _root.asset_manager.assets[parent_assetid].links.length;

		// else we must cycle through our parents kids until we find ourselves
		} else {
			for(var j = 0; j < _root.asset_manager.assets[parent_assetid].links.length; j++) {
				if (_root.asset_manager.assets[parent_assetid].links[j] == pos_item.linkid) {
					relative_pos = j;
					break;
				}

			}// end for

		}// end if

	}// end if

	trace('Parent Assetid : ' + parent_assetid);
	trace('Relative Pos   : ' + relative_pos);

	return {parent_assetid: parent_assetid, pos: relative_pos};

}// end posToAssetInfo()


mcListItemContainerClass.prototype.onPress = function() 
{

	// check if something else is modal
	if (_root.system_events.inModal(this)) return true;

	if (this.move_indicator_on) return true;

	switch(this.action) {
		case 'move' : 
			// do nothing here, wait for release
			return true;
		break;

		// OK we ain't doing anything at the moment let's check to see if we are over an
		// item and deal with any actions needed
		default :

			super.onPress();
			return true;


	}// end switch

}// end onPress()

mcListItemContainerClass.prototype.showActionsBar = function() 
{
	clearInterval(this.actions_bar_interval);
	this.actions_bar_interval = null;
	this.actions_bar.show(this.selected_item.assetid, this._xmouse, this._ymouse);

}// end showActionsBar()

mcListItemContainerClass.prototype.onRelease = function() 
{
	// check if something else is modal
	if (_root.system_events.inModal(this)) return true;
	_root.system_events.screenPress(this);

	if (this.move_indicator_on) {
		this.endMoveIndicator();
		return true;
	}

	if (this.actions_bar_interval) {
		trace("CLEAR INTERVAL : " + this.actions_bar_interval);
		clearInterval(this.actions_bar_interval);
		this.actions_bar_interval = null;
	}
	if (this.actions_bar._visible) this.actions_bar.hide();



	switch(this.action) {

		default :
			// if there is nothing selected then there is nothing for us to do
			if (this.selected_item == null) return;

			switch(this.selected_item.getMouseButton()) {
				case "kids" : 
					switch(this.selected_item.getKidState()) {
						case "plus" :
							//	Expand Branch
							this.showKids(this.selected_item.assetid, this.selected_item._name);
						break;
						case "minus" :
							//	Collapse Branch
							this.hideKids(this.selected_item.assetid, this.selected_item._name);
						break;
					}
				break;

				case "move" :
					this.itemStartMove();
				break;
				
			}// end switch

	}// end switch

	return true;

}// end onRelease();

mcListItemContainerClass.prototype.selectItem = function(item) 
{
	// if we in any action we don't want to select anything
	if (this.action != "") return false;

	if (this.selected_item == item) return true;

	if (this.selected_item != null) {
		this.unselectItem();
	}

	this.selected_item = item;
	this.selected_item.select();

	this.broadcastMessage("onListItemSelection", this.selected_item.assetid);

	return true;

}

mcListItemContainerClass.prototype.unselectItem = function() 
{
	this.selected_item.unselect();
	this.selected_item = null;
	this.broadcastMessage("onListItemUnSelection");
}

mcListItemContainerClass.prototype.selected = function(item) 
{
	return (this.selected_item == item);
}


mcListItemContainerClass.prototype.itemStartMove = function() 
{
	if (this.action != "") return false;

	if (this.startMoveIndicator("itemEndMove")) {
//		trace("Start Item Move");
		this.action = "move";
		// move to top of layers
		this.selected_item.setMoveState("on");
	}

}// end itemStartMove()

mcListItemContainerClass.prototype.itemEndMove = function(pos, where) 
{
	if (this.action != "move") return;

//	trace("End Item Move");

	var do_move = false;

	// if we actually moved
	if (pos != this.selected_item.pos) {
		// if the pos is the item after the selected 
		// then only move if the indicator is after that item
		if (pos == this.selected_item.pos + 1) {
			do_move = (where == "after" || where == "child");
		} else {
			do_move = true;
		}
	}// end if

	if (do_move) {

		var moved_to_item = this[this.items_order[pos].name];

		// if the indicator was before the position
		if (where == "before") {
			// AND if we are staying under the same parent 
			if (this.selected_item.getParentAssetid() == moved_to_item.getParentAssetid()) {
				var parent_asset = _root.asset_manager.assets[this.selected_item.getParentAssetid()];
				// AND if we are moving the asset down the list
				if (parent_asset.linkPos(this.selected_item.linkid) < parent_asset.linkPos(moved_to_item.linkid)) {
					// then we need to decrement the pos - see me for a demo of why (BCR)
					pos -= 1;
				}
			}
		}

		var params = new Object();
		params.moved_info           = this.posToAssetInfo(pos, where);
		params.moved_to_parent_name = moved_to_item.parent_item_name;
		
		trace("Move to Pos : " + pos + ", where : " + where);
		trace("Move Info   : " + params.moved_info.parent_assetid + ", pos : " + params.moved_info.pos);


		// if the parent has changed during the move then we need to ask what we want done
		if (this.selected_item.getParentAssetid() != moved_to_item.getParentAssetid()) {
			_root.options_box.init("Move Type", "Are you moving this asset, or creating a new link ?", this, "itemMoveConfirm", params);
			_root.options_box.addOption("new link",   "New Link");
			_root.options_box.addOption("move asset", "Moving");
			_root.options_box.show();

		// otherwise just move it
		} else {
			this.itemMoveConfirm("move asset", params);

		}// end if

	} else {
		// same as hitting cancel
		this.itemMoveConfirm(null, {});
	}// end if

}// end itemEndMove()


mcListItemContainerClass.prototype.itemMoveConfirm = function(move_type, params) 
{
	if (this.action != "move") return;

	// if they didn't hit cancel
	if (move_type != null) { 
		trace("End Item Move : move_type -> " + move_type);

		var xml = new XML();
		var cmd_elem = xml.createElement("command");
		cmd_elem.attributes.action              = move_type;
		cmd_elem.attributes.from_parent_assetid = this.selected_item.getParentAssetid();
		cmd_elem.attributes.linkid              = this.selected_item.linkid;
		cmd_elem.attributes.to_parent_assetid   = params.moved_info.parent_assetid;
		cmd_elem.attributes.to_parent_pos       = params.moved_info.pos;
		xml.appendChild(cmd_elem);

		trace(xml);

		// start the loading process
		var exec_identifier = _root.server_exec.init_exec(xml, this, "xmlMoveItem", "success");
		if (this.tmp.move_asset == undefined) this.tmp.move_asset = new Object();
		this.tmp.move_asset[exec_identifier] = params;
		_root.server_exec.exec(exec_identifier, ((move_type == "move asset") ? "Moving Asset" : "Creating new link"));

	}

	this.selected_item.setMoveState("off");
	this.action = "";

}// end itemMoveConfirm()


mcListItemContainerClass.prototype.xmlMoveItem = function(xml, exec_identifier)
{

	var select_linkid = xml.firstChild.attributes.linkid;

	// OK let's save the pos of that we were dragged to so that we can select the correct
	// list item later after the parents have been reloaded
	this.tmp.move_asset.new_select_name = this.tmp.move_asset[exec_identifier].moved_to_parent_name + "_" + select_linkid;

	trace("#### New Select Name : " + this.tmp.move_asset.new_select_name);

	// get the asset manager to reload the assets
	_root.asset_manager.reloadAssets([this.selected_item.getParentAssetid(), this.tmp.move_asset[exec_identifier].moved_info.parent_assetid]);

}// end xmlMoveItem()


mcListItemContainerClass.prototype.startMoveIndicator = function(on_end_fn) 
{
	if (this.move_indicator_on) return false;
	// attempt to get the modal status
	if (!_root.system_events.startModal(this)) return false;

	this.move_indicator_on = true;


	this.tmp.move_indicator = {on_end_fn: on_end_fn, pos: -1, where: ""};
	// move to top of layers
	this.move_indicator.swapDepths(this.num_items + 1);

	return true;

}// end startMoveIndicator()

mcListItemContainerClass.prototype.endMoveIndicator = function() 
{
	if (!this.move_indicator_on) return;
	_root.system_events.stopModal(this);

	// call back to the end fn for whoever called us
	this[this.tmp.move_indicator.on_end_fn](this.tmp.move_indicator.pos, this.tmp.move_indicator.where);

	// clear the move indicator
	this.move_indicator._visible = false;
	this.move_indicator.swapDepths(-1);

	// Re-Show the menus
	_root.menu_container.show();

	this.move_indicator_on = false;

}// end endMoveIndicator()


mcListItemContainerClass.prototype.onMouseMove = function() 
{
	if (this.move_indicator_on) {

		var xm = this._xmouse;
		var ym = this._ymouse;

		var pos = this.getItemPos(xm, ym, true);
		// make sure the number is valid
		if (pos.pos > this.items_order.length) pos.pos = this.items_order.length;
		else if (pos.pos < 0) pos.pos = 0;

		// if we are past the end of the list then we are really at the last pos, in the gap
		if (pos.pos == this.items_order.length) {
			pos.pos    = this.items_order.length - 1;
			pos.in_gap = true;
		}

		// if we are past the end of the list, improvise
		var item_name = this.items_order[pos.pos].name;

		this.tmp.move_indicator.pos    = pos.pos;

		if (pos.in_gap) {
			this.move_indicator.gotoAndStop("normal");
			this.move_indicator._x  = this[item_name]._x;
			this.move_indicator._y  = this[item_name]._y + _root.LIST_ITEM_POS_INCREMENT;
			this.tmp.move_indicator.where = "after";

		} else {

			var percentile = ((ym - this[item_name]._y) / _root.LIST_ITEM_POS_INCREMENT);

			if (percentile < 0.45) {
				this.move_indicator.gotoAndStop("normal");
				this.move_indicator._x  = this[item_name]._x;
				this.move_indicator._y  = this[item_name]._y;
				this.tmp.move_indicator.where = "before";

			} else {
				this.move_indicator.gotoAndStop("new_child");
				this.move_indicator._x  = this[item_name]._x + _root.LIST_ITEM_INDENT_SPACE;
				this.move_indicator._y  = this[item_name]._y + _root.LIST_ITEM_POS_INCREMENT;
				this.tmp.move_indicator.where = "child";

			}
		}

		this.move_indicator._visible = true;

	}// end if action

	return true;

}// end onMouseMove()


/**
* When an item is pressed in the menu, we need to check if we should take care of 
* the action or not
*
* @param string action	the action to perform
* @param string info	additional information needed for action
*
*/
mcListItemContainerClass.prototype.onMenuItemPress = function(action, info) 
{

	switch(action) {
		case "add" :
//			trace('Add ' + info);
			if (this.startMoveIndicator("processAddAsset")) {
				this.action = "add_asset";
				this.tmp.exec_action = {type_code: info};
			} else {
				_root.dialog_box.show("Unable to aquire the Move Indicator", "");
			}

		break;

	}// end switch

}// end onMenuItemPress()


/**
* Attempt to add the asset
*
* @param int		pos	the	position in the item_order array that the move indicator came to it's final rest
* @param boolean	in_gap	whether the indicator finished up in the gap at the end of the list
*
*/
mcListItemContainerClass.prototype.processAddAsset = function(pos, where) 
{

	trace("Add an asset of type '" + this.tmp.exec_action.type_code + "' to pos " + pos + ". Where ? " + where);
	var info = this.posToAssetInfo(pos, where);

	var xml = new XML();
	var cmd_elem = xml.createElement("command");
	cmd_elem.attributes.action  = "get url";
	cmd_elem.attributes.cmd     = "add";
	cmd_elem.attributes.type_code      = this.tmp.exec_action.type_code;
	cmd_elem.attributes.parent_assetid = info.parent_assetid;
	cmd_elem.attributes.pos            = info.pos;
	xml.appendChild(cmd_elem);

	trace(xml);

	// start the loading process
	var exec_identifier = _root.server_exec.init_exec(xml, this, "xmlGotoUrl", "url");
	_root.server_exec.exec(exec_identifier, "Sending Request");

	this.action = "";

}// end processAddAsset()



/**
* call back fn used by the execAction*() fns to after they sedn their urls requests to the server
*
* @param object xml	the xml object returned
*
*/
mcListItemContainerClass.prototype.xmlGotoUrl = function(xml, exec_identifier)
{
	var frame = xml.firstChild.attributes.frame;
	var link  = xml.firstChild.firstChild.nodeValue;

	trace("getURL(" + link + ", " + frame + ");");
	getURL(link, frame);

}// end xmlGotoUrl()


Object.registerClass("mcListItemContainerID", mcListItemContainerClass);
