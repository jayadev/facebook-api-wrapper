<?php

require_once 'fb_util.php';
require 'db_config_test.ini';
require 'ses.php';

abstract class db {
  protected static $dbh = false;

  function connect() {
	global $DB_CONFIG;
	try {

	self::$dbh = new PDO("mysql:host=".$DB_CONFIG['DB_HOST'].";dbname=".$DB_CONFIG['DB_NAME'],
						 $DB_CONFIG['DB_USER'],
						 $DB_CONFIG['DB_PASSWD'],
						 array(PDO::MYSQL_ATTR_USE_BUFFERED_QUERY => true, 
							   PDO::ATTR_EMULATE_PREPARES => true));
    self::$dbh->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
	} catch (PDOException $e) {
		echo "Connection failed :".$e->getMessage();
	}
  }

  protected function fatal_error($msg) {
    echo "<pre>Error!: $msg\n";
    $bt = debug_backtrace();
    foreach($bt as $line) {
      $args = var_export($line['args'], true);
      echo "{$line['function']}($args) at {$line['file']}:{$line['line']}\n";
    }
    echo "</pre>";
    die();
  }
}

class UserDb extends db {
  function save($record) {
    if(empty($record['user_id'])) return false;
    if(!self::$dbh) $this->connect();
    try {
      $stmt = self::$dbh->prepare('INSERT INTO users (user_id,username,firstname,lastname,img_url,headline,company,companyid,city,zip,ccode,email,hobby,activationKey,status) VALUES (:user_id,:username,:firstname,:lastname,:img_url,:headline,:company,:companyid,:city,:zip,:ccode,:email,:hobby,:activationKey,:status) ON DUPLICATE KEY UPDATE user_id=:user_id,username=:username,firstname=:firstname,lastname=:lastname,img_url=:img_url,headline=:headline,company=:company,companyid=:companyid,city=:city,zip=:zip,ccode=:ccode,email=:email');
      $params = array(':user_id'=>$record['user_id'],
                      ':username'=>$record['username'],
                      ':firstname'=>$record['firstname'],
                      ':lastname'=>$record['lastname'],
                      ':img_url'=>$record['img_url'],
                      ':headline'=>$record['headline'],
                      ':company'=>$record['company'],
                      ':companyid'=>$record['companyid'],
                      ':city'=>$record['company_city'],
                      ':zip'=>$record['zip'],
                      ':ccode'=>$record['ccode'],
                      ':email'=>$record['email'],
                      ':hobby'=>$record['hobby'],
					  ':activationKey'=>$record['activationKey'],
					  ':status'=>$record['status']
				);
	  //echo "about to write these records". print_r($params,true);
      $stmt->execute($params);
    } catch (PDOException $e) {
      $this->fatal_error($e->getMessage());
      return false;
    }
    return true;
  }

  function get_user_details($user_id) {
	if(empty($user_id)) return false;
    if(!self::$dbh) $this->connect();
    try {
	  $stmt = self::$dbh->prepare('SELECT company,city,zip from users where user_id = :user_id');
	  $params = array(':user_id' => $user_id);
      $stmt->execute($params);

      $row = $stmt->fetchAll(PDO::FETCH_ASSOC);
	  
	  $response = array();
	  $response['status'] = 1;
	  $response['response_data'] = $row;
	  
	  return $response;

    } catch (PDOException $e) {
      $this->fatal_error($e->getMessage());
	  return false;
    }
    return false;
  }

  function check_user($user_id,$all=false) {
	if(empty($user_id)) return false;
    if(!self::$dbh) $this->connect();
    try {
      $user_id= self::$dbh->quote($user_id);
      $result = self::$dbh->query("SELECT count(*) as c from users where user_id = $user_id");
      $row = $result->fetchAll(PDO::FETCH_ASSOC);
	  if(isset($row[0][c]) && ($row[0][c] == 0)) {		//Empty ==> new user
		  return false;
	  }else {
		  return true;
	  }
    } catch (PDOException $e) {
      $this->fatal_error($e->getMessage());
    }
    return false;
  }

