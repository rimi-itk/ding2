<?php
/**
 * @file
 * Handling of BPI channels.
 */

use Bpi\Sdk\Item\Channel;
use Bpi\Sdk\Item\User;

// We want to reuse some helper functions.
require_once __DIR__ . '/bpi.syndicate.inc';

function bpi_channel_list() {
  return array(
    'bpi_channel_subscription_form' => drupal_get_form('bpi_channel_subscription_form'),
    'bpi_channel_list_form' => drupal_get_form('bpi_channel_list_form'),
  );
}

/**
 * Create form for creating/deleting channel subscriptions.
 */
function bpi_channel_subscription_form() {
  return array(
    '#type' => 'form',
    '#action' => '/admin/bpi/channel/subscription',
    '#id' => 'bpi-channel-subscription-form',
    'channel_id' => array(
      '#type' => 'hidden',
      '#value' => '',
    ),
    'command' => array(
      '#type' => 'hidden',
      '#value' => '',
    ),
    'redirect' => array(
      '#type' => 'hidden',
      '#value' => request_uri(),
    ),
    'script' => array(
      '#type' => 'item',
      '#markup' => <<<JS
<script>
var BpiChannel = {
  subscribe: function(channelId) {
    var form = document.getElementById('bpi-channel-subscription-form');
    form.command.value = 'subscribe';
    form.channel_id.value = channelId;
    form.submit();
  },
  unsubscribe: function(channelId) {
    var form = document.getElementById('bpi-channel-subscription-form');
    form.command.value = 'unsubscribe';
    form.channel_id.value = channelId;
    form.submit();
  }
};
</script>
JS
    ),
  );
}

/**
 * Define channels page markup.
 *
 * @return array
 *   Form structure.
 *
 * @ingroup forms
 */
function bpi_channel_list_form(array $form, array &$form_state, $user_id = NULL) {
  bpi_check_bpi_user_id();

  if (bpi_service_is_configured()) {
    drupal_add_library('system', 'drupal.ajax');

    $items = bpi_get_channels($user_id);

    $total_items = $items['total_count'];
    $page_size = _bpi_get_query_parameter('amount', BPI_DEFAULT_PAGE_SIZE);
    pager_default_initialize($total_items, $page_size);

    $result_form = array(
      '#type' => 'container',
      '#attributes' => array(
        'class' => array(
          'bpi-result',
        ),
      ),
    );

    if (!empty($items['channels'])) {
      $result_form['bpi_search_info'] = array(
        '#type' => 'container',
        '#attributes' => array(
          'class' => array('bpi-search-info', 'container-inline'),
        ),
      );

      $current_page = pager_find_page();
      $items_from = $current_page * $page_size + 1;
      $items_to = min(($current_page + 1) * $page_size, $total_items);

      $result_form['bpi_search_info']['bpi_search_paging_info'] = array(
        '#type' => 'item',
        '#markup' => ($items_from < $items_to
                      ? t('Showing items @from–@to of @total', array(
                        '@from' => $items_from,
                        '@to' => $items_to,
                        '@total' => $total_items,
                      ))
                      : t('Showing item @from of @total', array(
                        '@from' => $items_from,
                        '@total' => $total_items,
                      ))),
      );

      $result_form['bpi_search_info']['bpi_search_amount'] = array(
        '#type' => 'select',
        '#title' => t('Page size'),
        '#options' => drupal_map_assoc(array(10, 20, 50, 100)),
        '#default_value' => _bpi_get_query_parameter('amount', BPI_DEFAULT_PAGE_SIZE),
        '#attributes' => array(
          'onchange' => 'form.submit()',
        ),
      );
    }
    else {
      $result_form['bpi_search_amount'] = array(
        '#type' => 'hidden',
      );
    }

    $result_form['bpi_search_results'] = array(
      '#type' => 'item',
      '#markup' => theme('bpi_channel_list', array(
        'items' => $items['channels'],
        'empty' => t('No results'),
      )),
    );

    $result_form['bpi_search_pager'] = array(
      '#type' => 'item',
      '#markup' => theme('pager'),
    );

    $query_form['loading'] = array(
      '#type' => 'item',
      '#markup' => '<div class="loading"><div class="message">' . t('Please wait') . '</div></div>',
    );

    $form['bpi_actions'] = array(
      '#type' => 'container',
      '#attributes' => array(
        'class' => array(
          'action-links',
        ),
      ),
    );

    $form['bpi_query'] = $query_form;
    $form['bpi_result'] = $result_form;

    $form['#attributes']['class'][] = 'bpi';
    $form['#attached']['css'][] = drupal_get_path('module', 'bpi') . '/css/bpi-admin.styles.css';
    $form['#attached']['js'][] = drupal_get_path('module', 'bpi') . '/js/bpi-admin.js';
    $form['#method'] = 'get';
  }

  return $form;
}

