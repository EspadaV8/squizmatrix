
/**
* Gives all objects a clone() fn
* By default it just returns a reference to the object
* Should be overridden if the need to clone is found for an object
*/
Object.prototype.clone = function() 
{
	return this;
}

/**
* Returns true if an object equals the other obj
*
* @param Object other_obj	the object to check
*
* @return boolean
*/
Object.prototype.equals = function(other_obj) 
{
	return (this == other_obj);
}


/**
* Returns true if an object equals the other obj
*
* @param Object other_obj	the object to check
*
* @return boolean
*/
Array.prototype.equals = function(other_obj) 
{
	return (this == other_obj);
}


/**
* Recursively creates a new clone of the array
*/
Array.prototype.clone = function() 
{
	var new_arr = new Array();
	for (var i = 0; i < this.length; i++) {
		new_arr[i] = (typeof this[i] == "object") ? this[i].clone() : this[i];
	}
	return new_arr;
}

/**
* Takes an array and a value returns the first index
* in the array that matches the passed value, 
* returns null if not found
*
* @param mixed val	the value to match
*
* @return int
*/
Array.prototype.search = function (val) {

    for (var i = 0; i < this.length; i++) {
		if (typeof this[i] == "object") {
			if (this[i].equals(val)) return i;
		} else if (this[i] == val) {
			return i;
		}
    }
    return null;

}// end Array.search()

/**
* Removes the first element in the array with passed value
*
* @param mixed val	the value to match
*/
Array.prototype.remove_element = function(val) 
{

	var i = this.search(val);
	if (i != null) {
		this.splice(i, 1);
	}// end if

}// end Array.remove_element()


/**
* Returns an array of all values that are in the current array 
* but not in passed array
*
* @param Array arr
*
* @return Array
*/
Array.prototype.diff(arr) 
{
	var new_arr = new Array();
	for (var i = 0; i < this.length; i++) {
		if (arr2.search(this[i]) == null) {
			new_arr.push(this[i]);
		}
	}

	return new_arr;

}// end Array.diff()


/**
* Sorts the array then removes any duplicates from it
*
* @param Array arr
*
* @return Array
*/
Array.prototype.unique = function() {

	var old_arr = this.clone();
	var new_arr = new Array();

	old_arr.sort();
	var tmp = '';

	for(var i = 0; i < old_arr.length; i++) {
		if (old_arr[i] != tmp) {
			new_arr.push(old_arr[i]);
			tmp = old_arr[i];
		}// end if
	}// end for

	return new_arr;

}// end Array.unique()