  function validate_user($record) {
	if(empty($record['user_id'])) return false;
	$user_id = $record['user_id'];
	$activationKey = $record['activationKey'];
    if(!self::$dbh) $this->connect();
    try {
      $user_id = self::$dbh->quote($user_id);
      $result = self::$dbh->query("SELECT activationKey FROM users WHERE user_id = $user_id");
      $row = $result->fetchAll(PDO::FETCH_ASSOC);
	  if(isset($row[0]['activationKey']) && ($row[0]['activationKey'] === $activationKey)) {
		  return true;
	  }else {
		  return false;
	  }
    } catch (PDOException $e) {
      $this->fatal_error($e->getMessage());
    }
    return false;
  }

}

class CommentsDb extends db {

 function addComment($record) {
    if(!self::$dbh) $this->connect();
    try {
      $stmt = self::$dbh->prepare('INSERT INTO comments (fkey_post_id,fkey_user_id,ip,comment,notify,spam) VALUES (:fkey_post_id,:fkey_user_id,:ip,:comment,:notify,:spam)');
      $params = array(':fkey_post_id'=>$record['prod_id'],
                      ':fkey_user_id'=>$record['user_id'],
                      ':ip'=>$record['ip'],
                      ':comment'=>$record['comment'],
                      ':notify'=>$record['notify'],
                      ':spam'=>$record['spam']
				);
	  //error_log("about to write these records". print_r($params,true));
      $stmt->execute($params);

	  //Equivalent of a trigger since trigger was not working
	  //Check whether such a post exists and if so increment the comments count
	  /*Create Trigger comments_count_trg
		  AFTER INSERT ON comments
		  FOR EACH ROW
		  BEGIN
		  DECLARE posting_exists INTEGER;

	  SELECT COUNT(*) 
		  INTO posting_exists
		  FROM postings
		  WHERE id = NEW.fkey_post_id;

	  IF posting_exists > 0 THEN
		  UPDATE postings
		  SET comments_count = comments_count + 1
		  WHERE id = NEW.fkey_post_id;
	  END IF
	  END$$ */
	  $result = self::$dbh->query("select count(*) as c, p.user_id as owner_id, u.firstname as owner_name, u.email as owner_mail_id from postings as p,users as u where p.id = ".$record['prod_id']." and u.user_id = p.user_id");
	  $posting_exists = $result->fetchAll(PDO::FETCH_ASSOC);
	  //error_log(print_r($posting_exists,true));
	  $post_exists = ($posting_exists[0]['c'] > 0) ? true : false;
	  if($post_exists){
		  self::$dbh->query("UPDATE postings SET comments_count = comments_count + 1 where id = ".$record['prod_id']);
	  }
	 
	  //Now Trigger notification mail to posting owner and other commentators
	  if($post_exists) {
		  $curr_comment = $record['comment'];
		  $owner_id = $posting_exists[0]['owner_id'];
		  $owner_mail_id = $posting_exists[0]['owner_mail_id'];
		  $owner_name = $posting_exists[0]['owner_name'];

		  $commentator_id = $record['user_id'];
		  $query = "SELECT c.fkey_user_id,u.firstname,u.email FROM comments as c, users as u".
				   " WHERE c.fkey_post_id = ".$record['prod_id'].
				   " AND u.user_id = c.fkey_user_id AND notify=1 AND spam=0 group by c.fkey_user_id";
		  error_log($query);
		  $result = self::$dbh->query($query);
		  $comments = $result->fetchAll(PDO::FETCH_ASSOC);
		  
		  //Go through the comments in a loop to dedupe the comments and find the 
		  //commentator name
		  $notifyList = array();
		  foreach($comments as $comment) {
			  $cuser_id = $comment['fkey_user_id'];
			  //the curr commentator shouldn't get a notification
			  if($cuser_id == $commentator_id) {
				  //get the commentator name 
				  $commentator_name = $comment['firstname'];
				  continue;
			  }

			  if(!isset($notifyList[$cuser_id])) {
				  $notifyList[$cuser_id] = array( 
											'email' => $comment['email'],
											'firstname'=>$comment['firstname']
										   );
			  }
		 }
		 //if owner is not part of notification list and if he is not the commentator , then add him to the list
		 if(!isset($notifyList[$owner_id]) && $owner_id != $commentator_id ) {
			 $notifyList[$owner_id] = array(
										'email' => $owner_mail_id,
										'firstname' => $owner_name
									);
		}

		  if(!empty($notifyList)) { 					//This shouldn't be empty, since this is a new comment 
			  foreach($notifyList as $curr_user_id => $details) {
				  $curr_mail_id = $details['email'];
				  $isOwner = ($owner_id == $curr_user_id) ? true : false;
				  error_log("$curr_mail_id -- $commentator_name -- $curr_comment -- $isOwner");
				  $this->sendNotificationMail($curr_mail_id,$commentator_name,$curr_comment,$isOwner);
				  $notifyDone[] = $curr_user_id;
				  }
			  }
		  }
		  //Notification done here


    } catch (PDOException $e) {
      $this->fatal_error($e->getMessage());
      return false;
    }
    return true;
  }

