<?php
/**
 * @file
 * Single item preview template.
 */
?>
<div class="bpi-item bpi-single-preview-item">
  <h3 class="item-title"><?php echo $item['title']; ?></h3>
  <span class="item-date"><?php echo $item['creation']; ?></span>
  <?php if (isset($item['teaser'])): ?>
    <p class="item-teaser"><?php echo $item['teaser']; ?></p>
  <?php endif ?>
  <?php if (isset($item['body'])): ?>
    <p class="item-body"><?php echo $item['body']; ?></p>
  <?php endif ?>
  <div class="item-filters item-details">
    <div class="details-left">
      <p class="item-details-author">
        <span class="item-details-author-label details-label"><?php echo bpi_label_mapper('author'); ?>: </span>
        <span class="item-details-author-value details-value"><?php echo l($item['author'], 'admin/bpi', array('query' => _bpi_get_filter_query('author', $item['author']))); ?></span>
      </p>
      <p class="item-details-agency">
        <span class="item-details-agency-label details-label"><?php echo bpi_label_mapper('agency'); ?>: </span>
        <span class="item-details-agency-value details-value"><?php echo l($item['agency_name'], 'admin/bpi', array('query' => _bpi_get_filter_query('agency', $item['agency_id']))); ?></span>
      </p>
    </div>
    <div class="details-right">
      <p class="item-details-category">
        <span class="item-details-category-label details-label"><?php echo bpi_label_mapper('category'); ?>: </span>
        <span class="item-details-category-value details-value"><?php echo l($item['category'], 'admin/bpi', array('query' => _bpi_get_filter_query('category', $item['category']))); ?></span>
      </p>
      <p class="item-details-audience">
        <span class="item-details-audience-label details-label"><?php echo bpi_label_mapper('audience'); ?>: </span>
        <span class="item-details-audience-value details-value"><?php echo l($item['audience'], 'admin/bpi', array('query' => _bpi_get_filter_query('audience', $item['audience']))); ?></span>
      </p>
      <?php if (isset($item['material'])): ?>
        <p class="item-details-material">
          <span class="details-label"><?php echo bpi_label_mapper('material'); ?>: </span>
          <span class="details-value"><?php if (!empty($item['material'])): echo implode(', ', (array) $item['material']); endif; ?></span>
        </p>
      <?php endif ?>
      <?php if (isset($item['url'])): ?>
        <p class="item-details-url">
          <span class="details-label"><?php echo bpi_label_mapper('url'); ?>: </span>
          <span class="details-value"><?php echo l($item['url'], $item['url']); ?></span>
        </p>
      <?php endif ?>
      <?php if (isset($item['data']) && is_array($item['data'])): ?>
        <?php foreach ($item['data'] as $key => $value): ?>
          <div class="item-details-<?php echo $key ?>">
            <span class="details-label"><?php echo t($key); ?>: </span>
            <span class="details-value"><?php echo t($value); ?></span>
          </div>
        <?php endforeach ?>
      <?php endif ?>
    </div>
  </div>

  <?php if (!empty($item['assets'])): ?>
    <div class="item-assets">
      <h4><?php echo t('Assets'); ?></h4>
      <?php foreach ($item['assets'] as $type => $assets): ?>
        <?php foreach ($assets as $asset): ?>
          <div class="item-asset">
            <img src="<?php echo $asset['path'] ?>"/><br/>
            <?php echo '(' . t($type) . ')'; ?></header>
          </div>
        <?php endforeach ?>
      <?php endforeach ?>
    </div>
  <?php endif ?>

  <?php if (!$no_actions): ?>
    <p class="item-action item-action-syndicate">
      <?php echo l(t('Syndicate'), 'admin/bpi/syndicate/' . $item['id']); ?>
      <?php echo l(t('Add to channel'), 'admin/bpi/channel/node/add/' . $item['id']); ?>
    </p>
  <?php endif ?>
</div>
