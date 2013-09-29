<?php 

	require_once '../db/db.inc';
	require_once '../db/fb_util.php';
	require_once '../db/fb_oauth_config.php';
	require_once 'api_util.php';
	
	$debug_on = false;	
	$debug_on = isset($_REQUEST['debug']) ? $_REQUEST['debug'] : false;
	if($debug_on) {
		out($_REQUEST,"Input params: ");
	}
	error_log("Input Params: ".print_r($_REQUEST,true));

	// Response to be sent to the client.
	// Initialise with internal error. If no one resets this, we surely
	// have a weird internal error.
	$response = array('status' => 0,
	                  'error_desc' => 'Status was not set by the function') ;
	
	if(empty($_GET['fn'])) {
		$response['error_desc'] = "Function is not present in the api";
		send_response($response);
	}
	$retval = callApiFunction($_REQUEST,$debug_on);

	if($debug_on) {
		out($retval,"Function output :");
	}

	if ($retval === false) {
		$response['status'] = 0;
		$response['error_desc'] = "Error in the function call";
	}else {
		$response = $retval;	
	}

	send_response($response);

?>