  function sendNotificationMail($toEmail,$name,$comment,$isOwner) {

	if($isOwner) {
	  $subject = $name." commented on your Cubesales posting";    
	} else {
	  $subject = $name." added a comment after you on the Cubesales posting";    
	}
	$message = $name.":\r\"".$comment."\"\r\rRespond on your Cubesales App";
	$fromEmail = "notify@cubesales.com";
	$ses = new SimpleEmailService('AKIAIONKBSAJGKUYWEGA', 'Lz9DTa6LoNR2tfMabxob9LirHt7qFyB3Zry5gKyb');

	$m = new SimpleEmailServiceMessage();
	$m->addTo($toEmail);
	$m->setFrom($fromEmail);
	$m->setSubject($subject);
	$m->setMessageFromString($message);

	$mail_status = $ses->sendEmail($m);
	if(!$mail_status) {
		echo "Error in the send Email some parameter missing ?";
	}
	return $mail_status;
  }



  function getComments($prod_id) {
    if(!self::$dbh) $this->connect();
    try {
	  $query = "SELECT c.*,u.user_id,u.username,u.company,u.img_url,u.city,u.email from comments as c, users as u where c.fkey_post_id = $prod_id and u.user_id = c.fkey_user_id order by c.mtime desc ";
      $result = self::$dbh->query($query);
      $row = $result->fetchAll(PDO::FETCH_ASSOC);
	  $response = array();
	  $response['status'] = 1;
	  $response['response_data'] = $row;
      return $response;
    } catch (PDOException $e) {
      $this->fatal_error($e->getMessage());
	  return false;
    }
  }
}

class PostingsDb extends db {

  function save($record) {
    if(empty($record['user_id'])) return false;
    if(!self::$dbh) $this->connect();
    try {
      //TODO: update
      $stmt = self::$dbh->prepare('INSERT INTO postings (user_id,category,title,description,price,building_name,photo_url,photo_url_small,audio_url,posting_type) VALUES (:user_id,:category,:title,:description,:price,:building_name,:photo_url,:photo_url_small,:audio_url,:posting_type)');
      $params = array(':user_id'=>$record['user_id'],
                      ':category'=>$record['category'],
                      ':title'=>$record['title'],
                      ':description'=>$record['description'],
                      ':price'=>$record['price'],
                      ':building_name'=>$record['building_name'],
                      ':photo_url'=>$record['photo_url'],
                      ':photo_url_small'=>$record['photo_url_small'],
                      ':audio_url'=>$record['audio_url'],
                      ':posting_type'=>$record['posting_type']
                      );
	  echo "about to write these records". print_r($params,true);
      $stmt->execute($params);
    } catch (PDOException $e) {
      $this->fatal_error($e->getMessage());
    }
    return true;
  }
  
