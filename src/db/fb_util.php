<?php
$base_path = '/home/ec2-user/gitSrc';
require_once $base_path.'/src/db/fb_oauth_config.php';
require_once $base_path.'/src/fb_sdk/facebook.php';

/**
 * Session existance check.
 * 
 * Helper function that checks to see that we have a 'set' $_SESSION that we can
 * use for the demo.   
 */ 
function oauth_session_exists() {
  if((is_array($_SESSION)) && (array_key_exists('oauth', $_SESSION))) {
    return TRUE;
  } else {
    return FALSE;
  }
}

function checkUserLogin() {
	//If you are coming here as part of signout flow
	$index_page = "http://".$_SERVER['HTTP_HOST']."/v3.2/index.php";
	if(is_loggedIn()) {
		//user logged in to cb
		if(is_linkedInAuthorized()) {
			return true;
		}else {
			//user logged not authorized - weird case - removed the cookies manually
			error_log("User logged in but not authorized to linkedin");
			//Initiate linked in authorization
			header('Location: ' . $index_page);
		}
	}else {
		//Not logged in redirect him to index page for login in
		error_log("User not logged in ");
		header('Location: ' . $index_page);
	}
}

function is_loggedIn() {
	if(isset($_SESSION['cb']) && ($_SESSION['cb'] == TRUE)) {
		return true;
	}else {
		return false;
	}
}

function is_linkedInAuthorized(){
	if(isset($_SESSION['oauth']) && $_SESSION['oauth']['linkedin']['authorized']){
		return true;
	}
	else{ 
		return false;
	}
}

function getLinkedInUserId() {

	global $API_CONFIG;
	if(!oauth_session_exists()) {
		throw new LinkedInException('This script requires session support, which doesn\'t appear to be working correctly.');
	}
	
	$OBJ_linkedin = new LinkedIn($API_CONFIG);
	$OBJ_linkedin->setTokenAccess($_SESSION['oauth']['linkedin']['access']);
	$response = $OBJ_linkedin->profile('~:(id,first-name,last-name,picture-url)');
	if($response['success'] === TRUE) {
		$response['linkedin'] = new SimpleXMLElement($response['linkedin']);
	} else {
		// profile retrieval failed
		error_log("Error retrieving profile information:<br /><br />RESPONSE:<br /><br /><pre>" . print_r($response,true) . "</pre>");
	} 

	$id = isset($response['linkedin']->id) ? $response['linkedin']->id : false;
	return $id;
}

function getLinkedInUserProfile() {

	global $API_CONFIG;
	if(!oauth_session_exists()) {
		throw new LinkedInException('This script requires session support, which doesn\'t appear to be working correctly.');
	}
	
	$OBJ_linkedin = new LinkedIn($API_CONFIG);
	$OBJ_linkedin->setTokenAccess($_SESSION['oauth']['linkedin']['access']);
	$response = $OBJ_linkedin->profile('~:(id,first-name,last-name,picture-url,headline,location,three-current-positions)');
	if($response['success'] === TRUE) {
		$response['linkedin'] = new SimpleXMLElement($response['linkedin']);
	} else {
		// profile retrieval failed
		error_log("Error retrieving profile information:<br /><br />RESPONSE:<br /><br /><pre>" . print_r($response,true) . "</pre>");
	} 

	$id = isset($response['linkedin']->id) ? $response['linkedin']->id : false;
	if(!$id) return false;

	//Extract the info for easy access
	$profile = array();
	$profile['id'] = (string)$id;
	$complete_name = (string)$response['linkedin']->{'first-name'} ." ". (string)$response['linkedin']->{'last-name'};
	$profile['user_name'] = ucwords($complete_name);
	$picture_url = (string)$response['linkedin']->{'picture-url'};
	$profile['picture_url'] = (empty($picture_url)) ? "anonymous.png" : $picture_url;
	$profile['headline'] = (string)$response['linkedin']->headline;
	$profile['location'] = (string)$response['linkedin']->location->name;

	$current_positions = $response['linkedin']->{'three-current-positions'};
	$no_positions = (int)$current_positions['total'];
	$company =(string)$current_positions->position->company->name;

	$profile['company'] = $company;
	return $profile;
}

function shareToFriendStream($action,$content) {
	global $API_CONFIG;
	
	$fb = new facebook($API_CONFIG);
	$fb->setAccessToken($API_CONFIG['access_tokens']['oauth_token']);
	$user_id = $API_CONFIG['access_tokens']['user_id'];

	try {
		//Post it to user feed
		$ret_obj = $fb->api("/$user_id/feed", 'POST',
					array(
						//'caption' => $content['title'],
						'link' => $content['submitted-url'],
						'message' => $content['comment'],
						'picture' => $content['submitted-image-url'],
						'description' => $content['description'],
						'application' => 'cubesales',
						'status_type' => 'app_created_story'
					));
	} catch(FacebookApiException $e) {
		//Possible that the access token for user_id has expired
		error_log($e->getType().":".$e->getMessage());
		return false;
	}   	

	return $ret_obj;
}

function getUserProfileForMobile() {
	global $API_CONFIG;
	
	$fb = new Facebook($API_CONFIG);
	$fb->setAccessToken($API_CONFIG['access_tokens']['oauth_token']);
	$user_id = $API_CONFIG['access_tokens']['user_id'];

	try {
		//Get user profile info
		$user_profile = $fb->api("/$user_id",'GET');
		return $user_profile;
	} catch(FacebookApiException $e) {
		//Possible that the access token for user_id has expired
		error_log($e->getType().":".$e->getMessage());
		return false;
	}   	
}

function getUserFriends($count) {
	global $API_CONFIG;

	//@todo currently return empty list of friends, will enable it once the format
	//is clear
	return array();

	$fb = new Facebook($API_CONFIG);
	$fb->setAccessToken($API_CONFIG['access_tokens']['oauth_token']);
	$user_id = $API_CONFIG['access_tokens']['user_id'];

	try {
		//Get user profile info
		$connections = $fb->api("/$user_id/friends",'GET');
		return $connections['data'];
	} catch(FacebookApiException $e) {
		//Possible that the access token for user_id has expired
		error_log($e->getType().":".$e->getMessage());
		return false;
	}   	
	
	return $connections;
}

?>
