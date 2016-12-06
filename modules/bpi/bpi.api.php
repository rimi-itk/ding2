<?php

/**
 * @file
 * Describe hooks provided by the BPI module.
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * Return a list of BPI node type a module can handle.
 *
 * @return array
 *   An array of BPI type names.
 */
function hook_bpi_syndicate_get_bpi_types() {
}

function hook_bpi_get_field_mapping_form($bpi_type, $content_type, array $field_mapping) {
}

function hook_bpi_mapping_form_alter($bpi_type, array &$form_item) {
}

function hook_bpi_syndicate_action_url($bpi_type, $bpi_id, array $mapping) {
}

/**
 * Alter data to be pushed to BPI.
 *
 * @param array $bpi_content
 *   The BPI content.
 * @param array $node
 *   The node..
 * @param array $mapping
 *   The BPI mapping of the node.
 */
function hook_bpi_convert_to_bpi_alter(array &$bpi_content, $node, array $mapping) {
}

/**
 * @} End of "addtogroup hooks".
 */