  function simpleQueryExecute($query) {
	  if(!self::$dbh) $this->connect();
	  try {
		  $result = self::$dbh->query($query);
		  $response = array();
		  $response['status'] = 1;
		  return $response;
	  } catch (PDOException $e) {
		  $this->fatal_error($e->getMessage());
		  return false;
	  }
	  return false;
  }

  function createUrlForProd($prod_id) {
	  return "http://cubesales.com/listing/cubesales.php?prod_id=$prod_id";
  }

  function postListingLog($record) {
	  global $_REQUEST;
	  global $_FILES;

	  $file = "/var/www/html/v3.2/upload/post_log";
	  $data = "New Req \n\n".print_r($_REQUEST,true) . print_r($_FILES,true);
	  $status = file_put_contents($file, $data, FILE_APPEND | LOCK_EX);	
	  $response=array();
	  $response['status'] = 1;
	  if(!$status) { return false; }
	  else { return $response;}
  }

  function registerUserTestLog($record) {

	  $file = "/var/www/html/v3.2/log/register_user_log";
	  $data = "New Req \n\n".print_r($record,true);
	  $status = file_put_contents($file, $data, FILE_APPEND | LOCK_EX);	
	  $response=array();
	  $response['status'] = 1;
	  if(!$status) { return false; }
	  else { return $response;}
  }

  function postListing($record) {
	if(empty($record['user_id'])) {
		throw new ErrorException("User id not present");
	}
    if(!self::$dbh) $this->connect();
	
	$file = "../upload/post_log";
	$data = "New Req \n\n".print_r($_REQUEST,true) . print_r($_FILES,true);
	//file_put_contents($file, $data, FILE_APPEND | LOCK_EX);	
	
	$prod_id = isset($record['prod_id'])? $record['prod_id'] : false;
	$ptype = isset($record['req_type'])? $record['req_type'] : "sell_post";
	$price = isset($record['posting_price']) ? $record['posting_price'] : 0;
	
	$posting_status = isset($record['posting_status'])?(int)$record['posting_status']:0;

	//Firstly check whether the posting status is set to sold or delete if so do a simple update
	if($posting_status == 1 || $posting_status == 2){ 
		$u_id = self::$dbh->quote($record['user_id']);
		$query ='UPDATE postings SET posting_status='.$posting_status.
				' WHERE user_id='.$u_id.
				' AND id='.$prod_id;
		return $this->simpleQueryExecute($query);
	}

	if($ptype == "sell_post") {
		$image_name = $this->upload_file("posting_image");
		$image_name_small = $this->upload_file("posting_image_small");
		$audio_name = $this->upload_file("posting_audio");
		$posting_type = 0;
		if($image_name == false) {
			$image_name="";
		}
		if($image_name_small == false) {
			$image_name_small="";
		}
		if($audio_name == false) {
			$audio_name = "";
		}
	}else if($ptype == "seek_post") {
		$image_name = "";
		$posting_type = 1;
	}else if($ptype == "shout_post") {
		$image_name = "";
		$posting_type = 2;
	}

	/*Some fields disabled for mobile apis*/
	$posting_category = isset($record['posting_category'])? $record['posting_category'] : "";
	$posting_title = isset($record['posting_title'])? $record['posting_title'] : "";
	$posting_building_name = isset($record['posting_building_name'])? $record['posting_building_name'] : "";
	$posting_status = isset($record['posting_status'])?(int)$record['posting_status']:0;

    try {
      $params = array(':user_id'=>$record['user_id'],
                      ':category'=>$posting_category,
                      ':title'=>$posting_title,
                      ':description'=>$record['posting_description'],
                      ':price'=>$price,
                      ':building_name'=>$posting_building_name,
                      ':photo_url'=>$image_name,
                      ':photo_url_small'=>$image_name_small,
                      ':audio_url'=>$audio_name,
                      ':posting_type'=>$posting_type,
                      ':posting_status'=>$posting_status
                      );

      if(false === $prod_id) {
          $stmt = self::$dbh->prepare('INSERT INTO postings (user_id,category,title,description,price,building_name,photo_url,photo_url_small,audio_url,posting_type,posting_status) VALUES (:user_id,:category,:title,:description,:price,:building_name,:photo_url,:photo_url_small,:audio_url,:posting_type,:posting_status)');
      }else {
		  $params[':prod_id'] = $prod_id;
		  $stmt = self::$dbh->prepare('UPDATE postings SET category=:category,title=:title,description=:description,price=:price,building_name=:building_name,photo_url=:photo_url,photo_url_small=:photo_url_small,audio_url=:audio_url,posting_type=:posting_type,posting_status=:posting_status WHERE user_id=:user_id AND id=:prod_id');
      }

      $stmt->execute($params);

	  //Now that we have inserted the item into db we will post in his linkedin stream about the product details
	  $consent = isset($record['consent_linkedin']) ? $record['consent_linkedin'] : "0";
      error_log("consent_linkedin value is $consent");
	  if($consent == "1") {
		  $result = self::$dbh->query("SELECT * from postings where postings.id = LAST_INSERT_ID()");
		  $row = $result->fetchAll(PDO::FETCH_ASSOC);
		  $this->expandUrls($row);
		  error_log("** before sharing ". print_r($row,1)); 
		  $prod_url = $this->createUrlForProd($row[0]['id']);

		  $price_types = array("lunch","free","coffee");
		  $price = "sale";
		  if(in_array($row[0]['price'],$price_types)) {
			  $price = $row[0]['price'];
		  }

		  $comment = "listed an item for $price on Cubesales. Check it out !";
		  $image_url = $row[0]['photo_url_small'];
		  $share_content = array ('title' => "post title", 
				  'comment' => $comment, 
				  'submitted-url' => $prod_url, 
				  'submitted-image-url' => $image_url, 
				  'description' => $row[0]['description']);
		  $response = shareToFriendStream("new",$share_content);
		  error_log(" response from share " .print_r($response,true));
	  }	
	  $response = array();
	  $response['status'] = 1;
	  return $response;
    } catch (PDOException $e) {
      $this->fatal_error($e->getMessage());
	  return false;
    }
    return true;
  }


