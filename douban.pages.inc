<?php

/**
 * Login
 */
function douban_login() {
  global $user;
  if ($user->uid) {
    if ($tok = douban_get_access_token($user->uid)){
          drupal_set_message(t('Authorizetion success'), status);
          drupal_goto('user/'. $user->uid);
    }
  }

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
      if ($douban_user = db_fetch_array($result)) {
        $douban_usercount = user_load($douban_user);
      }

      if (isset($douban_usercount->uid)) {
        if ($user->uid) {
          if ($user->uid != $douban_usercount->uid) {
            drupal_set_message(t('username has been used'), 'error');
            drupal_goto('user/'. $user->uid);
          }
        } else {
          user_external_login($douban_usercount, 'douban');
        }

        if (isset($tok)) {
          db_query("UPDATE {douban} SET douban_uid = '%s', token = '%s' WHERE uid = %d",
          $douban_uid, serialize($tok), $douban_usercount->uid);
        }
      } else if ($user->uid > 0) {
        if (db_result(db_query('SELECT uid FROM {douban} WHERE uid = %d', $user->uid))) {
          drupal_get_message(t('You already had binded Douban account', 'error'));
          drupal_goto('user/' . $user->uid);
        } else {
          $account = $user;
          db_query("INSERT INTO {douban} (uid, douban_uid, douban_name, token) VALUES ('%s', '%s', '%s', '%s')",
          $user->uid, $douban_uid, $douban_name, serialize($tok));
        }
      }

      else {
        // Create a new user(use differernt name)
        $new_name = $douban_name;
        $i = 1;
        while ($uid = db_result(db_query("SELECT uid FROM {users} WHERE name = '%s'", $new_name))) {
          $new_name = $douban_name.'_'.$i;
          ++$i;
        }
        $pass = user_password();
        $douban_user = array(
          'name' => $new_name,
          'mail' => $new_name . '@douban', // Use @douban as email address
          'pass' => $pass,
          'status' => variable_get('user_register', 1) == 1,
          'auth_douban' => $new_name
        );

        if (!$douban_usercount = user_save('', $douban_user)) {
          drupal_set_message('Creating user failed', 'error');
          drupal_goto('user/register');
        }

        db_query("INSERT INTO {douban} (uid, douban_uid, douban_name, token) VALUES ('%s', '%s', '%s', '%s')",
        $douban_usercount->uid, $douban_uid, $douban_name, serialize($tok));
        drupal_set_message(t('A temporary account has been created, password: @pass<br /> Remember this password if you want to use it in the future, or change it at' . '', array('@pass'=>$pass)));
        user_external_login($douban_usercount);
      }

      if (isset($_REQUEST['redirect']) && $_REQUEST['redirect']) {
        drupal_goto($_REQUEST['redirect']);
      } else {
        drupal_goto('user/'. $user->uid .'/douban/config');
      }
    }
  }

  drupal_set_message(t('Try it again'), 'error');
  drupal_goto('user/login');
}

/**
 * User settings
 */
function douban_user_config($douban_user) {
  $output = '';
  $data = db_fetch_object(db_query('SELECT uid, token, douban_name FROM {douban} WHERE uid = %d', $douban_user->uid));
  if ($data->uid) {
    $items[] = t('Your douban account: '). $data->douban_name
    . l(t(' [Unbind your douban account]'), 'user/'.$douban_user->uid.'/douban/unbind', array('attributes' => array('class' => 'douban_user_unbind')));
    if ($GLOBALS['user']->uid == $douban_user->uid) {
      $items[] = drupal_get_form('douban_send_miniblog_form', $douban_user->uid);
    }
  } else {
    $items[] = 'Douban account has not been binded, ' .
    l(t('login your Douban account now'), 'douban/login');
  }
  $output = theme('item_list', $items, 'Douban', 'ul', array('id' => 'douban_user_config'));
  return $output;

  if ($data = douban_get_info($douban_user)) {
    if ($data->douban_name) {
      $items[] = t('Your Douban account: '). $data->douban_name;
      $output = theme('item_list', $items);
    }
  } else {
  }
  return $output;
}

/**
 * Confirm unbind form
 */
function douban_unbind_confirm($form_state, $douban_user) {
  if ($data = db_fetch_object(db_query('SELECT uid, token, douban_name FROM {douban} WHERE uid = %d', $douban_user->uid))) {
    $form = array();
    $form['uid'] = array('#type' => 'hidden', '#value' => $douban_user->uid);
    $output = confirm_form($form,
                    t('Unbind douban account'),
                    'user/'.$douban_user->uid.'/douban/config',
                    'current account: '.$data->douban_name . ', are you sure you want to do that?',
                    t('Unbind'),
                    t('Cancel'));
    return $output;
  } else {
    drupal_access_denied();
    exit;
  }
}

function douban_unbind_confirm_submit($form, &$form_state) {
  db_query('DELETE FROM {douban} WHERE uid = %d', $form_state['values']['uid']);
  db_query("DELETE FROM {authmap} WHERE uid = %d AND module = '%s'", $form_state['values']['uid'], 'douban');
  drupal_set_message(t('Unbind success!'));
  $form_state['redirect'] = 'user/'.$form_state['values']['uid'].'/douban/config';
  return;
}