/**
 * Show channel details.
 *
 * @param string $channel_id
 *   The channel id.
 *
 * @return string
 *   The rendered view.
 */
function bpi_channel_view($channel_id) {
  $channel = bpi_get_channel($channel_id);

  if (!$channel) {
    return '';
  }

  $bpi = bpi_client_instance();
  $nodes = $bpi->searchNodes(array(
    'filter' => array(
      'channels' => array($channel->getId()),
    ),
  ));

  return theme('bpi_channel_view', array(
    'channel' => $channel,
    'nodes' => bpi_nodelist_to_array($nodes),
  ));
}

/**
 * Channel edit form. Also used for creating new channels.
 *
 * @param array $form
 *   The form.
 * @param array $form_state
 *   The form state.
 *
 * @return array
 *   The form.
 */
function bpi_channel_edit_form(array $form, array &$form_state) {
  $channel_id = isset($form_state['build_info']['args'][0]) ? $form_state['build_info']['args'][0] : NULL;

  if ($channel_id) {
    // Edit channel.
    $channel = bpi_get_channel($channel_id);

    if (!bpi_can_edit_channel($channel)) {
      drupal_set_message(t("You're not allowed to edit this channel"), 'error');
      return $form;
    }
  }

  $form['channel']['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#default_value' => isset($channel) ? $channel->getName() : NULL,
    '#required' => TRUE,
  );

  $form['channel']['description'] = array(
    '#type' => 'textarea',
    '#title' => t('Description'),
    '#default_value' => isset($channel) ? $channel->getDescription() : NULL,
    '#required' => TRUE,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => $channel_id ? t('Save') : t('Create channel'),
  );

  $form['cancel'] = array(
    '#type' => 'markup',
    '#markup' => l(t('Cancel'), 'admin/bpi/channel/' . (isset($channel) ? $channel->getId() : '')),
  );

  $form['#submit'][] = 'bpi_channel_edit_form_submit';

  $form['#attributes']['class'][] = 'bpi';
  $form['#attached']['css'][] = drupal_get_path('module', 'bpi') . '/css/bpi-admin.styles.css';
  $form['#attached']['js'][] = drupal_get_path('module', 'bpi') . '/js/bpi-admin.js';

  return $form;
}

/**
 * Submit handler for bpi_channel_edit_form.
 *
 * @param array $form
 *   The form.
 * @param array $form_state
 *   The form state.
 */
