<?php
/**
 * @file
 * Admin related things here.
 */

/**
 * Creates an administrative block on the configuration page for grouping
 * various settings.
 *
 * @return string
 *   HTML for the block.
 */
function bpi_admin_menu_block_page() {
  $item = menu_get_item();
  $content = system_admin_menu_block($item);

  if ($content) {
    $output = theme('admin_block_content', array('content' => $content));
  }
  else {
    $output = t('You do not have any administrative items.');
  }

  return $output;
}

/**
 * Administrative settings form.
 *
 * @param array $form
 *   Form structure.
 * @param array $form_state
 *   Form state values.
 *
 * @return array
 *   Form structure prepared for admin settings.
 *
 * @ingroup forms
 */
function bpi_admin_settings_form($form, $form_state) {
  $form['bpi_service_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('BPI Service settings'),
  );

  $form['bpi_service_settings']['bpi_agency_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Agency ID'),
    '#description' => t('The 6-digit code representing the library organization'),
    '#default_value' => variable_get('bpi_agency_id', variable_get('ting_agency', '')),
    '#required' => TRUE,
  );

  $form['bpi_service_settings']['bpi_service_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Service URL'),
    '#description' => t('The location of the web-service (http://bpi1.inlead.dk)'),
    '#default_value' => variable_get('bpi_service_url', 'http://bpi1.inlead.dk'),
    '#required' => TRUE,
  );

  $form['bpi_service_settings']['bpi_secret_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Secret key'),
    '#default_value' => variable_get('bpi_secret_key', ''),
    '#required' => TRUE,
  );

  $form['bpi_service_settings']['bpi_api_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Public key'),
    '#default_value' => variable_get('bpi_api_key', ''),
    '#required' => TRUE,
  );

  return system_settings_form($form);
}

/**
 * Implements hook_FROM_ID_validate().
 */
function bpi_admin_settings_form_validate(&$form, &$form_state) {
  // Check if ting agency matches BPI agency, if not give a warning message.
  if (module_exists('ting') && variable_get('ting_agency', FALSE) != $form_state['values']['bpi_agency_id']) {
    drupal_set_message(t('T!NG agency ID %id does not match BPI agency ID', array('%id' => variable_get('ting_agency', FALSE))), 'warning');
  }

  // Check that the service URL is a valid URL.
  $url = $form_state['values']['bpi_service_url'];
  if (!valid_url($url, TRUE)) {
    form_set_error('bpi_service_url', t('Please enter a valid url.'));
  }
  else {
    // Check that the client can connect to the service.
    try {
      $values = $form_state['values'];
      new Bpi($values['bpi_service_url'], $values['bpi_agency_id'], $values['bpi_api_key'], $values['bpi_secret_key']);;
    }
    catch (\Guzzle\Http\Exception\CurlException $exception) {
      // Communication error e.i. not correct end-point.
      form_set_error('bpi_service_url', t($exception->getError()));
    }
    catch (Exception $exception) {
      // Unknown error.
      drupal_set_message(t('Communication with the servies failed. Check the log for more information.'), 'error');
      watchdog_exception('bpi', $exception);
    }

  }
}

/**
 * Map BPI fields to local node fields when syndicating and/or pushing.
 *
 * Since on different installations it's most likely possible that
 * fields will differ from what is to be expected, field mapping
 * functionality is required.
 *
 * The mapping settings from here are used both when syndicating
 * and when pushing content from/to BPI.
 *
 * @param array $form
 *   Form structure.
 * @param array $form_state
 *   Form state values.
 *
 * @return array
 *   Form structure prepared for admin settings.
 *
 * @ingroup forms
 */