  function load($u_id,$filter_type,$connections=array()) {
    if(!self::$dbh) $this->connect();
    try {
      $u_id = self::$dbh->quote($u_id);
	  
	  if($filter_type == "my") {
		$result = self::$dbh->query("SELECT * from postings,users where users.user_id = postings.user_id and users.user_id = $u_id order by postings.mtime desc");
	  }else if($filter_type = "myc" && !empty($connections)) {
		$conn_in = "( " . implode(",",$connections). " )";
		//Get my connections and get the postings only from my connections
		$result = self::$dbh->query("SELECT * from postings,users where users.user_id in $conn_in and users.user_id = postings.user_id order by postings.mtime desc");
	  } else {
		$result = self::$dbh->query("SELECT * from postings,users where users.user_id = postings.user_id order by postings.mtime desc");
	  }
      $row = $result->fetchAll(PDO::FETCH_ASSOC);
      return $row;
    } catch (PDOException $e) {
      $this->fatal_error($e->getMessage());
	  return false;
    }
    return false;
  }

  function getEmptyResponse() {
	  $response = array('status' => 1, 'response_data' => array());
	  return $response;
  }

  function getUserInfo($args) {
	$response = array();
	if(!isset($args['user_id'])) {
		throw new ErrorException("User id not present");
	}
	$u_id = $args['user_id'];

	/*Instead of getting user details from linked in , get it from db instead
	  This avoids and unnecessary linked in call.
	  @todo : we need to refresh user's facebook/linked in data somewhere*/
		$udb = new UserDb();
		$userInfo = $udb->get_user_details($u_id);	
		if($userInfo) {
			$userInfo['response_data'] = $userInfo['response_data'][0];
		}
		return $userInfo;
	}


