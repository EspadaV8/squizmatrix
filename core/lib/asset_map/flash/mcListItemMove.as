
// Create the Class
function mcListItemMoveClass()
{
}

// Make it inherit from MovieClip
mcListItemMoveClass.prototype = new MovieClip();

mcListItemMoveClass.prototype.getState = function()
{

	switch(this._currentframe) {
		case 1 :
			return "off";
		break;
		case 2 :
			return "on";
		break;
		default :
			return "";
	}	
}

mcListItemMoveClass.prototype.setState = function(state)
{
	switch(state) {
		case "on" :
		case "off":
			this.gotoAndStop(state);
		break;
		default :
			trace("ERROR: State '" + state + "' unknown for mcListItemMove");
	}	
}

mcListItemMoveClass.prototype.onRelease = function()
{
	if(this.getState() == "off") {
		this._parent.startMove();
		break;
	}

}// end onRelease()


Object.registerClass("mcListItemMoveID", mcListItemMoveClass);
