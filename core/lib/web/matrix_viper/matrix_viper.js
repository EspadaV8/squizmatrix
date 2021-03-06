/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
*
*/

/**
 * Matrix_Viper
 *
 * Purpose
 *    Enables Viper editor in Matrix
 *
 * @author  Edison Wang <ewang@squiz.com.au>
 * @package   MySource_Matrix
 * @subpackage __core__
 */
var Matrix_Viper = new function() {
    var viper = null;
    this.loadViper = function(options)
    {
	var settings = {
	    // Viper content container
	    toolbarContainer: jQuery('body').find('.viper-toolbar-container-wrapper'),

	    // Default accessibility standard
	    standard: 'WCAG2AA',

	    // Editable div elements (jQuery collection)
	    editableDivs: jQuery('div .with-viper'),

	    // Threshold to apply 'fixedScroll' to the body
	    scrollable: true,
	    scrollOffsetThreshold: 0,

	    // Default plugin set to use
	    plugins: ['ViperCoreStylesPlugin',
		    'ViperKeyboardEditorPlugin',
		    'ViperInlineToolbarPlugin',
		    'ViperHistoryPlugin',
		    'ViperListPlugin',
		    'ViperFormatPlugin',
		    'ViperToolbarPlugin',
		    'ViperTableEditorPlugin',
		    'ViperCopyPastePlugin',
		    'MatrixCopyPastePlugin',
		    'MatrixImagePlugin',
		    'MatrixLinkPlugin',
		    "MatrixKeywordsPlugin",
		    'ViperAccessibilityPlugin',
		    'ViperSourceViewPlugin',
		    'ViperSearchReplacePlugin',
		    'ViperLangToolsPlugin',
		    'ViperCharMapPlugin',
		    'ViperCursorAssistPlugin'],

	    // Give the viper instance a name
	    viperName: 'admin-viper',

	    // The order of buttons for the wysiwyg
	    buttons: [['bold', 'italic', 'subscript', 'superscript', 'strikethrough', 'class'], 'removeFormat', ['justify', 'formats', 'headings'], ['undo', 'redo'], ['unorderedList', 'orderedList', 'indentList', 'outdentList'], 'insertTable', 'image', 'hr', ['insertLink', 'removeLink', 'anchor'], 'insertCharacter', 'searchReplace', 'langTools', 'accessibility', 'sourceEditor', ['insertKeywords', 'insertSnippets']],

	    inlineButtons: [['bold', 'italic', 'class'], ['justify', 'formats', 'headings'], ['unorderedList', 'orderedList', 'indentList', 'outdentList'], ['insertLink', 'removeLink', 'anchor'], ['image', 'imageMove']]
	};
    

	if (settings.editableDivs.length >= 1 && typeof(Viper) !== "undefined") {
	    var  viper = new Viper('MatrixViper', {language: 'en'}, function(viper) {
		var pm = viper.getPluginManager();
		var $body = jQuery('body');
		
		// set scroll event
		if (settings.scrollable && settings.toolbarContainer.length >= 1) {    
		    // Make sure we unbind event before re-binding it
		    jQuery(window).unbind('scroll').bind('scroll',function(){
			var offset_t = settings.toolbarContainer.parent().offset().top - jQuery(window).scrollTop();
			var changedClass = false;
			if (offset_t <= settings.scrollOffsetThreshold) {
			    if ($body.hasClass('fixedScroll') === false) {
					$body.addClass('fixedScroll');
					$body.addClass('backendViperToolbar');
					changedClass = true;
			    }
			} else if ($body.hasClass('fixedScroll') === true) {
			    $body.removeClass('fixedScroll');
			    $body.removeClass('backendViperToolbar');
			    changedClass = true;
			}// End if

			if (changedClass === true) {
			    pm.getPlugin('ViperToolbarPlugin').positionUpdated();
			}
		    });
		}
		
		pm.setPlugins(settings.plugins);
		pm.setPluginSettings('ViperInlineToolbarPlugin', {buttons: settings.inlineButtons});
		pm.setPluginSettings('ViperToolbarPlugin', {buttons: settings.buttons});
		pm.setPluginSettings('ViperAccessibilityPlugin', {standard: settings.standard});

		// need to specify jquery url for the new window view of source code mode
		pm.setPluginSettings('ViperSourceViewPlugin', {jqueryURL: options.jQueryPath});

		
		// Get the toolbar plugin and apply it to the container
		if(settings.toolbarContainer.length >=1) {
		    var toolbar = pm.getPlugin('ViperToolbarPlugin');
		    toolbar.setParentElement(settings.toolbarContainer.children().get(0));
		    
		    // if a parent element is set, it's fixed to element position
		    $body.addClass('fixedElement');
		}
		else {
		    // if there is no parent element, let the tool bar float to top
		    $body.addClass('fixedScroll');
		}
		
		
		settings.editableDivs.each(function() {

			if (typeof MatrixViperDefaultTag != 'undefined') {
				viper.setSetting('defaultBlockTag', MatrixViperDefaultTag);
			}

			viper.registerEditableElement(this);
			var $editable = jQuery(this);

			var $editCallToAction = jQuery('<div/>',{
			"class": 'matrix-viper-no-content',
			text: "This area contains no content yet. Click here to start editing."
			});
			$editable.after($editCallToAction);
			$editCallToAction.hide();
			// when click the no content div, hide it and focus on the editor
			// this ugly mousedown and up event workaround is for IE8. a straight click event would not work for viper because IE is so slow that it thinks the div is not visible to focus yet
			$editCallToAction.bind('mousedown',function(e){
				$editCallToAction.hide();		
				$editable.show();
				jQuery(document).bind('mouseup.viper',function(e){
				$editable.attr('contenteditable', 'true');
				viper.setEditableElement($editable.get(0));
				// Firefox doesn't swallow the BR, so select it.
				if (viper.browser === 'Firefox') {
					viper.selectNode($editable.find('br').get(0));
				}
				viper.element.focus(); 
				viper.focus();
				jQuery(document).unbind('mouseup.viper');
				});
			});

			// Check to see if the area has any content to edit, if not we need
			// to supply a clickable element that brings attention to the fact
			// this area is currently empty

			var rawText = $editable.html();
			if (typeof MatrixViperDefaultTag == 'undefined' && (rawText.match(/^\s+$/) !== null ||
			rawText === "" || rawText.match(/^<[a-z]+>[\s\n(&nbsp;)]*<\/[a-z]+>$/i) !== null)) {
				$editable.html('<p><br/> </p>');
				$editCallToAction.show();
				$editable.hide();
			}

			$editable.bind('mousedown',function(){
				if (viper.getViperElement() !== $editable.get(0)) {
					viper.setEditableElement($editable.get(0));
				}// End if
			});
		});// End foreach
		});// End new Viper

		if (typeof PluginsEnabled != "undefined") {
			// disable/hide the plugins that aren't set to be active
			viper.getPluginManager().setPlugins(PluginsEnabled);

			// Get the toolbar plugin and apply it to the container
			if(settings.toolbarContainer.length >=1) {
				var toolbar = viper.getPluginManager().getPlugin('ViperToolbarPlugin');
				toolbar.setParentElement(settings.toolbarContainer.children().get(0));
			}
		}

		// allow external script to call it
		this.viper = viper;
	}

    };
};


// let's fire it up
jQuery(document).ready(function() {
    var scripts = document.getElementsByTagName('script');
    var options = {};

    // Loop through all the script tags that exist in the document and
    // find which one has included jQuery.
    for (var i = 0; i < scripts.length; i++) {
        if (scripts[i].src) {
            if (scripts[i].src.match(/jquery\.js/)) {
                // We have found our appropriate <script> tag that includes
                // this file, we can extract the path.
                options.jQueryPath = scripts[i].src;
                break;
            }
        }
    }

    Matrix_Viper.loadViper(options);
});