  //Only used from mobile app, since the token is obtained from the app
  function getUserListings($args) {
	$response = array();
	if(!isset($args['user_id'])) {
		throw new ErrorException("User id not present");
	}
	$u_id = $args['user_id'];
	$filter_type = isset($args['filter_type'])? $args['filter_type'] : "all";

	/*Instead of getting user details from linked in , get it from db instead
	  This avoids and unnecessary linked in call.
	  @todo : we need to refresh user's facebook/linked in data somewhere*/

	$udb = new UserDb();
	$user_profile = $udb->get_user_details($u_id);	

	//Imp fields extracted
	$company = isset($user_profile['response_data'][0]['company']) ? $user_profile['response_data'][0]['company'] : false;
	$location = isset($user_profile['response_data'][0]['city']) ? $user_profile['response_data'][0]['city'] : false; 

	/*@todo: currently looking up the company from db and not making a call*/
	/*First Get the Linkedin user profile to get his latest company*/
	/*$user_profile = getLinkedInUserProfileForMobile();
	//Imp fields extracted
	$company = isset($user_profile['company']) ? $user_profile['company'] : false;
	$location = isset($user_profile['location']) ? $user_profile['location'] : false; 
	*/

    try {

		if(!self::$dbh) $this->connect();
		$u_id = self::$dbh->quote($u_id);
		$location = self::$dbh->quote($location);
		$company = self::$dbh->quote($company);
		
		switch($filter_type) {

			case "my":
				$result = self::$dbh->query("SELECT  p.*,u.user_id,u.username,u.company,u.img_url,u.headline,u.city from postings as p,users as u where u.user_id = p.user_id and u.user_id = $u_id and p.posting_status != 2 order by p.mtime desc");
				break;

			case "myc":
				$connections = getUserFriends(1000);
				if(empty($connections)) { 
					return $this->getEmptyResponse();
				}
				$conn_in = "( " . implode(",",$connections). " )";
				//Get my connections and get the postings only from my connections
				$result = self::$dbh->query("SELECT p.*,u.user_id,u.username,u.company,u.img_url,u.city from postings as p,users as u where u.user_id in $conn_in and u.user_id = p.user_id and p.posting_status != 2 order by p.mtime desc");
				break;

			case "mycl":
				$connections = getUserFriends(1000);
				if(empty($connections)) { 
					return $this->getEmptyResponse();
				}
				$conn_in = "( " . implode(",",$connections). " )";
				//Get my connections and get the postings only from my connections
				$result = self::$dbh->query("SELECT p.*,u.user_id,u.username,u.company,u.img_url,u.city from postings as p,users as u where u.user_id in $conn_in and u.user_id = p.user_id and u.city = $location and p.posting_status != 2 order by p.mtime desc");
				break;
		
			case "mycmpc":
				$connections = getUserFriends(1000);
				if(!empty($connections)) { 
					$conn_in = "( " . implode(",",$connections). " )";
				}else {
					$conn_in = "('')";
				}
				
				//p.id,p.title,p.price,p.photo_url,p.photo_url_small,p.audio_url,p.description,p.posting_status,p.mtime,p.comments_count,p.num_views
				$query = "SELECT p.*,u.user_id,u.username,u.company,u.img_url,u.city ".
						 "FROM postings as p,users as u ".
						 "WHERE (u.company = $company and u.user_id = p.user_id and p.posting_status != 2) || ".
						 "(u.user_id in $conn_in and u.user_id = p.user_id and p.posting_status != 2) ".
						 "ORDER BY p.mtime desc";
				//error_log($query);
				$result = self::$dbh->query($query);
				break;

			//Currently it has just company listings , location too restrictive
			case "mycmpl":
				$query = "SELECT p.*,u.user_id,u.username,u.company,u.img_url,u.city ".
						 "FROM postings as p,users as u ".
						 "WHERE u.company = $company and u.user_id = p.user_id and p.posting_status != 2 ".
						 "ORDER BY p.mtime desc";
				$result = self::$dbh->query($query);
				break;

			case "mycmploc":
				$query = "SELECT p.*,u.user_id,u.username,u.company,u.img_url,u.city ".
						 "FROM postings as p,users as u ".
						 "WHERE u.company = $company and u.city = $location and u.user_id = p.user_id and p.posting_status != 2 ".
						 "ORDER BY p.mtime desc";
				$result = self::$dbh->query($query);
				break;

			//default should be my company
			default :
				$result = self::$dbh->query("SELECT p.*,u.user_id,u.username,u.company,u.img_url,u.city from postings as p,users as u where u.user_id = p.user_id and p.posting_status != 2 order by p.mtime desc");
		}

		$row = $result->fetchAll(PDO::FETCH_ASSOC);
		$this->expandUrls($row);
		$response['status'] = 1;
		$response['response_data'] = $row;
		return $response;
    } catch (PDOException $e) {
      $this->fatal_error($e->getMessage());
	  return false;
    }
    return false;
  }

