<?php
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
* $Id: insert_dfn.php,v 1.7 2013/04/23 08:10:10 cupreti Exp $
*
*/

/**
* Insert Definition Popup for the WYSIWYG
*
* @author  Scott Kim <skim@squiz.net>
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix
*/

require_once dirname(__FILE__).'/../../../../core/include/init.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_FUDGE_PATH.'/var_serialise/var_serialise.inc';

if (empty($GLOBALS['SQ_SYSTEM']->user) || !($GLOBALS['SQ_SYSTEM']->user->canAccessBackend() || $GLOBALS['SQ_SYSTEM']->user->type() == 'simple_edit_user' || (method_exists($GLOBALS['SQ_SYSTEM']->user, 'isShadowSimpleEditUser') && $GLOBALS['SQ_SYSTEM']->user->isShadowSimpleEditUser()))) {
	exit;
}

if (!isset($_GET['title'])) $_GET['title'] = "";
//Sanitise
$_GET['title'] = preg_replace('/[\'"\(\);\[\]{}<>=]+/', '', $_GET['title']);
$_GET['dfn'] = preg_replace('/[\'"\(\);\[\]{}<>=]+/', '', $_GET['dfn']);
?>

<!DOCTYPE html>
<html style="height: 250px;">
	<head>
		<title>Insert Definition</title>
		<link rel="stylesheet" type="text/css" href="<?php echo sq_web_path('lib').'/web/css/edit.css' ?>" />
		<link rel="stylesheet" type="text/css" href="<?php echo sq_web_path('root_url')?>/__fudge/wysiwyg/core/popup.css" />

		<?php
		// add required js translation files, as we are using asset finder
		$include_list = Array(sq_web_path('lib').'/js/translation.js');

		$locales = $GLOBALS['SQ_SYSTEM']->lm->getCumulativeLocaleParts($GLOBALS['SQ_SYSTEM']->lm->getCurrentLocale());

		foreach($locales as $locale) {
			if (file_exists(SQ_DATA_PATH.'/public/system/core/js_strings.'.$locale.'.js')) {
				$include_list[] = sq_web_path('data').'/system/core/js_strings.'.$locale.'.js';
			}
		}

		foreach($include_list as $link) {
			?><script type="text/javascript" src="<?php echo $link; ?>"></script>
		<?php
		}
		?>
		<script type="text/javascript" src="../../core/popup.js"></script>
		<script type="text/javascript" src="<?php echo sq_web_path('fudge').'/var_serialise/var_serialise.js' ?>"></script>
		<script type="text/javascript" src="<?php echo sq_web_path('lib').'/html_form/html_form.js' ?>"></script>
		<script type="text/javascript" src="<?php echo sq_web_path('lib').'/js/general.js' ?>"></script>

		<script type="text/javascript">

			function getFocus() {
				setTimeout('self.focus()',100);
			};

			function Init() {
				__dlg_init("matrixInsertDfn");
				setTimeout('self.focus()',100);
			};

			function onOK() {
				// pass data back to the calling window
				var fields = ["title"];
				var param = new Object();
				var f = document.main_form;

				param["title"] = form_element_value(f.title);
				param["dfn"]  = form_element_value(f.dfn);

				__dlg_close("matrixInsertDfn", param);
				return false;
			};

			function onCancel() {
				__dlg_close("matrixInsertDfn", null);
				return false;
			};

			function buildForm() {
				document.write('<tr>');
				document.write('<td class="label"><?php echo translate('definition'); ?>:</td>');
				document.write('<td colspan="3"><?php text_box('dfn', trim($_GET['dfn']), 40, 0);?></td>');
				document.write('</tr>');
				document.write('<tr>');
				document.write('    <td class="label"><?php echo translate('title'); ?>:</td>');
				document.write('    <td colspan="3"><?php text_box('title', trim($_GET['title']), 40, 0);?></td>');
				document.write('</tr>');
			};

		</script>
		
	</head>

	<body onload="Javascript: Init();">

		<div class="sq-popup-heading-frame">
			<h1><?php echo translate('insert_definition'); ?></h1>
		</div>
		<form action="" method="get" name="main_form" id="main-form">
			<table width="100%" class="sq-fieldsets-table">
				<tr>
					<td valign="top" width="100%">
						<fieldset>
						<legend><b><?php echo translate('general'); ?></b></legend>
						<table style="width:100%">
							<script type="text/javascript">
								buildForm();
							</script>
						</table>
						</fieldset>
					</td>
				</tr>
			</table>

			<div class="sq-popup-button-wrapper">					
				<button type="button" name="cancel" onclick="return onCancel();"><?php echo translate('cancel'); ?></button>
				<script type="text/javascript" language="javascript">
				document.write('<button type="button" name="ok" onclick="return onOK();" class="sq-btn-green"><?php echo translate('ok'); ?></button>');
				</script>
			</div>
		</form>
	</body>
</html>
