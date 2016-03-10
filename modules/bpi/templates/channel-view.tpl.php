<div class="channel-view">
	<div class="bpi-actions">
		<?php if (bpi_can_edit_channel($channel)) { echo l(t('Edit'), 'admin/bpi/channel/' . $channel->getId() . '/edit'); } ?>
		<?php if (bpi_can_edit_channel($channel)) { echo l(t('Delete'), 'admin/bpi/channel/' . $channel->getId() . '/delete'); } ?>
	</div>

	<h1><?php echo $channel->getName(); ?></h1>

	<div class="description"><?php echo $channel->getDescription(); ?></div>

	<div>
		<?php echo t('Administrator'); ?>: <?php echo l($channel->getAdmin()->getName(), 'admin/bpi/user/' . $channel->getAdmin()->getId()); ?>
	</div>

	<h2><?php echo t('Editors'); ?></h2>

	<p><?php if (bpi_can_edit_channel($channel)) { echo l(t('Add editors'), 'admin/bpi/user', array('query' => array('add-to-channel' => $channel->getId()))); } ?></p>

	<?php echo theme('bpi_user_list', array(
    'items' => $channel->getEditors(),
    'empty' => t('Channel has no editors'),
  )); ?>

	<h2><?php echo t('Content'); ?></h2>

	<p><?php if (bpi_can_contribute_to_channel($channel)) { echo l(t('Add nodes'), 'admin/bpi/'); } ?></p>

	<?php if ($nodes): ?>
		<table>
			<?php foreach ($nodes as $node): $properties = $node->getProperties(); ?>
				<tr>
					<td><?php echo $properties['id']; ?></td>
					<td><?php echo l(t('Preview'), 'admin/bpi/preview/' . 'nojs' . '/' . $properties['id']); ?></td>
					<td><?php echo l(t('Syndicate'), 'admin/bpi/syndicate/' . $properties['id']); ?></td>
					<td><?php echo l(t('Remove'), 'admin/bpi/channel/' . $channel->getId() . '/node/remove/' . $properties['id']); ?></td>
				</tr>
			<?php endforeach ?>
		</table>
	<?php elseif ($channel->getNodes()): ?>
		<table>
			<?php foreach (array_map(function($id) {
				return (object)[
					'id' => $id,
				];
			}, $channel->getNodes()) as $node): ?>
				<tr>
					<td><?php echo $node->id; ?></td>
					<td>
						<?php echo l(t('Preview'), 'admin/bpi/preview/' . 'nojs' . '/' . $node->id); ?>
						<?php echo l(t('Syndicate'), 'admin/bpi/syndicate/' . $node->id); ?>
						<?php echo l(t('Remove'), 'admin/bpi/channel/' . $channel->getId() . '/node/remove/' . $node->id); ?>
					</td>
				</tr>
			<?php endforeach ?>
		</table>
	<?php else: ?>
		<p><?php echo t('No nodes in channel'); ?></p>
	<?php endif ?>
</div>