  function expandUrls(&$row) {
	  $imgPrefix = "http://cubesales.com/upload/posting_image/";
	  $imgPrefixSmall = "http://cubesales.com/upload/posting_image_small/";
	  $audioPrefix = "http://cubesales.com/upload/posting_audio/";
	  if(!empty($row)) {
		  foreach($row as &$r) {
			  $r['photo_url'] = !empty($r['photo_url']) ? $imgPrefix.$r['photo_url'] : "";
			  $r['photo_url_small'] = !empty($r['photo_url_small']) ? $imgPrefixSmall.$r['photo_url_small'] : "";
			  $r['audio_url'] = !empty($r['audio_url']) ? $audioPrefix.$r['audio_url'] : "";
		  }
	  }
  }

			  

  function getProductDetails($args) {
	$response = array();
	if(!isset($args['prod_id'])) {
		throw new ErrorException("Product id not present");
	}else {
		$prod_id = $args['prod_id'];
	}

    if(!self::$dbh) $this->connect();
    try {
      //$prod_id = self::$dbh->quote($prod_id);
	  $result = self::$dbh->query("SELECT * from postings,users where users.user_id = postings.user_id and postings.id IN ($prod_id)");
      $row = $result->fetchAll(PDO::FETCH_ASSOC);
	  $this->expandUrls($row);
	  $response['status'] = 1;
	  $response['response_data'] = $row;
      return $response;
    } catch (PDOException $e) {
      $this->fatal_error($e->getMessage());
	  return false;
    }
    return false;
  }

 function addComment($record) {
	if(empty($record['user_id']) || empty($record['prod_id'])) {
		throw new ErrorException("User id or prod id not present while adding comments");
	}
	
	$ip = (!empty($_SERVER['REMOTE_ADDR'])) ? $_SERVER['REMOTE_ADDR'] : "na";
	
	$cdb = new CommentsDb();
	$record['notify'] = 1;
	$record['spam'] = 0; 
	$record['ip'] = $ip;
	$status = $cdb->addComment($record);

	if(!$status) {
		throw new ErrorException("Sorry couldn't save your comment, try after sometime");
	}else {
	  $response = array();
	  $response['status'] = 1;
	  return $response;
	}
   }

 function getComments($record) {
	if(empty($record['prod_id'])){
		throw new ErrorException("Prod id not present while getting comments");
	}

	/*Just before getting comments increment the view count for the product 
	 * There is no other place to  increment the count of the product as all the 
	 * product listings are cached in the first call*/
	$query ='UPDATE postings SET num_views=num_views+1'.
			' WHERE id='.$record['prod_id'];
	$status = $this->simpleQueryExecute($query);
	if(!$status) {
		error_log("Incrementing view count failed for query $query, but continuing the execution");
	}
	
	$cdb = new CommentsDb();
	return ($cdb->getComments($record['prod_id']));
 }

