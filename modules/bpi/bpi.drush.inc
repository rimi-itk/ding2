<?php
/**
 * @file
 * Drush integration for the bpi module.
 */

/**
 * Implements hook_drush_command().
 */
function bpi_drush_command() {
  $items = array();

  $items['bpi-user-set-bpi-user-id'] = array(
    'description' => 'Set bpi user id for a user.',
    'arguments' => array(
      'username' => 'The username.',
      'bpi-user-id' => 'The bpi user id.',
    ),
    // Argument "username" is required.
    'required-arguments' => 1,
  );

  return $items;
}

/**
 * Command callback. Set the bpi user id for a user.
 *
 * @param string $username
 *   The username.
 * @param string $bpi_user_id
 *   The bpi user id.
 *
 * @return bool
 *   Success.
 */
function drush_bpi_user_set_bpi_user_id($username, $bpi_user_id = NULL) {
  $account = user_load_by_name($username);
  if (!$account) {
    return drush_log('No such user: ' . $username, 'error');
  }
  user_save($account, array('data' => array('bpi_user_id' => $bpi_user_id)));

  return TRUE;
}
