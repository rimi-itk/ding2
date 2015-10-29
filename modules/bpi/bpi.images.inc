<?php
/**
 * @file
 * Images syndication routine here.
 */

/**
 * Entry point for image syndication.
 *
 * @param string $type
 *   Whether it's an ajax call.
 *
 * @return array
 *   A set of ajax commands.
 */
function bpi_syndicate_images($type = 'ajax') {
  $assets = isset($_SESSION['bpi']['assets']) ? $_SESSION['bpi']['assets'] : array();

  if (empty($assets)) {
    return;
  }

  $form = drupal_get_form('bpi_assets_form', $assets);
  $commands = array();

  $commands[] = ajax_command_ding_popup(
    'bpi-syndicate-images',
    t('Syndicate images'),
    drupal_render($form)
  );

  return array('#type' => 'ajax', '#commands' => $commands);
}

/**
 * Image syndication form.
 *
 * @param array $form
 *   Form structure.
 * @param array $form_state
 *   Form state values.
 * @param array $assets
 *   Array of assets to be downloaded.
 *
 * @return array
 *   Form structure.
 *
 * @ingroup forms
 */
function bpi_assets_form($form, $form_state, $assets = array()) {
  if (!empty($assets)) {
    $form['bpi_assets'] = array(
      '#type' => 'fieldset',
      '#title' => t('Available images'),
    );

    foreach ($assets as $type => $items) {
      foreach ($items as $asset) {
        $form['bpi_assets']['asset::' . $type . '::' . $asset['name']] = array(
          '#type' => 'checkbox',
          '#default_value' => 1,
          '#prefix' => '<div class="bpi-syndicate-asset"><img src="' . $asset['path'] . '" width="100" height="100" />' . '<br/> (' . bpi_get_image_name($asset['type']) . ')',
          '#suffix' => '</div>',
        );
      }
    }
  }

  $form['bpi_assets_syndicate'] = array(
    '#type' => 'submit',
    '#value' => t('Import'),
    '#ajax' => array(
      'callback' => 'bpi_asset_action_ajax_callback',
    ),
  );

  return $form;
}

/**
 * Submit handler for images form.
 *
 * @see bpi_assets_form()
 *
 * @param array $form
 *   Form structure.
 * @param array $form_state
 *   Form state values.
 *
 * @ingroup forms
 */
function bpi_assets_form_submit($form, &$form_state) {
  $bpi_assets = $_SESSION['bpi']['assets'];

  $assets = array();

  // Seek the assets.
  foreach ($form_state['input'] as $name => $value) {
    if (preg_match('/^asset::(?<type>[^:]+)::(?<name>[^_]+)$/', $name, $matches) && !empty($value)) {
      $type = $matches['type'];
      $name = $matches['name'];
      if (isset($bpi_assets[$type][$name])) {
        $assets[] = $bpi_assets[$type][$name];
      }
    }
  }

  unset($_SESSION['bpi']['assets']);
  bpi_clear_syndicated_images();

  // Group syndicated images by type.
  $syndicated_images = array();
  $status = TRUE;
  foreach ($assets as $asset) {
    $filename = $asset['name'] . '.' . $asset['extension'];
    $managed_file = bpi_fetch_image('public://bpi/' . $filename, $asset['path']);

    if (!isset($syndicated_images[$asset['type']])) {
      $syndicated_images[$asset['type']] = array();
    }
    $syndicated_images[$asset['type']][] = $asset + array('@managed_file' => $managed_file);

    if (!is_object($managed_file)) {
      $status = FALSE;
    }
  }

  if (isset($status) && $status) {
    drupal_set_message(t('Images downloaded successfully.'));
    // Store syndicated images for use in edit form.
    bpi_set_syndicated_images($syndicated_images);

    if (isset($_SESSION['bpi']['destination'])) {
      $url = drupal_parse_url($_SESSION['bpi']['destination']['destination']);

      // @see http://drupal.stackexchange.com/a/86593
      $redirect_url = $url['path'];
      ctools_include('ajax');
      $commands[] = ctools_ajax_command_redirect($redirect_url, 0, array(
        'query' => array_merge($url['query'], array(
          'bpi_images_syndicated' => 1,
        )),
      ));
      print ajax_render($commands);
      exit;
    }
  }
  else {
    drupal_set_message(t('Not all images were fetched successfully, check reports for more info.'));
  }
}

/**
 * Images form download action ajax callback.
 *
 * @see bpi_assets_form()
 *
 * @param array $form
 *   Form structure.
 * @param array $form_state
 *   Form state values.
 *
 * @return array
 *   Set of ajax commands.
 *
 * @ingroup forms
 */
