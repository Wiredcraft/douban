<?php
/**
 * @file
 * Admin settings page for the douban module.
 */

/**
 * Setting API key and secret key at settings page.
 */
function douban_admin_settings() {
  $module_path = drupal_get_path('module', 'douban') .'/images/';
  $form = array();
  $form['douban_consumer_key'] = array(
    '#type' => 'textfield',
    '#title' => t('API key'),
    '#default_value' => variable_get('douban_consumer_key', ''),
    '#description' => t('Consumer key from your application settings page.'),
  );
  $form['douban_consumer_secret'] = array(
    '#type' => 'textfield',
    '#title' => t('Secret Key'),
    '#default_value' => variable_get('douban_consumer_secret', ''),
    '#description' => t('Consumer secret from your application settings page.'),
  );
  $form['douban_button'] = array(
    '#type' => 'fieldset',
    '#title' => t('Douban Button'),
  );
  $form['douban_button']['image'] = array(
    '#type' => 'item',
    '#value' =>  theme_image($module_path .'douban_button.gif'),
  );
  return system_settings_form($form);
}

/**
 * List douban account
 */
function douban_admin_users() {
  $result = pager_query('SELECT u.uid, u.name FROM {douban} s INNER JOIN {users} u ON s.uid = u.uid ORDER BY u.uid DESC', 50);
  $table = NULL;
  while ($o = db_fetch_object($result)) {
    $destination = array('query' => drupal_get_destination());
    $table[] = array(
      theme('username', $o),
      l(t('Unbind user'), 'user/'.$o->uid.'/douban/unbind', $destination),
      l(t('Delete user'), 'user/'.$o->uid.'/delete', $destination)
    );
  }
  if ($table) {
    $output =  theme('table', array(), $table);
    $output .= theme('pager');
  } else {
    $output = t('No Douban account');
  }

  return $output;
}