function bpi_channel_edit_form_submit(array $form, array &$form_state) {
  $channel_id = isset($form_state['build_info']['args'][0]) ? $form_state['build_info']['args'][0] : NULL;

  $values = $form_state['values'];

  $channel_data = array(
    'name' => $values['name'],
    'description' => $values['description'],
  );

  if (!$channel_id) {
    $channel_data['adminId'] = bpi_get_current_bpi_user_id();
    try {
      $bpi = bpi_client_instance();
      $channel = $bpi->createChannel($channel_data);
      drupal_set_message(t('Channel created.'));
      $channel_id = $channel->getId();
    }
    catch (\Exception $e) {
      watchdog_exception('bpi', $e);
      drupal_set_message(t('Channel creation failed.'), 'error');
    }
  }
  else {
    try {
      $bpi = bpi_client_instance();
      $bpi->updateChannel($channel_id, $channel_data);
      drupal_set_message(t('Channel updated.'));
    }
    catch (\Exception $e) {
      watchdog_exception('bpi', $e);
      drupal_set_message(t('Channel update failed.'), 'error');
    }
  }

  drupal_goto('admin/bpi/channel/' . $channel_id);
}

/**
 * Delete channel form.
 *
 * @param array $form
 *   The form.
 * @param array $form_state
 *   The form state.
 *
 * @return array
 *   The form.
 */