function bpi_asset_action_ajax_callback($form, &$form_state) {
  $html = theme('status_messages');

  $commands = array();

  $commands[] = ajax_command_ding_popup(
    'bpi-syndicate-images',
    t('Syndicate images'),
    $html
  );

  return array('#type' => 'ajax', '#commands' => $commands);
}

/**
 * Images form cancel action ajax callback.
 *
 * @see bpi_assets_form()
 *
 * @param array $form
 *   Form structure.
 * @param array $form_state
 *   Form state values.
 *
 * @return array
 *   Set of ajax commands.
 *
 * @ingroup forms
 */
function bpi_asset_cancel_ajax_callback($form, &$form_state) {
  $commands = array();

  $commands[] = ajax_command_ding_popup_close(
    'bpi-syndicate-images'
  );

  return array('#type' => 'ajax', '#commands' => $commands);
}

/**
 * Download images to local instance.
 *
 * @param string $filename
 *   Filename, with extension. An URI is optional.
 * @param string $image_url
 *   File URL form where to download.
 *
 * @return object
 *   File object or FALSE on failure.
 */
function bpi_fetch_image($filename, $image_url) {
  $result = drupal_http_request($image_url);

  if ($result->code != 200) {
    watchdog(
      'bpi',
      'Failed to fetch image %url, with HTTP status %status', array('%url' => $image_url, '%status' => $result->code),
      WATCHDOG_ERROR
    );

    return FALSE;
  }

  $directory = dirname($filename);

  if (!file_prepare_directory($directory, FILE_CREATE_DIRECTORY | FILE_MODIFY_PERMISSIONS)) {
    watchdog(
      'bpi',
      t('Failed to create directory: %directory'),
      array('%directory' => $directory),
      WATCHDOG_ERROR
    );

    return FALSE;
  }

  return file_save_data($result->data, $filename, FILE_EXISTS_REPLACE);
}


/**
 * Compute the image type from a field name on a node.
 *
 * @param string $image_field_name
 *   The image field name.
 * @param array $node
 *   The node.
 *
 * @return NULL|string
 *   The image type.
 */
function bpi_get_image_type($image_field_name, $node) {
  /*
   * We assume that the image type is the last part of the field name, i.e.
   *   field_{{node type}}_{{image type}}
   */
  $pattern = '/field_' . preg_quote($node->type, '/') . '_(?<image_type>.+)/';
  if (preg_match($pattern, $image_field_name, $matches)) {
    return $matches['image_type'];
  }
  return NULL;
}

/**
 * Decide if the given field is a container for the given bpi image type.
 *
 * @param string $field_name
 *   The field name.
 * @param array $node
 *   The node.
 * @param string $image_type
 *   The bpi image type.
 *
 * @return bool
 *   True if the field is a container for the the bpi image type.
 */
function bpi_is_bpi_image_field($field_name, $node, $image_type) {
  return bpi_get_image_type($field_name, $node) == $image_type;
}

function bpi_set_syndicated_images(array $syndicated_images) {
  $_SESSION['bpi']['syndicated_images'] = $syndicated_images;
}

function bpi_get_syndicated_images() {
  return isset($_SESSION['bpi']['syndicated_images']) ? $_SESSION['bpi']['syndicated_images'] : NULL;
}

function bpi_clear_syndicated_images() {
  unset($_SESSION['bpi']['syndicated_images']);
}

/**
 * Place syndicated images in node edit form.
 *
 * @param array $form
 *   The form.
 * @param string $current_language
 *   The current language.
 * @param array $syndicated_images
 *   The syndicated images.
 */
function bpi_place_images(array &$form, $current_language, array $syndicated_images) {
  $node = $form['#node'];
  $image_field_names = bpi_fetch_image_fields($node->type);
  if (!empty($image_field_names)) {
    foreach ($syndicated_images as $image_type => $images) {
      foreach ($image_field_names as $image_field_name) {
        if (bpi_is_bpi_image_field($image_field_name, $node, $image_type)) {
          $field = &$form[$image_field_name][$current_language];
          $cardinality = isset($field['#cardinality']) ? $field['#cardinality'] : 1;
          $number_of_images = count($images);
          for ($i = 0; $i < min($cardinality, $number_of_images); $i++) {
            if (is_object($images[$i]['@managed_file'])) {
              $field[$i]['#default_value'] = array('fid' => $images[$i]['@managed_file']->fid);
            }
          }
        }
      }
    }
  }
}

function bpi_get_image_name($image_type) {
  $types = array(
    'body' => t('body_image'),
    'list_image' => t('list_image'),
    'title_image' => t('title_image'),
  );
  return isset($types[$image_type]) ? $types[$image_type] : t($image_type);
}