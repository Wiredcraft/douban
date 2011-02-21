<?php

/**
 * Login
 */
function douban_login() {
  global $user;
  if ($_GET['op'] == 'validate') {
    if ($tok = douban_access_token()) {
      douban_login_handle($tok);
    } else {
      drupal_set_message(t('Validation failed'), 'error');
      drupal_goto('user/login');
    }
    exit;
  }

  if ($to_url = douban_login_url()) {
    drupal_goto($to_url);
  } else {
    drupal_set_message(t('Server did not responsed'), 'error');
    drupal_goto('user/login');
  }
}

/**
 *  Transform the result into an array
 */
function douban_object2array($object) {
  return @json_decode(@json_encode($object),1);
}

/**
 *  User login if exist, or create a new user and login
 */
function douban_login_handle($tok = NULL) {
  global $user;
  $values = array();
  if (isset($tok)) {
    $values['oauth_access_token_key'] = $tok->key;
    $values['oauth_access_token_secret'] = $tok->secret;
    $values['douban_user_id'] = $tok->douban_user_id;
  }

  $douban = drupal_http_request('http://api.douban.com/people/'. $values['douban_user_id'], $headers = array(), $method = 'GET');
  $douban_info = douban_object2array(simplexml_load_string($douban->data));
  $douban_name = $douban_info['title'];
  $douban_uid = $values['douban_user_id'];

  if ($douban->code == 200) {
    if (!empty($douban_name) && !empty($douban_uid)) {
      // If douban_name exists
      $result = db_query("SELECT uid FROM {douban} WHERE douban_name = '%s'", $douban_name);
      if ($ac = db_fetch_array($result)) {
        $account = user_load($ac);
      }

      if (isset($account->uid)) {
        if ($user->uid) {
          if ($user->uid != $account->uid) {
            drupal_set_message(t('username has been used'), 'error');
            drupal_goto('user/'.$user->uid);
          }
        } else {
          user_external_login($account, 'douban');
        }

        if (isset($tok)) {
          db_query("UPDATE {douban} SET douban_uid = '%s', token = '%s' WHERE uid = %d",
          $douban_uid, serialize($tok), $account->uid);
        }
      } else {
        // Create a new user(use differernt name)
        $new_name = $douban_name;
        $i = 1;
        while ($uid = db_result(db_query("SELECT uid FROM {users} WHERE name = '%s'", $new_name))) {
          $new_name = $douban_name.'_'.$i;
          ++$i;
        }
        $pass = user_password();
        $ac = array(
          'name' => $new_name,
          'mail' => $new_name . '@douban', // Use @douban as email address
          'pass' => $pass,
          'status' => variable_get('user_register', 1) == 1,
          'auth_douban' => $new_name
        );

        if (!$account = user_save('', $ac)) {
          drupal_set_message('Creating user failed', 'error');
          drupal_goto('user/register');
        }

        db_query("INSERT INTO {douban} (uid, douban_uid, douban_name, token) VALUES ('%s', '%s', '%s', '%s')",
        $account->uid, $douban_uid, $douban_name, serialize($tok));
        drupal_set_message(t('A temporary account has been created, password: @pass<br /> Remember this password if you want to use it in the future, or change it in the following', array('@pass'=>$pass)));
        user_external_login($account);
      }

      if (isset($_REQUEST['redirect']) && $_REQUEST['redirect']) {
        drupal_goto($_REQUEST['redirect']);
      } else {
        drupal_goto('user/'.$account->uid.'/edit');
      }
    }
  }

  drupal_set_message(t('Try it again'), 'error');
  drupal_goto('user/login');
}