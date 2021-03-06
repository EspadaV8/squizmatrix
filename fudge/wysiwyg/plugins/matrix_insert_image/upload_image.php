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
* $Id: upload_image.php,v 1.10 2013/04/23 08:54:32 cupreti Exp $
*
*/

/**
* Upload Image Popup for the WYSIWYG
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix
*/

require_once dirname(__FILE__).'/../../../../core/include/init.inc';
require_once dirname(__FILE__).'/../../../../core/assets/files/image/image.inc';

if (empty($GLOBALS['SQ_SYSTEM']->user) || !($GLOBALS['SQ_SYSTEM']->user->canAccessBackend() || $GLOBALS['SQ_SYSTEM']->user->type() == 'simple_edit_user' || (method_exists($GLOBALS['SQ_SYSTEM']->user, 'isShadowSimpleEditUser') && $GLOBALS['SQ_SYSTEM']->user->isShadowSimpleEditUser()))) {
	echo return_javascript_error('You cannot upload file as a non-backend user');
	exit;
}

// verify nonce secuirty token to make sure the user submitting the request is using Matrix's backend interface
    if(!isset($_POST['token'])) {
	trigger_error('Security token not found');
	exit;
    }
    $token = get_unique_token();
    if($_POST['token'] !== $token) {
	trigger_error('Invalid secuirty token');
	exit;
    } 

// Check if something was submitted
if (!isset($_FILES['create_image_upload']['name']) || !isset($_FILES['create_image_upload']['tmp_name']) || empty($_FILES['create_image_upload']['tmp_name']) || !isset($_FILES['create_image_upload']['error']) || !empty($_FILES['create_image_upload']['error'])) {
	// No file submitted
	echo return_javascript_error('No file submitted');
	exit;
} else if (!isset($_POST['f_create_root_node']['assetid']) || empty($_POST['f_create_root_node']['assetid'])) {
	// No root node specified
	echo return_javascript_error('No root node selected');
	exit;
}//end if

// OK, hopefully I have the right information, let's continue and try and create the asset
$am = $GLOBALS['SQ_SYSTEM']->am;
$root_node = $am->getAsset($_POST['f_create_root_node']['assetid']);
$successful = FALSE;
$success_return = '';
if (!is_null($root_node)) {
	$new_image = new Image();

	// Prepare the image for uploading
	$new_image->_tmp['uploading_file'] = TRUE;
	$_FILES['create_image_upload']['filename'] = $_FILES['create_image_upload']['name'];
	$_FILES['create_image_upload']['path'] = $_FILES['create_image_upload']['tmp_name'];
	
	// Check for valid file types
	$invalid_file_type = $new_image->validFile($_FILES['create_image_upload']);
	if (!$invalid_file_type) {
		echo return_upload_error('File extension not allowed. [CORE0106]');
		exit();
	}//end if

	// Create the image
	$new_image->setAttrValue('name', $_FILES['create_image_upload']['name']);
	$new_image->saveAttributes();
	$link = Array(
				'asset'			=> $root_node,
				'link_type'		=> SQ_LINK_TYPE_1,
				'value'			=> '',
				'sort_order'	=> -1,
			);
	$successful = $new_image->create($link, $_FILES['create_image_upload']);
	if ($successful) {
		// If image creation successful, update the form and close this dialog
		ob_start();
			?>
			<html><head>
			<script type="text/javascript">
				top.frames['sq_wysiwyg_popup_main'].toggleCreateImage();
				top.frames['sq_wysiwyg_popup_main'].document.getElementById('sq_asset_finder_f_imageid_assetid').value = "<?php echo $new_image->id; ?>";
				top.frames['sq_wysiwyg_popup_main'].document.getElementById('sq_asset_finder_f_imageid_label').value = "<?php echo $new_image->short_name; ?>";
				top.frames['sq_wysiwyg_popup_main'].document.getElementById('f_imageid[assetid]').value = "<?php echo $new_image->id; ?>";
				top.frames['sq_wysiwyg_popup_main'].setImageInfo();
			</script>
			</head>
			<body></body>
			</html>
			<?php
			$success_return = ob_get_contents();
		ob_end_clean();

		echo $success_return;
		exit();
	} else {
		echo return_upload_error('Unable to create file, web path already exists[CORE0086] or file is infected[CORE0300]');
		exit();
	}//end if
} else {
	echo return_upload_error('Invalid root node');
	exit();
}//end if

// We get to here than an ERROR occurred, so respond with a generic error
echo return_upload_error('Could not create image asset');


/**
* Return a javascript error inside the iframe
*
* @return void
* @access public
*/
function return_javascript_error($error='') {
	$return_code = '';
	if (!empty($error)) {
		ob_start();
			?>
			<html><head>
			<script type="text/javascript">
				alert('<?php echo $error; ?>');
			</script>
			</head>
			<body></body>
			</html>
			<?php
			$return_code = ob_get_contents();
		ob_end_clean();
	}//end if

	return $return_code;

}//end return_javascript_error()


/**
* Return a upload error by changing the 'show_upload_error' div inside the main frame
*
* @return void
* @access public
*/
function return_upload_error($error='') {
	$return_code = '';
	if (empty($error)) {
		$error = 'Service unavailable, could not upload image';
	}//end if
	ob_start();
		?>
		<html><head>
		<script type="text/javascript">
				top.frames['sq_wysiwyg_popup_main'].document.getElementById('show_upload_error').style.display = "block";	
				top.frames['sq_wysiwyg_popup_main'].document.getElementById('show_upload_error').style.visibility = "visible";	
				top.frames['sq_wysiwyg_popup_main'].document.getElementById('show_upload_error').innerHTML = "<?php echo $error ?>";
		</script>
		</head>
		<body></body>
		</html>
		<?php
		$return_code = ob_get_contents();
	ob_end_clean();

	return $return_code;

}//end return_upload_error()


?>
