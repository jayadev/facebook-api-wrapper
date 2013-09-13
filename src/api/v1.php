<?php 

	require_once '/var/www/html/v3.2/db.inc';
	require_once '/var/www/html/v3.2/php/util.php';
	require_once '/var/www/html/v3.2/oauth_config.php';
	
	$debug_on = false;	
	$debug_on = isset($_REQUEST['debug']) ? $_REQUEST['debug'] : false;
	//$debug_on = true;	
	if($debug_on) {
		out($_REQUEST,"Input params: ");
	}
		error_log("Input Params: ".print_r($_REQUEST,true));
	// Response to be sent to the client.
	// Initialise with internal error. If no one resets this, we surely
	// have a weird internal error.
	$response = array('status' => 0,
	                  'error_desc' => 'Description not added with this error') ;
	
	if(empty($_GET['fn'])) {
		$response['error_desc'] = "Function is not present in the api";
		send_response($response);
	}else {
		$fun = $_GET['fn'];
	}

	//Create the db connection object
	$db = new PostingsDb();
	// Check for function existence
	if (!method_exists($db,$fun)) {
		$response['status'] = 0;
		$response['error_desc'] = "Requested Function doesnot exist: $function";
		send_response($response);
	} // Util function to send response, defined here

	try {
		//Finally call the function here
		//Set the access tokens for this call
		$token = isset($_REQUEST['_token']) ? $_REQUEST['_token'] : "";
		$secret = isset($_REQUEST['_secret']) ? $_REQUEST['_secret'] : "";
		$user_id = isset($_REQUEST['user_id']) ? $_REQUEST['user_id'] : "";

		//if(empty($token) || empty($user_id)) {
		if(empty($token)) {
			throw new Exception("Token or User_id not passed along with request");
		}
		//$token = "7a0a1185-1b0c-4074-bfb3-1e2b5fe91222";
		//$secret = "1e3b0967-6e3b-4085-ba81-33e051390c6b";
		$API_CONFIG['access_tokens'] = array ( 'oauth_token' => $token,
											   'oauth_token_secret' => $secret,
											   'user_id' => $user_id );
		$retval = call_user_func(array($db,$fun),$_REQUEST);
	} catch(ErrorException $e) {
		$response['status'] = 0;
		$response['error_desc'] = $e->getMessage();
		send_response($response);
	}

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

	// Send the response back to the client
	function send_response($response) {
		$response = json_encode($response); // JSON-encode the response hash
		header('Cache-Control: no-cache, must-revalidate');
		header('Expires: Mon, 01 Jan 1970 05:00:00 GMT');
		header('Content-type: application/json');
		print($response); // Print the JSON string (becomes response data) and terminate
		exit;
	}

	function out($data,$message=null) {
		print "<BR>".$message."<BR>";
		print "<PRE>";
		print_r($data);
		print "</PRE>";
	}

?>
