<?php
$base_path = '/home/ec2-user/gitSrc';
require_once $base_path.'/src/db/db.inc';
require_once $base_path.'/src/db/fb_util.php';
require_once $base_path.'/src/api/api_util.php';
require_once $base_path.'/src/db/fb_oauth_config.php';

class ApiTest extends PHPUnit_Framework_TestCase {

	function __construct() {
		error_reporting(E_ALL);
		ini_set('display_errors','On');
		$this->access_token = "CAACMVKZCPtEgBAF8ZBzcZC8hEgxnc0Tbkp7970Y39LlZBbJKdGJQhXXr5VnewzoSNQolYn44TQhWoiLEpqi3fFSnlONIj37DzakFLHZCKimLo4m8T42o7NCleB8JbLTHJydh8qM0yyUNM4TSCp8LKzZBS6NbRFf6muoV6ztOAy1UGZAjgO0lhIPLYZAFVtnruV4Fu82WZCnXN8AZDZD";
		$this->registerUserParams = array
			(
			 "fn" => "registerUser",
			 "_token" => $this->access_token,
			 "user_id" => "100006561245572",
			 "email" => "sushant@cubesales.com",
			 "firstname" => "Cube",
			 "lastname" => "Sales",
			 "username" => "Cube Sales",
			 "img_url" => "https://fbcdn-profile-a.akamaihd.net/hprofile-ak-prn2/186115_100006388052764_1981138342_q.jpg",
			 "profile_url" => "https://www.facebook.com/cube.sales",
			 "company_email" => "sushantk@yahoo-inc.com",
			 "company_zip" => "94115",
			 "company_city" => "San Francisco",
			 "company_ccode" => "US",
			 "headline" => "Senior Software Developer",
			 "update" => "0",
			);
	}

	public function testDbConnection() {
		$udb = new UserDb();
		$user_profile = $udb->get_user_details("user_id");
	}

	/*public function testSharedStream() {
		global $API_CONFIG;
		$input = $this->registerUserParams;

		$prodLogo = "http://cubesales.com/img/Linkedin_Post.png";
		$prodUrl  = "http://www.cubesales.com/";
		$prodDescription = "Build a trusted & convenient marketplace with your colleagues.";
		$comment = "From unit test: Started using Cubesales Mobile App! Buy, sell & trade at your workplace with trust & convenience";
		$share_content = array ('comment' => $comment,
				'title' => 'Check out Cubesales',
				'submitted-url' => $prodUrl,
				'submitted-image-url' => $prodLogo,
				'description' => $prodDescription);
		
		$API_CONFIG['access_tokens'] = array (
				'oauth_token' => $input['_token'],
				'user_id' => $input['user_id'] );

		
		$response = shareToFriendStream("new",$share_content);
		print_r($response);
	}*/


	public function testRegisterUser() {
		$input = $this->registerUserParams;
		$this->assertEquals(15,count($input));
	
		//Make sure registration requires user_id
		unset($input['user_id']);
		$response = callApiFunction($input);
		$this->assertEquals($response['status'],0);
		$this->assertEquals($response['error_desc'],"User id / company_email  not present");
	
	    //Make sure registration requires company_email 
		$input = $this->registerUserParams;
		unset($input['company_email']);
		$response = callApiFunction($input);
		$this->assertEquals($response['status'],0);
		$this->assertEquals($response['error_desc'],"User id / company_email  not present");
	
		//Wrong email id
		$input = $this->registerUserParams;
		$input['company_email'] = "blah";
		$response = callApiFunction($input);
		$this->assertEquals($response['status'],0);
		$this->assertEquals($response['error_desc'],"Company email domain not valid");
		print_r($response);
		
		
		$input = $this->registerUserParams;
		$response = callApiFunction($input);
		$this->assertEquals($response['status'],1);
	}

	/*public function testPostListing() {
		$postListingParams = array (
				"fn" => "postListing",
				"_token" => $this->access_token,
				"user_id" => "100006561245572",
				"posting_photo_size" => 480,
				"posting_price" => 'coffee',
				"posting_description" => "Selling a car",
				"posting_status" => 0,
				"consent_linkedin" => 1
				);
			
		
		  $_FILES = array (
			"posting_image" => 
			array (
			 "name" => "image.jpg",
			 "type" => "image/jpeg",
			 "tmp_name" => "/tmp/phpRZZtB8",
			 "error" => 0,
			 "size" => 38185
			),

			"posting_image_small" => array
			(
			 "name" => "image_small.jpg",
			 "type" => "image/jpeg",
			 "tmp_name" => "/tmp/phpdbODDW",
			 "error" => 0,
			 "size" => 18539
			),

			"posting_audio" => array
			(
			 "name" => "test.caf",
			 "type" => "audio/x-caf",
			 "tmp_name" => "/tmp/phpdbblah",
			 "error" => 0,
			 "size" => 18539
			)

			);

	

		unset($postListingParams['user_id']);
		$response = callApiFunction($postListingParams);
		$this->assertEquals($response['error_desc'],"User id not present");

		$postListingParams['user_id'] = "100006561245572";
		$response = callApiFunction($postListingParams);
		print_r($response);
		$this->assertEquals($response['status'],1);
	}

	public function testgetUserProfileForMobile() {
		global $API_CONFIG;
		
		$API_CONFIG['access_tokens'] = array (
				'oauth_token' => $this->access_token,
				'user_id' => "100006561245572" );

		$response = getUserProfileForMobile();
		$this->assertEquals($response['id'],"100006561245572");
		$this->assertEquals($response['name'],"Manoj Kumar");
	}

	public function testgetUserFriends() {
		global $API_CONFIG;
		
		$API_CONFIG['access_tokens'] = array (
				'oauth_token' => $this->access_token,
				'user_id' => "100006561245572" );

		$response = getUserFriends(100);
		print_r($response);
		//$this->assertEquals($response['id'],"100006561245572");
		//$this->assertEquals($response['name'],"Manoj Kumar");
	}*/

	public function testGetUserListing() {

		$getListingParams = array( 
				"filter_type" => "my",
				"fn" => "getUserListings",
				"_token" => $this->access_token,
				"user_id" => "100006561245572"
				);
				
		$response = callApiFunction($getListingParams);
		$this->assertEquals($response['status'],1);
	
		$getListingParams['filter_type'] = "myl";
		$response = callApiFunction($getListingParams);
		print_r($response);
		$this->assertEquals($response['status'],1);
	}

	public function testGetUserInfo() {

		$getUserInfoParams = array( 
				"fn" => "getUserInfo",
				"_token" => $this->access_token,
				"user_id" => "100006561245572"
				);
				
		$response = callApiFunction($getUserInfoParams);
		$this->assertEquals($response['status'],1);
		//print_r($response);
		
		$getUserInfoParams = array( 
				"fn" => "getUserInfo",
				"_token" => $this->access_token
				//"user_id" => "100006561245572"
				);
				
		$response = callApiFunction($getUserInfoParams);
		$this->assertEquals($response['status'],0);

	}

	private $registerUserParams;
	private $access_token;

}
?>
