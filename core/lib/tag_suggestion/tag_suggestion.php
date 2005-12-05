<?php
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
* $Id: tag_suggestion.php,v 1.1 2005/12/05 06:07:54 emcdonald Exp $
*
*/

	require_once dirname(__FILE__).'/../../include/init.inc';
	require_once dirname(__FILE__).'/../html_form/html_form.inc';
	if (!isset($_GET['assetid'])) return false;
	if (!isset($_GET['prefix'])) return false;



	$assetid = $_GET['assetid'];
	assert_valid_assetid($assetid);
	$prefix = $_GET['prefix'];

	$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
	$button_name = 'sq_asset_finder_'.$prefix.'_tags_more_btn';
	$labelname = 'sq_asset_finder_'.$prefix.'_tags';
	$idname = $prefix.'_tags';



?>

		<?php
$sm = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager');
if (empty($sm)) {
	echo translate('tag_list_not_available');
} else {

	$keyword_ids = Array();
	$keyword_names = Array();
	$current_tag_ids = Array();
	$quoted_keyword_ids = Array();
	$keywords = $sm->getAssetidsByWordIntersection($asset->id, 'thesaurus_term');

	$current_tag_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($assetid, SQ_LINK_NOTICE);

	foreach ($current_tag_links as $key => $current_tag_link) {
		$current_tag_ids[] = $current_tag_link['minorid'];
	}

	foreach ($keywords as $keyword) {
		$keyword_ids[] = $keyword['assetid'];
	}

	$keyword_ids = array_values(array_diff($keyword_ids, $current_tag_ids));
	foreach ($keyword_ids as $keyword_id) {
		$quoted_keyword_ids[] = "'".$keyword_id."'";
	}

	$keywords_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($keyword_ids);

	?>

<html>
	<head>
		<title><?php echo "'".$asset->attr('name')."' ".translate('tag_suggestion'); ?></title>
		<style>
			body {
				background-color:	#FFFFFF;
			}

			body, p, td, ul, li, input, select, textarea{
				color:				#000000;
				font-family:		Arial, Verdana Helvetica, sans-serif;
				font-size:			11px;
			}

			fieldset {
				padding:			0px 10px 5px 5px;
				border:				1px solid #E0E0E0;
			}

			legend {
				color:				#2086EA;
			}
		</style>
		<script type="text/javascript">
		displayedButtons = new Array(<?php echo implode(',', $quoted_keyword_ids); ?>);


		function addTag(name, id)
		{
			next_index = 0;
			while (opener.document.getElementById('<?php echo $labelname ?>_'+(next_index + 1)+'__label') != null){
				next_index++;
			}
			label = opener.document.getElementById('<?php echo $labelname ?>_'+next_index+'__label');
			label.value = name+' (id: #'+id+')';
			assetid = opener.document.getElementById('<?php echo $idname ?>['+next_index+'][assetid]');
			assetid.value = id;
			more_btn = opener.document.getElementById('<?php echo $button_name ?>');
			more_btn.onclick();
		}
		function hideTagButton(id)
		{
			removediv = document.getElementById(('tag'+id));
			removediv.style.display = 'none';

			newArray = new Array();
			for (var i=0; i<displayedButtons.length; i++)
			{
				if(displayedButtons[i] != id) {
					newArray.push(displayedButtons[i]);
				}
			}
			displayedButtons = newArray;
			checkTagsLeft();

		}
		function checkTagsLeft()
		{

			if (displayedButtons.length == 0) {
					noMoreTags = document.getElementById('noMoreTags');
					noMoreTags.style.visibility = 'visible';

					addAll = document.getElementById('addAll');
					addAll.style.display = 'none';
			}
		}

		</script>
	</head>

	<body>
	<?php
		require_once dirname(__FILE__).'/../../include/backend_outputter.inc';
		$o =& new Backend_Outputter();
		$o->addOnLoad('checkTagsLeft()');
		$o->openSection(translate('suggested_tags_for').' '.get_asset_tag_line($asset->id));
		$o->openField('');
	?>
				<p><?php echo translate('suggested_tags_for').' '.get_asset_tag_line($asset->id); ?></p>
				<p><?php echo translate('click_to_tag'); ?></p>

		<p>
		<fieldset>
			<legend><b><?php echo 'Suggested Tags'; ?></b></legend>



	<?php
	foreach ($keywords_info as $keyword_info) {
		?><input type="button" name="Tag" id="tag<?php echo $keyword_info['assetid']; ?>" value="<?php echo $keyword_info['name']; ?>"
		onclick="javascript:
							addTag('<?php echo $keyword_info['name']; ?>', '<?php echo $keyword_info['assetid']; ?>');
							hideTagButton('<?php echo $keyword_info['assetid']; ?>');
				"
			class="sq-form-field" />
		<?php
	}

}
?><div id="noMoreTags" style="visibility:hidden;">
			<?php echo translate('no_tags_to_add'); ?>
			</div>
	<div id="addAll" onLoad="javascript: checkTagsLeft();" >
			<input type="button" name="Tag" id="add_all" value="Add all"
		onclick="javascript:
					staticLength = displayedButtons.length
					for (var i=0; i<staticLength; i++)
					{
						button = getElementById('tag'+displayedButtons[0]);
						button.onclick();
					}
				"
			class="sq-form-field" />
	</div>
		</fieldset>
		</p>
<?php
$o->openField('', 'commit');
normal_button('cancel', translate('close_window'), 'window.close()');
$o->closeSection();
$o->paint();
?>
	</body>
</html>