function bpi_channel_delete_form(array $form, array &$form_state) {
  $channel_id = isset($form_state['build_info']['args'][0]) ? $form_state['build_info']['args'][0] : NULL;

  $channel = bpi_get_channel($channel_id);

  if (!bpi_can_edit_channel($channel)) {
    drupal_set_message(t("You're not allowed to edit this channel"), 'error');
    return $form;
  }

  return confirm_form($form,
    t('Are you sure you want to delete %title?', array('%title' => $channel->getName())),
    'admin/bpi/channel/' . $channel->getId(),
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Submit handler for bpi_channel_delete_form.
 *
 * @param array $form
 *   The form.
 * @param array $form_state
 *   The form state.
 */
function bpi_channel_delete_form_submit(array $form, array &$form_state) {
  $channel_id = isset($form_state['build_info']['args'][0]) ? $form_state['build_info']['args'][0] : NULL;

  try {
    $bpi = bpi_client_instance();
    $success = $bpi->deleteChannel($channel_id);
    if ($success) {
      drupal_set_message(t('Channel deleted.'));
      drupal_goto('admin/bpi/channel');
    }
    else {
      drupal_set_message(t('Deleting channel failed.'), 'error');
    }
  }
  catch (\Exception $e) {
    watchdog_exception('bpi', $e);
    drupal_set_message(t('Deleting channel failed.'), 'error');
  }
}

function bpi_channel_subscription() {
  $user = bpi_get_user();
  $command = isset($_POST['command']) ? $_POST['command'] : NULL;
  $channel_id = isset($_POST['channel_id']) ? $_POST['channel_id'] : NULL;
  $redirect = isset($_POST['redirect']) ? $_POST['redirect'] : 'admin/bpi/channel';

  $channel = bpi_get_channel($channel_id);
  if (!$channel) {
    drupal_set_message(t('No such channel: %channel', array('%channel' => $channel_id)), 'error');
    return drupal_goto($redirect);
  }

  $title = _bpi_get_channel_subscription_title($user, $channel);

  switch ($command) {
    case 'subscribe':
      try {
        $bpi = bpi_client_instance();
        $bpi->createSubscription($user->getId(), array(
            'title' => $title,
            'filter' => array(
              'channels' => array($channel->getId()),
            ),
          ));
        drupal_set_message(t('Subscription on channel %channel created', array('%channel' => $channel->getName())));
      }
      catch (\Exception $e) {
        watchdog_exception('bpi', $e);
        drupal_set_message(t('Cannot subscribe to channel %channel', array('%channel' => $channel->getName())), 'error');
      }
      break;

    case 'unsubscribe':
      try {
        $bpi = bpi_client_instance();
        $bpi->deleteSubscription($user->getId(), $title);
        drupal_set_message(t('Subscription on channel %channel deleted', array('%channel' => $channel->getName())));
      }
      catch (\Exception $e) {
        watchdog_exception('bpi', $e);
        drupal_set_message(t('Cannot unsubscribe from channel %channel', array('%channel' => $channel->getName())), 'error');
      }
      break;

    default:
      break;
  }

  return drupal_goto($redirect);
}

/**
 * Create a subscription to a channel for the current user.
 *
 * @param string $channel_id
 */
function bpi_channel_subscribe($channel_id) {
  $user = bpi_get_user();
  $channel = bpi_get_channel($channel_id);
  if (!$channel) {
    drupal_set_message(t('No such channel: %channel', array('%channel' => $channel_id)), 'error');
    return drupal_goto('admin/bpi/channel');
  }

  $title = _bpi_get_channel_subscription_title($user, $channel);

  try {
    $bpi = bpi_client_instance();
    $bpi->createSubscription($user->getId(), array(
        'title' => $title,
        'filter' => array(
          'channels' => array($channel->getId()),
        ),
      ));
    drupal_set_message(t('Channel subscription created'));
  }
  catch (\Exception $e) {
    watchdog_exception('bpi', $e);
    drupal_set_message(t('Cannot subscribe to channel %channel', array('%channel' => $channel_id)), 'error');
  }

  return drupal_goto('admin/bpi/channel');
}

/**
 * Remove a subscription to a channel for the current user.
 *
 * @param string $channel_id
 */
function bpi_channel_unsubscribe($channel_id) {
  $user = bpi_get_user();
  $channel = bpi_get_channel($channel_id);
  if (!$channel) {
    drupal_set_message(t('No such channel: %channel', array('%channel' => $channel_id)), 'error');
    return drupal_goto('admin/bpi/channel');
  }

  $title = _bpi_get_channel_subscription_title($user, $channel);

  try {
    $bpi = bpi_client_instance();
    $bpi->deleteSubscription($user->getId(), $title);
    drupal_set_message(t('Channel subscription deleted'));
  }
  catch (\Exception $e) {
    watchdog_exception('bpi', $e);
    drupal_set_message(t('Cannot unsubscribe from channel %channel', array('%channel' => $channel_id)), 'error');
  }

  return drupal_goto('admin/bpi/channel');
}

/**
 * Get title for a channel subscription for a user.
 *
 * Subscriptions must have unique names (per user).
 */
function _bpi_get_channel_subscription_title(User $user, Channel $channel) {
  return '### ' . $user->getId() . ' - ' . $channel->getId() . ' ###';
}

/**
 * Get current user's bpi user id.
 *
 * @return string|null
 *   The bpi user id if any.
 */
function bpi_get_current_bpi_user_id() {
  global $user;

  return isset($user->data['bpi_user_id']) ? $user->data['bpi_user_id'] : NULL;
}

/**
 * Get all channels or channels having a user as administrator or editor.
 *
 * @param string|null $user_id
 *   Optional user to get channels for.
 *
 * @return array|null
 *   The channels.
 */
function bpi_get_channels($user_id = NULL) {
  $page = pager_find_page();
  $query = _bpi_get_query_parameter('query');
  $filters = array();
  if ($user_id) {
    $filters['userId'] = $user_id;
  }
  $sort = _bpi_get_current_sorting();

  return bpi_search_channels($query, $page, $filters, $sort);
}

/**
 * Get a channel by id.
 *
 * @param string $id
 *   The channel id.
 *
 * @return \Bpi\Sdk\Item\Channel|null
 *   The channel if found.
 */
function bpi_get_channel($id) {
  try {
    $bpi = bpi_client_instance();

    $channel = $bpi->getChannel($id);

    return $channel;
  }
  catch (\Exception $e) {
    watchdog_exception('bpi', $e);
    drupal_set_message(t('Failed to fetch channel. Check reports for more information.'), 'error');
  }

  return NULL;
}

/**
 * Get a user by id.
 *
 * @param string $id
 *   The user id.
 *
 * @return \Bpi\Sdk\Item\User|null
 *   The user if found.
 */
function bpi_get_user($id = NULL) {
  if ($id === NULL) {
    $id = bpi_get_current_bpi_user_id();
  }
  try {
    $bpi = bpi_client_instance();

    $user = $bpi->getUser($id);

    return $user;
  }
  catch (\Exception $e) {
    watchdog_exception('bpi', $e);
    drupal_set_message(t('Failed to fetch user. Check reports for more information.'), 'error');
  }

  return NULL;
}

/**
 * Search channels.
 *
 * @param string $query
 *   The search query.
 * @param int $page
 *   The page (for paging).
 * @param array $filters
 *   The search filters.
 * @param array $sorting
 *   How to sort the result.
 *
 * @return array|null
 *   The channels matching the search query and filters.
 */
function bpi_search_channels($query = '', $page = 1, $filters = array(), $sorting = array()) {
  try {
    $bpi = bpi_client_instance();
    $amount = max(intval(_bpi_get_query_parameter('amount')), BPI_DEFAULT_PAGE_SIZE);
    $offset = $page * $amount;

    if (isset($filters['userId'])) {
      $channels = $bpi->getChannelsByUser($filters['userId']);
    }
    else {
      $channels = $bpi->searchChannels(
        array(
          // 'amount' => $amount,
          // 'offset' => $offset,
          // 'filter' => $filters,
          // 'sort' => $sort ? $sort : array(),
          'search' => $query,
        )
      );
    }

    // Convert to array.
    $list = [];
    foreach ($channels as $channel) {
      $list[] = $channel;
    }

    if ($sorting) {
      // @FIXME: usort is not stable (cf. http://php.net/manual/en/function.usort.php).
      usort($list, function($c0, $c1) use ($sorting) {
        foreach ($sorting as $key => $dir) {
          $v0 = NULL;
          $v1 = NULL;
          $cmp = 0;
          switch ($key) {
            case 'title':
              $v0 = $c0->getName();
              $v1 = $c1->getName();
              break;

            case 'description':
              $v0 = $c0->getDescription();
              $v1 = $c1->getDescription();
              break;

            case 'admin':
              $v0 = $c0->getAdmin() ? $c0->getAdmin()->getName() : NULL;
              $v1 = $c1->getAdmin() ? $c1->getAdmin()->getName() : NULL;
              break;

            case 'nodes':
              $v0 = count($c0->getNodes());
              $v1 = count($c1->getNodes());
              break;

            case 'node_last_added_at':
              $v0 = $c0->getNodeLastAddedAt() ? $c0->getNodeLastAddedAt()->format('c') : NULL;
              $v1 = $c1->getNodeLastAddedAt() ? $c1->getNodeLastAddedAt()->format('c') : NULL;
              break;
          }

          if (is_numeric($v0) && is_numeric($v1)) {
            if ($v0 < $v1) {
              $cmp = -1;
            }
            elseif ($v0 > $v1) {
              $cmp = 1;
            }
          }
          else {
            $cmp = strcasecmp($v0, $v1);
          }
          if ($cmp != 0) {
            return $dir == 'desc' ? -$cmp : $cmp;
          }
        }
        return 0;
      });
    }

    $result = [
      'total_count' => count($list),
      'channels' => [],
    ];

    foreach ($list as $index => $channel) {
      if ($offset <= $index && $index < $offset + $amount) {
        $result['channels'][] = $channel;
      }
      $index++;
    }

    return $result;
  }
  catch (Exception $e) {
    watchdog_exception('bpi', $e);
    drupal_set_message(t('Failed to fetch channels. Check reports for more information.'), 'error');
  }

  return NULL;
}

/**
 * Implements theme_HOOK().
 *
 * @param array $variables
 *   Theme variables.
 *
 * @return string
 *   An HTML string representing the themed output.
 */
function theme_bpi_channel_list(array $variables) {
  return theme('table', $variables);
}

/**
 * Implements hook_preprocess_HOOK().
 */
function bpi_preprocess_bpi_channel_list(array &$variables) {
  $items = $variables['items'];

  $user = bpi_get_user();

  $table_head = array(
    array(
      'data' => bpi_get_sort_link(t('Title'), 'title'),
      'class' => array('bpi-title', _bpi_get_current_sorting('title') ? 'active' : ''),
    ),
    array(
      'data' => t('Description'),
      'data' => bpi_get_sort_link(t('Description'), 'description'),
      'class' => array('bpi-description', _bpi_get_current_sorting('description') ? 'active' : ''),
    ),
    array(
      'data' => bpi_get_sort_link(t('Administrator'), 'admin'),
      'class' => array('bpi-title', _bpi_get_current_sorting('title') ? 'active' : ''),
    ),
    array(
      'data' => bpi_get_sort_link(t('Agency'), 'agency'),
      'class' => array('bpi-agency', _bpi_get_current_sorting('agency') ? 'active' : ''),
    ),
    array(
      'data' => bpi_get_sort_link(t('Number of nodes'), 'nodes'),
      'class' => array('bpi-nodes', _bpi_get_current_sorting('nodes') ? 'active' : ''),
    ),
    array(
      'data' => bpi_get_sort_link(t('Node last added at'), 'node_last_added_at'),
      'class' => array('bpi-node-last-added-at', _bpi_get_current_sorting('node_last_added_at') ? 'active' : ''),
    ),
    array(
      'data' => bpi_get_sort_link(t('Subscribed to channel?'), 'subscribed_to_channel'),
      'class' => array('bpi-subscribed-to-channel', _bpi_get_current_sorting('subscribed_to_channel') ? 'active' : ''),
    ),
    array(
      'data' => t('Actions'),
      'class' => 'bpi-actions',
    ),
  );

  // Set table defaults.
  $variables['header'] = $table_head;
  $variables['sticky'] = TRUE;
  $variables['rows'] = array();


  $rows = array();
  if (!empty($items)) {
    foreach ($items as $item) {
      $title = l($item->getName(), 'admin/bpi/channel/' . $item->getId());
      $description = $item->getDescription();

      $administrator = '';
      $agency = '';
      $admin = $item->getAdmin();
      if ($admin) {
        $administrator = l($admin->getName(), 'admin/bpi/user/' . $admin->getId());
        $agency = $admin->getAgency()->getName();
      }
      $is_subscribed = _bpi_channel_is_subscribed($user, $item);
      $subscribed_to_channel = array(
        'data' => $is_subscribed ? t('Yes') : t('No'),
        'class' => $is_subscribed ? 'bpi-is-subscribed' : 'bpi-is-not-subscribed'
      );

      $actions = [];
      if (bpi_can_edit_channel($item)) {
        $actions[] = l(t('Edit'), 'admin/bpi/channel/' . $item->getId() . '/edit');
        $actions[] = l(t('Delete'), 'admin/bpi/channel/' . $item->getId() . '/delete');
      }
      $actions[] = $is_subscribed
                 ? '<a href="#" onclick="BpiChannel.unsubscribe(\'' . $item->getId() . '\')">' . t('Unsubscribe from channel') . '</a>'
                 : '<a href="#" onclick="BpiChannel.subscribe(\'' . $item->getId() . '\')">' . t('Subscribe to channel') . '</a>';

      $row = array(
        'title' => $title,
        'description' => $description,
        'administrator' => $administrator,
        'agency' => $agency,
        'nodes' => count($item->getNodes()),
        'node_last_added_at' => $item->getNodeLastAddedAt() ? $item->getNodeLastAddedAt()->format('Y-m-d H:i:s') : NULL,
        'subscribed_to_channel' => $subscribed_to_channel,
        'actions' => implode(' ', $actions),
      );

      $rows[] = $row;
    }
  }

  // Unset old data.
  unset($variables['items']);

  // Set table data.
  $variables['rows'] = $rows;
}

function _bpi_channel_is_subscribed(User $user, Channel $channel) {
  if (!$channel || !$user->getSubscriptions()) {
    return FALSE;
  }

  $title = _bpi_get_channel_subscription_title($user, $channel);

  return !empty(array_filter($user->getSubscriptions(), function($subscription) use ($channel, $title) {
    $filter = $subscription->getFilter();
    return $subscription->getTitle() == $title
      && isset($filter['channels']) && is_array($filter['channels']) && in_array($channel->getId(), $filter['channels']);
  }));
}

/**
 * Implements theme_HOOK().
 *
 * @param array $variables
 *   Theme variables.
 *
 * @return string
 *   An HTML string representing the themed output.
 */
function theme_bpi_channel_node_list(array $variables) {
  return theme('table', $variables);
}

/**
 * Implements hook_preprocess_HOOK().
 */
function bpi_preprocess_bpi_channel_node_list(array &$variables) {
  drupal_add_css(drupal_get_path('module', 'bpi') . '/css/bpi-admin.styles.css');

  bpi_preprocess_bpi_search_results($variables);
}

/**
 * Form for removing an editor from a channel.
 *
 * @param array $form
 *   The form.
 * @param array $form_state
 *   The form state.
 *
 * @return array
 *   The form.
 */
function bpi_channel_editor_remove_form(array $form, array &$form_state) {
  list($channel_id, $editor_id) = $form_state['build_info']['args'];

  $channel = bpi_get_channel($channel_id);

  if (!bpi_can_edit_channel($channel)) {
    drupal_set_message(t("You're not allowed to edit this channel"), 'error');
    return $form;
  }

  $editor = bpi_get_user($editor_id);

  if (!$editor) {
    drupal_set_message(t('Invalid editor'), 'error');
    return $form;
  }

  return confirm_form($form,
    t('Are you sure you want to remove %editor from %channel?', array('%editor' => $editor->getName() ? $editor->getName() : $editor->getEmail(), '%channel' => $channel->getName())),
    'admin/bpi/channel/' . $channel->getId(),
    t('This action cannot be undone.'),
    t('Remove'),
    t('Cancel')
  );
}

/**
 * Submit handler for bpi_channel_editor_remove_form.
 *
 * @param array $form
 *   The form.
 * @param array $form_state
 *   The form state.
 */
function bpi_channel_editor_remove_form_submit(array $form, array &$form_state) {
  list($channel_id, $editor_id) = $form_state['build_info']['args'];

  $bpi = bpi_client_instance();
  $success = $bpi->removeEditorFromChannel($channel_id, bpi_get_current_bpi_user_id(), array($editor_id));

  if ($success) {
    drupal_set_message(t('Editor removed from channel'));
  }

  drupal_goto('admin/bpi/channel/' . $channel_id);
}

/**
 * Form for adding a node to a channel.
 *
 * @param array $form
 *   The form.
 * @param array $form_state
 *   The form state.
 *
 * @return array
 *   The form.
 */
function bpi_channel_node_add_form(array $form, array &$form_state) {
  list($node_id) = $form_state['build_info']['args'];

  $items = bpi_get_channels(bpi_get_current_bpi_user_id());
  $options = array('' => '');
  if (isset($items['channels'])) {
    foreach ($items['channels'] as $channel) {
      $options[$channel->getId()] = $channel->getName();
    }
  }

  $bpi = bpi_client_instance();
  $bpi_node = $bpi->getNode($node_id);
  $bpi_content = $bpi_node->getProperties();

  $form['node'] = array(
    '#type' => 'item',
    '#markup' => theme('bpi_preview_item', array(
                 'item' => $bpi_content,
                 'no_actions' => TRUE,
               )),
  );

  $form['bpi_actions_bottom'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'class' => array(
        'bpi-actions',
        drupal_html_class('container-inline'),
      ),
    ),
  );

  $form['bpi_actions_bottom']['channel'] = array(
    '#type' => 'select',
    '#title' => t('Channel'),
    '#options' => $options,
    '#required' => TRUE,
    '#default_value' => isset($_GET['channel']) ? $_GET['channel'] : '',
  );

  $form['bpi_actions_bottom']['add_to_channel'] = array(
    '#type' => 'submit',
    '#value' => t('Add node to channel'),
    '#submit' => array('bpi_channel_node_add_form_submit'),
  );

  return $form;
}

/**
 * Submit handler for bpi_channel_node_add_form.
 *
 * @param array $form
 *   The form.
 * @param array $form_state
 *   The form state.
 */
function bpi_channel_node_add_form_submit(array $form, array &$form_state) {
  list($node_id) = $form_state['build_info']['args'];
  $values = $form_state['values'];
  $channel_id = $values['channel'];

  $bpi = bpi_client_instance();
  $success = $bpi->addNodeToChannel($channel_id, bpi_get_current_bpi_user_id(), array($node_id));

  if ($success) {
    drupal_set_message(t('Node added to channel'));
  }

  drupal_goto('admin/bpi/channel/' . $channel_id);
}

/**
 * Form for removing a node from a channel.
 *
 * @param array $form
 *   The form.
 * @param array $form_state
 *   The form state.
 *
 * @return array
 *   The form.
 */
function bpi_channel_node_remove_form(array $form, array &$form_state) {
  list($channel_id, $node_id) = $form_state['build_info']['args'];

  $bpi = bpi_client_instance();
  $channel = $bpi->getChannel($channel_id);

  if (!bpi_can_edit_channel($channel)) {
    drupal_set_message(t("You're not allowed to edit this channel"), 'error');
    return $form;
  }

  $node = $bpi->getNode($node_id);

  return confirm_form($form,
    t('Are you sure you want to remove %node from %channel?', array('%node' => $node->getProperties()['title'], '%channel' => $channel->getName())),
    'admin/bpi/channel/' . $channel->getId(),
    t('This action cannot be undone.'),
    t('Remove'),
    t('Cancel')
  );
}

/**
 * Submit handler for bpi_channel_node_remove_form.
 *
 * @param array $form
 *   The form.
 * @param array $form_state
 *   The form state.
 */
function bpi_channel_node_remove_form_submit(array $form, array &$form_state) {
  list($channel_id, $node_id) = $form_state['build_info']['args'];

  $bpi = bpi_client_instance();
  $success = $bpi->removeNodeFromChannel($channel_id, bpi_get_current_bpi_user_id(), array($node_id));

  if ($success) {
    drupal_set_message(t('Node removed from channel'));
  }

  drupal_goto('admin/bpi/channel/' . $channel_id);
}

/**
 * Decide if the current user can edit a channel.
 *
 * @param Channel $channel
 *   The channel.
 *
 * @return bool
 *   True iff the current user can edit the channel.
 */
function bpi_can_edit_channel(Channel $channel) {
  return $channel && $channel->getAdmin() && $channel->getAdmin()->getId() == bpi_get_current_bpi_user_id();
}

/**
 * Decide if the current user can administer a channel.
 *
 * @param Channel $channel
 *   The channel.
 *
 * @return bool
 *   True iff the current user can administer the channel.
 */
function bpi_can_contribute_to_channel(Channel $channel) {
  if (bpi_can_edit_channel($channel)) {
    return TRUE;
  }
  $user_id = bpi_get_current_bpi_user_id();
  $editors = $channel->getEditors();
  if ($editors) {
    foreach ($editors as $editor) {
      if ($editor->getId() == $user_id) {
        return TRUE;
      }
    }
  }
  return FALSE;
}

function bpi_check_bpi_user_id() {
  global $user;

  if (empty(bpi_get_current_bpi_user_id())) {
    drupal_set_message(t('Current user (!email) does not have a valid BPI user id', array('!email' => $user->mail)), 'warning');
  }
}
