<?php

/**
 * @file
 * Implementation of BPI hooks.
 */

/**
 * Implements hook_bpi_syndicate_get_bpi_types().
 */
function ding_campaign_bpi_syndicate_get_bpi_types() {
  return array('campaign');
}

/**
 * Implements bpi_syndicate_action_url().
 */
function ding_campaign_bpi_syndicate_action_url($bpi_type, $bpi_id, array $mapping) {
  if (in_array($bpi_type, ding_campaign_bpi_syndicate_get_bpi_types())) {
    return url(str_replace('_', '-', 'node/add/' . $mapping['bpi_content_type']), array('query' => array('bpi_type' => $bpi_type, 'bpi_id' => $bpi_id)));
  }

  return NULL;
}

/**
 * Implements hook_bpi_get_field_mapping().
 */
function ding_campaign_bpi_get_field_mapping_form($bpi_type, $content_type, array $field_mapping) {
  if (in_array($bpi_type, ding_campaign_bpi_syndicate_get_bpi_types())) {
    $field_options = array('' => '') + bpi_get_field_instances($content_type);

    return array(
      'bpi_field_title' => array(
        '#type' => 'select',
        '#title' => t('BPI title'),
        '#description' => t('Titles are automatically assigned.'),
        '#options' => array('title' => t('Title')),
        '#default_value' => isset($field_mapping['bpi_field_title']) ? $field_mapping['bpi_field_title'] : NULL,
        '#disabled' => TRUE,
      ),

      // 'bpi_field_teaser' => array(
      //   '#type' => 'select',
      //   '#title' => t('BPI teaser'),
      //   '#description' => t('The field to extract the teaser from. If the content type have a body summary, assign it to the body field.'),
      //   '#options' => $field_options,
      //   '#default_value' => isset($field_mapping['bpi_field_teaser']) ? $field_mapping['bpi_field_teaser'] : NULL,
      // ),

      'bpi_field_body' => array(
        '#type' => 'select',
        '#title' => t('BPI body'),
        '#description' => t('Field to extract the main cotenten from (body field).'),
        '#options' => $field_options,
        '#default_value' => isset($field_mapping['bpi_field_body']) ? $field_mapping['bpi_field_body'] : NULL,
      ),

      'bpi_field_tags' => array(
        '#type' => 'select',
        '#title' => t('BPI tags'),
        '#description' => t(''),
        '#options' => $field_options,
        '#default_value' => isset($field_mapping['bpi_field_tags']) ? $field_mapping['bpi_field_tags'] : NULL,
      ),

      'bpi_field_materials' => array(
        '#type' => 'select',
        '#title' => t('BPI materials'),
        '#description' => t('Field used to get reference to the T!NG data well.'),
        '#options' => $field_options,
        '#default_value' => isset($field_mapping['bpi_field_materials']) ? $field_mapping['bpi_field_materials'] : NULL,
      ),
    );
  }

  return NULL;
}

/**
 * Implements hook_bpi_convert_to_bpi_alter().
 */
function ding_campaign_bpi_convert_to_bpi_alter(&$bpi_content, $node, $mapping) {
  if (isset($mapping['bpi_node_type']) && in_array($mapping['bpi_node_type'], ding_campaign_bpi_syndicate_get_bpi_types())) {
    $bpi_content['type'] = 'campaign';
    $bpi_content['data'] = array(
      'campaign_type' => 'image',
    );
    $bpi_content['assets'] = array(
      array(
        'path' => 'https://placekitten.com/400/300',
        'alt' => '',
        'title' => '',
        'type' => 'image',
        'name' => '400x300',
        'extension' => 'jpg',
      ),
    );
  }
}

/**
 * Implements hook_bpi_syndicate_form().
 */
function ding_campaign_bpi_syndicate_form_alter(&$form, $bpi_content, $assets) {
  $current_language = $form['language']['#value'];
  $data = @json_decode($bpi_content['data'], TRUE);
  $type = isset($data['type']) ? $data['type'] : 'plain';

  if (!in_array($type, array('full', 'image', 'plain'))) {
    $type = 'plain';
  }

  $form['field_camp_settings'][$current_language]['#default_value'] = $type;
  $form['field_camp_link'][$current_language][0]['value']['#default_value'] = isset($bpi_content['url']) ? $bpi_content['url'] : NULL;

  switch ($type) {
    case 'full':
      break;
    case 'image':
      ding_campaign_bpi_place_images($form, $current_language, $assets);
      break;
    case 'plain':
      break;
  }
}

function ding_campaign_bpi_place_images(array &$form, $current_language, $assets) {
  foreach ($assets as $type => $images) {
    if ($type === 'image') {
      $field = &$form['field_camp_image'][$current_language];
      $cardinality = isset($field['#cardinality']) ? $field['#cardinality'] : 1;
      // The cardinality of a field with an unlimited number of values is -1.
      if ($cardinality < 0) {
        $cardinality = PHP_INT_MAX;
      }
      $number_of_images_to_place = min(count($images), $cardinality);
      for ($i = 0; $i < $number_of_images_to_place; $i++) {
        if (is_object($images[$i]['@managed_file'])) {
          $field[$i]['#default_value'] = array('fid' => $images[$i]['@managed_file']->fid);
        }
      }
    }
  }
}