 function registerUserTest($record) {
	return $this->registerUserTestLog($record); 
 }

 function registerUser($record) {
	if(empty($record['user_id'])) {
		throw new ErrorException("User id not present");
	}
	//Create the activate key for the user
	$activationKey =  mt_rand() . mt_rand() . mt_rand();

	//Save the user details and set him as not active
	$udb = new UserDb();
	$record['activationKey'] = $activationKey;
	$record['status'] = 0 ; //basic - mail not yet send for activation

	//@todo remove this once fixed
	if(!isset($record['zip'])) { $record['zip'] = '94086'; }
	if(!isset($record['ccode'])) { $record['ccode'] = 'US'; }
	if(!isset($record['hobby'])) { $record['hobby'] = '';}
	if(empty($record['companyid'])) { $record['companyid'] = '-1';}
	error_log(print_r($record,true));
	$status = $udb->save($record);

	if(!$status) {
		throw new ErrorException("Sorry couldn't save your account, try after sometime");
	}else {
		//check whether we should post in linkedin stream about the registration
		$consent = isset($record['update']) ? $record['update'] : "1";
		error_log("consent value is $consent");
		if($consent == "1") {
			error_log(" User has given consent to put registration into stream ");
			$prodLogo = "http://cubesales.com/img/Linkedin_Post.png";
			$prodUrl  = "http://www.cubesales.com/";
			$prodDescription = "Build a trusted & convenient marketplace with your colleagues.";
			//$comment = "started using Cubesales, marketplace app for LinkedIn";
			$comment = "Started using Cubesales Mobile App! Buy, sell & trade at your workplace with trust & convenience";
			$share_content = array ('comment' => $comment, 
					'title' => 'Check out Cubesales',
					'submitted-url' => $prodUrl, 
					'submitted-image-url' => $prodLogo, 
					'description' => $prodDescription);
			$response = shareToFriendStream("new",$share_content);

		}else {
			error_log(" User has NOT given consent to put registration into stream ");
		}
		$response = array();
		$response['status'] = 1;
		return $response;
	}
   }

  function upload_file($fileType) {
	if ((($_FILES[$fileType]["type"] == "image/gif")
				|| ($_FILES[$fileType]["type"] == "image/jpeg")
				|| ($_FILES[$fileType]["type"] == "image/pjpeg")
				|| ($_FILES[$fileType]["type"] == "audio/x-caf"))
			&& ($_FILES[$fileType]["size"] < 20000000))
	{
		if ($_FILES[$fileType]["error"] > 0)
		{
			error_log("Return Code: " . $_FILES[$fileType]["error"] . "<br />");
			return false;
		}
		else
		{
			$ext = "";
			if($fileType == "posting_image") { $ext = ".jpg"; }
			else if($fileType == "posting_image_small") { $ext = ".jpg"; }
			else if($fileType == "posting_audio") { $ext = ".caf"; }
			error_log("Upload: " . $_FILES[$fileType]["name"] . "<br />");
			error_log("Type: " . $_FILES[$fileType]["type"] . "<br />");
			error_log("Size: " . ($_FILES[$fileType]["size"] / 1024) . " Kb<br />");
			error_log("Temp file: " . $_FILES[$fileType]["tmp_name"] . "<br />");
			$randFileName = basename($_FILES[$fileType]["tmp_name"]) . $ext;
			$destFileName = "/var/www/html/upload/$fileType/" . $randFileName;

			if (file_exists($destFileName))
			{
				error_log($destFileName . " already exists. ");
				return false;
			}
			else
			{
				move_uploaded_file($_FILES[$fileType]["tmp_name"],$destFileName);
				error_log("Stored in: " . $destFileName);
				//echo '<img src="./upload/'.$_FILES[$fileType]["name"].'" alt="uploaded image" height="100px" width="100px"/>';
				return $randFileName;
			}
		}
	}
	else
	{
		error_log("Invalid file $fileType in upload");
		return false;
	}
  }
}
