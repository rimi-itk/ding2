<?php

/**
 * @file
 * Theme implementation for a paragraph item displayed as image and text.
 *
 * Available variables:
 * - $content: An array of content items. Use render($content) to print them
 *   all, or print a subset such as render($content['field_example']). Use
 *   hide($content['field_example']) to temporarily suppress the printing of a
 *   given element.
 * - $classes: String of classes that can be used to style contextually through
 *   CSS. It can be manipulated through the variable $classes_array from
 *   preprocess functions. By default the following classes are available, where
 *   the parts enclosed by {} are replaced by the appropriate values:
 *   - entity
 *   - entity-paragraphs-item
 *   - paragraphs-item-{bundle}
 *
 * Other variables:
 * - $classes_array: Array of html class attribute values. It is flattened into
 *   a string within the variable $classes.
 * - $paragraph_styles: Specific styles depending on paragraph type field.
 *
 * @see template_preprocess()
 * @see template_preprocess_entity()
 * @see template_process()
 * @see ddbasic_preprocess_entity_paragraphs_item()
 */

?>

<?php
// @see https://www.computerminds.co.uk/articles/rendering-drupal-7-fields-right-way
$field = field_get_items('paragraphs_item', $paragraphs_item, 'field_studieunivers_url');
$output = field_view_value('paragraphs_item', $paragraphs_item, 'field_studieunivers_url', $field[0]);
$url = render($output);

$field = field_get_items('paragraphs_item', $paragraphs_item, 'field_studieunivers_type');
$output = field_view_value('paragraphs_item', $paragraphs_item, 'field_studieunivers_url', $field[0]);
$type = render($output);
// var_export(array_keys(get_defined_vars()));


function render_field_value($field) {
  $field = ['#label_display' => 'hidden'] + $field;

  return render($field);
}

function print_field_value($field) {
  print render_field_value($field);
}
?>

<div class="<?php print $classes ?? ''; ?> <?php print 'paragraphs-item-studieunivers-link-box-'.$type; ?> <?php print $paragraph_styles ?? ''; ?>">
  <a href="<?php print $url; ?>">
    <div class="paragraphs-block--inner">
      <h1><?php print_field_value($content['field_studieunivers_title']); ?></h1>
      <?php print_field_value($content['field_studieunivers_image']); ?>
      <?php // print render($content); ?>
    </div>
  </a>
</div>
