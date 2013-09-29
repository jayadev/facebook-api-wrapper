<?php
	$base_path = '/home/ec2-user/gitSrc';
	require_once $base_path.'/src/db/fb_oauth_config.php';

	function callApiFunction($RequestParams) {
		global $API_CONFIG;

		//Create the db connection object
		$db = new PostingsDb();
		$fun = $RequestParams['fn'];
		// Check for function existence
		if (!method_exists($db,$fun)) {
			$response['status'] = 0;
			$response['error_desc'] = "Requested Function doesnot exist: $function";
			return $response;
		} // Util function to send response, defined here

		try {
			//Finally call the function here
			//Set the access tokens for this call
			$token = isset($RequestParams['_token']) ? $RequestParams['_token'] : "";
			$secret = isset($RequestParams['_secret']) ? $RequestParams['_secret'] : "";
			$user_id = isset($RequestParams['user_id']) ? $RequestParams['user_id'] : "";

			//if(empty($token) || empty($user_id)) {
			if(empty($token)) {
				throw new Exception("Token or User_id not passed along with request");
			}
			
			$API_CONFIG['access_tokens'] = array ( 
					'oauth_token' => $token,
					'oauth_token_secret' => $secret,
					'user_id' => $user_id );

			return call_user_func(array($db,$fun),$RequestParams);

		} catch(ErrorException $e) {
			$response['status'] = 0;
			$response['error_desc'] = $e->getMessage();
			return $response;
		}

	}

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
