<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: thank_you_keywords.php,v 1.4 2005/04/29 05:40:28 gsherwood Exp $
*
*/

	require_once dirname(__FILE__).'/../../../../../core/include/init.inc';
	require_once dirname(__FILE__).'/../../../../../core/lib/html_form/html_form.inc';
	if (!isset($_GET['assetid'])) return false;

	assert_valid_assetid($_GET['assetid']);
	$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($_GET['assetid']);
	if (!is_a($asset, 'form')) {
		trigger_localised_error('CMS0002', E_USER_ERROR, $asset);
		return false;
	}
?>

<html>
	<head>
		<title>'<?php echo $asset->attr('name') ?>' Thank You / Emails Keyword Replacements</title>
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
	</head>

	<body>
	<?php
		require_once dirname(__FILE__).'/../../../../../core/include/backend_outputter.inc';
		//$backend = new Backend();
		$o =& new Backend_Outputter();

		$o->openSection('Keyword List for \''.$asset->attr('name').'\' (#'.$asset->id.')');
		$o->openField('&nbsp;');

		$questions = $asset->getQuestions();
		$sections  = $asset->getSections();
	?>
				<p>These keywords are available for use in Complex Formatting for insertion into the 'Thank You' bodycopy, if it is enabled, as well as in emails sent from this form. The <b>'Response'</b> keywords (%response_*%) are replaced with the actual response for that question. The <b>'Section Title'</b> keywords (%section_title_*%) will be replaced with the name of the section.</p>

		<p>
		<fieldset>
			<legend><b>Unattached Questions</b></legend>
			<table border="0" width="100%">
				<?php
					foreach ($questions as $q_id => $question) {
						$q_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($asset->id.':q'.$q_id);
						$q_name = $q_asset->attr('name');
						?>							<tr><td valign="top" width="200"><b>%response_<?php echo $asset->id.'_q'.$q_id ?>%</b></td><td valign="top"><?php echo $q_name ?></td></tr><?php
					}?>
			</table>
		</fieldset>
		</p>

			<?php
				foreach ($sections as $section) { ?>
				<p>
					<fieldset>
					<legend><b>Form Section Asset '<?php echo $section->attr('name') ?>' (Id: #<?php echo $section->id ?>)</b></legend>
						<table border="0" width="100%">
							<tr><td valign="top" width="200"><b>%section_title_<?php echo $section->id ?>%</b></td><td valign="top">Section Title</td></tr>
				<?php
					$replacements['section_title_'.$section->id] = $section->attr('name');
					$questions = &$section->getQuestions();
					foreach ($questions as $q_id => $question) {
						$q_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($section->id.':q'.$q_id);
						$q_name = $section->attr('name').': '.$q_asset->attr('name');
						?>
						<tr><td valign="top" width="200"><b>%response_<?php echo $section->id.'_q'.$q_id; ?>%</b></td><td valign="top"><?php echo $q_name; ?></td></tr><?php } ?>
						</table>
					</fieldset>
				</p>
				<?php } ?>


			</table>
		</fieldset>
		</p>

<?php
$o->openField('', 'commit');
normal_button('cancel', 'Close Window', 'window.close()');
$o->closeSection();
$o->paint();
?>
	</body>
</html>