function bpi_admin_content_mapping_form($form, &$form_state) {
  $bpi_mappings = bpi_get_mappings();

  $form['#tree'] = TRUE;
  $form['bpi_mappings'] = array(
    '#type' => 'container',
    '#title' => t('BPI content mappings'),
    '#prefix' => '<div id="bpi-wrapper">',
    '#suffix' => '</div>',
  );

  $node_types = node_type_get_names();
  $node_types_names = array_keys($node_types);

  if (empty($form_state['num_mappings'])) {
    $form_state['num_mappings'] = max(1, count($bpi_mappings));
  }
  for ($i = 0; $i < $form_state['num_mappings']; $i++) {
    $default_values = isset($bpi_mappings[$i]) ? $bpi_mappings[$i] : array();
    $field_mapping = isset($default_values['bpi_field_mapping']) ? $default_values['bpi_field_mapping'] : array();
    $selected_node_type = isset($default_values['bpi_content_type']) ? $default_values['bpi_content_type'] : NULL;
    if (isset($form_state['values']['bpi_mappings'][$i]['bpi_content_type'])) {
      $selected_node_type = $form_state['values']['bpi_mappings'][$i]['bpi_content_type'];
    }
    $field_instances = array('' => '') + bpi_get_field_instances($selected_node_type);

    $form['bpi_mappings'][$i] = array(
      '#type' => 'fieldset',
      '#title' => t('Mapping (@bpi_type)', array('@bpi_type' => isset($default_values['bpi_node_type']) ? $default_values['bpi_node_type'] : '–')),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,

      'bpi_node_type' => array(
        '#type' => 'select',
        '#title' => t('BPI node type'),
        '#description' => t('Enter the BPI node type, e.g. ding_news, campaign, carousel.'),
        '#options' => drupal_map_assoc(array(
          'ding_news',
          'ding_campaign',
          'ding_rolltab',
        )),
        '#default_value' => isset($default_values['bpi_node_type']) ? $default_values['bpi_node_type'] : NULL,
        '#required' => TRUE,
      ),

      'bpi_content_type' => array(
        '#type' => 'select',
        '#title' => t('Content type'),
        '#description' => t('Select a content type into which content from BPI will be syndicated.')
        .'<br/>'. t('If you change the content type, you have to save the form before editing the field mapping.'),
        '#options' => $node_types,
        '#default_value' => isset($default_values['bpi_content_type']) ? $default_values['bpi_content_type'] : NULL,
      ),

      'bpi_field_mapping' => array(
        '#type' => 'fieldset',
        '#title' => t('Field mapping'),
        '#description' => t('Define you custom mapping rules. Each dropdown maps BPI related fields to your content fields.'),
        '#prefix' => '<div id="bpi-field-mapper-wrapper">',
        '#suffix' => '</div>',

        'bpi_field_title' => array(
          '#type' => 'select',
          '#title' => t('BPI title'),
          '#description' => t('Titles are automatically assigned.'),
          '#options' => array('title' => t('Title')),
          '#default_value' => isset($field_mapping['bpi_field_title']) ? $field_mapping['bpi_field_title'] : NULL,
          '#disabled' => TRUE,
        ),

        'bpi_field_teaser' => array(
          '#type' => 'select',
          '#title' => t('BPI teaser'),
          '#description' => t('The field to extract the teaser from. If the content type have a body summary, assign it to the body field.'),
          '#options' => $field_instances,
          '#default_value' => isset($field_mapping['bpi_field_teaser']) ? $field_mapping['bpi_field_teaser'] : NULL,
        ),

        'bpi_field_body' => array(
          '#type' => 'select',
          '#title' => t('BPI body'),
          '#description' => t('Field to extract the main cotenten from (body field).'),
          '#options' => $field_instances,
          '#default_value' => isset($field_mapping['bpi_field_body']) ? $field_mapping['bpi_field_body'] : NULL,
        ),

        'bpi_field_materials' => array(
          '#type' => 'select',
          '#title' => t('BPI materials'),
          '#description' => t('Field used to get reference to the T!NG data well.'),
          '#options' => $field_instances,
          '#default_value' => isset($field_mapping['bpi_field_materials']) ? $field_mapping['bpi_field_materials'] : NULL,
        ),
      ),
    );
  }
  $form['bpi_mappings']['add_mapping'] = array(
    '#type' => 'submit',
    '#value' => t('Add mapping'),
    '#submit' => array('bpi_admin_content_mapping_form_add_mapping_add_one'),
    '#ajax' => array(
      'callback' => 'bpi_admin_content_mapping_form_add_mapping_callback',
      'wrapper' => 'bpi-wrapper',
    ),
  );
  if ($form_state['num_mappings'] > 1) {
    $form['bpi_mappings']['remove_mapping'] = array(
      '#type' => 'submit',
      '#value' => t('Remove mapping'),
      '#submit' => array('bpi_admin_content_mapping_form_add_mapping_remove_one'),
      '#ajax' => array(
        'callback' => 'bpi_admin_content_mapping_form_add_mapping_callback',
        'wrapper' => 'bpi-wrapper',
      ),
    );
  }

  $form['#attached']['css'][] = drupal_get_path('module', 'bpi') . '/css/bpi-admin.styles.css';

  return system_settings_form($form);
}

/**
 * Callback for both ajax-enabled buttons.
 */
function bpi_admin_content_mapping_form_add_mapping_callback($form, $form_state) {
  return $form['bpi_mappings'];
}

/**
 * Submit handler for the "Add mapping" button.
 *
 * Increments the counter and causes a rebuild.
 */
function bpi_admin_content_mapping_form_add_mapping_add_one($form, &$form_state) {
  $form_state['num_mappings']++;
  $form_state['rebuild'] = TRUE;
}

/**
 * Submit handler for the "Remove mapping" button.
 *
 * Decrements the counter and causes a form rebuild.
 */
function bpi_admin_content_mapping_form_add_mapping_remove_one($form, &$form_state) {
  if ($form_state['num_mappings'] > 1) {
    $form_state['num_mappings']--;
  }
  $form_state['rebuild'] = TRUE;
}

/**
 * Get a list of fields, for a certain node type.
 *
 * Simplifies and filters the output of the core field_info_instances()
 * function.
 *
 * Filtering means that we do not want text values into image fields, etc.
 *
 * @param string $node_type
 *   Node type machine name, whose fields list is expected.
 *
 * @return array
 *   An array with the fields, for the specified node type.
 */
function bpi_get_field_instances($node_type) {
  if (empty($node_type)) {
    return array();
  }

  $fields = array();
  $allowed_widgets = array(
    'text_textfield',
    'text_textarea',
    'text_textarea_with_summary',
    'ting_reference_simple',
    'ting_reference_advanced',
  );

  $field_instances = field_info_instances('node', $node_type);

  foreach ($field_instances as $instance) {
    if (in_array($instance['widget']['type'], $allowed_widgets)) {
      $fields[$instance['field_name']] = $instance['label'];
    }
  }

  return $fields;
}
