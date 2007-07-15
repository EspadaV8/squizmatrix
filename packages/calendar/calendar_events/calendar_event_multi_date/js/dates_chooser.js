/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: dates_chooser.js,v 1.1 2007/07/15 23:15:38 mbrydon Exp $
*
*/

var nextRowNum = 0;

function setEventType(radioButton)
{
	var td = radioButton.parentNode.parentNode.firstChild;
	while (td !== null) {
		if (td.tagName == 'TD') {
			if (td.className == radioButton.value) {
				td.style.display = '';
			} else if (td.className != '') {
				td.style.display = 'none';
			}
		}
		td = td.nextSibling;
	}
}

function str_replace(subject, search, replace)
{
	res = subject;
	var i;
	var len = search.length;
	while (-1 != (i = res.indexOf(search))) {
		res = res.substr(0, i) + replace + res.substr(i+len);
	}
	return res;
}

function addDate()
{
	var template = document.getElementById('template');
	var newRow = template.cloneNode(true);

	var sourceSelects = template.getElementsByTagName('SELECT');
	var targetSelects = newRow.getElementsByTagName('SELECT');
	for (var i=0; i < sourceSelects.length; i++) {
		targetSelects[i].selectedIndex = sourceSelects[i].selectedIndex;
	}

	var td = newRow.firstChild;
	while (td != null) {
		if (td.tagName == 'TD') {
			td.innerHTML = str_replace(td.innerHTML, 'template', nextRowNum);
		}
		td = td.nextSibling;
	}
	newRow.id = '';
	newRow.style.display = '';
	template.parentNode.appendChild(newRow);
	nextRowNum++;
}


function deleteRow(elt)
{
	// Find the row
	var row = elt.parentNode;
	while (row.tagName != 'TR') {
		row = row.parentNode;
	}

	// Add a replacement 'type' field so that processing doesn't stop at this row
	var fieldName = row.getElementsByTagName('INPUT')[0].name;
	var form = row.parentNode;
	while (form.tagName != 'FORM') {
		form = form.parentNode;
	}
	var hiddenField = document.createElement('INPUT');
	hiddenField.type = 'hidden';
	hiddenField.name = fieldName;
	hiddenField.value = '';
	form.appendChild(hiddenField);

	// remove the row.
	row.parentNode.removeChild(row);
}
