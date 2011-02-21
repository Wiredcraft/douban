<?php
/**
 * @file
 * Admin settings page for the douban module.
 */

/**
 * Setting API key and secret key at settings page.
 */
function douban_admin_settings() {
//  global $base_url;
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