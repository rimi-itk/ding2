<?php
/**
 * @file
 * Handling of BPI users.
 */

// We want to reuse some helper functions.
require_once __DIR__ . '/bpi.channel.inc';

/**
 * Render a list of users.
 *
 * @param array $form
 *   The form.
 * @param array $form_state
 *   The form state.
 *
 * @return array
 *   The form.
 */
function bpi_user_list_form(array $form, array &$form_state) {
  if (bpi_service_is_configured()) {
    drupal_add_library('system', 'drupal.ajax');

    $query_form = array(
      '#type' => 'container',
      '#attributes' => array(
        'class' => array(
          'bpi-query',
        ),
      ),
    );

    $query_form['bpi_search'] = array(
      '#type' => 'container',
      '#attributes' => array(
        'class' => array('bpi-search'),
      ),
    );

    $query_form['bpi_search']['text'] = array(
      '#type' => 'textfield',
      '#title' => t('Name'),
      '#default_value' => _bpi_get_current_search_phrase(),
      '#description' => t('Search in name'),
    );

    $query_form['bpi_search']['bpi_search_agency_id'] = array(
      '#type' => 'select',
      '#multiple' => TRUE,
      '#options' => bpi_get_agencies(),
      '#title' => t('Agency'),
      '#description' => t('Agency id'),
    );

    $query_form['bpi_search']['bpi_search_actions'] = array(
      '#type' => 'container',
      '#attributes' => array(
        'class' => array(
          drupal_html_class('container-inline'),
        ),
      ),
    );

    $query_form['bpi_search']['bpi_search_actions']['bpi_search_submit'] = array(
      '#type' => 'submit',
      '#value' => t('Search'),
    );

    if (_bpi_get_current_search_phrase()) {
      $query_form['bpi_search']['bpi_search_actions']['bpi_search_reset'] = array(
        '#type' => 'item',
        '#markup' => l(t('Reset search'), current_path(), array(
          'query' => NULL,
          'attributes' => array(
            'class' => array('button'),
          ),
        )),
      );
    }

    $items = bpi_get_users();

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

    if (!empty($items['users'])) {
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

    $result_form['bpi_search_results'] = array_merge(bpi_build_user_table($items['users']), array(
      '#empty' => t('No users found'),
    ));

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
          'bpi-actions',
        ),
      ),
    );

    $form['bpi_query'] = $query_form;
    $form['bpi_result'] = $result_form;

    $form['bpi_actions_bottom'] = array(
      '#type' => 'container',
      '#attributes' => array(
        'class' => array(
          'bpi-actions',
          drupal_html_class('container-inline'),
        ),
      ),
    );

    $items = bpi_get_channels(bpi_get_current_bpi_user_id());
    $options = array('' => '');
    if (isset($items['channels'])) {
      foreach ($items['channels'] as $channel) {
        if (bpi_can_edit_channel($channel)) {
          $options[$channel->getId()] = $channel->getName();
        }
      }
    }
    $form['bpi_actions_bottom']['channel'] = array(
      '#type' => 'select',
      '#title' => t('Channel'),
      '#options' => $options,
      '#default_value' => isset($_GET['add-to-channel']) ? $_GET['add-to-channel'] : '',
    );

    $form['bpi_actions_bottom']['add_editors_to_channel_submit'] = array(
      '#type' => 'submit',
      '#value' => t('Add selected editors to channel'),
      '#submit' => array('bpi_user_list_form_add_editors_to_channel_submit'),
      '#validate' => array('bpi_user_list_form_add_editors_to_channel_validate'),
    );

    $form['#attributes']['class'][] = 'bpi';
    $form['#attached']['css'][] = drupal_get_path('module', 'bpi') . '/css/bpi-admin.styles.css';
    $form['#attached']['js'][] = drupal_get_path('module', 'bpi') . '/js/bpi-admin.js';
  }

  return $form;
}

/**
 * Render user details.
 *
 * @param string $user_id
 *   The user id.
 *
 * @return string
 *   The rendered view.
 */
function bpi_user_view($user_id) {
  $user = bpi_get_user($user_id);

  if (!$user) {
    return '';
  }

  $items = bpi_get_channels($user->getId());

  return theme('bpi_user_view', [
    'user' => $user,
    'channels' => $items['channels'],
  ]);
}


/**
 * Validation handler for bpi_user_list_form_add_editors_to_channel_validate.
 *
 * @param array $form
 *   The form.
 * @param array $form_state
 *   The form state.
 */
function bpi_user_list_form_add_editors_to_channel_validate(array $form, array &$form_state) {
  $values = $form_state['values'];
  $channel_id = $values['channel'];
  $editor_ids = array_filter($values['bpi_search_results']);

  if (!$channel_id) {
    form_set_error('channel', t('Please select a channel'));
  }
  if (!$editor_ids) {
    form_set_error('bpi_search_results', t('Please select one or more users'));
  }
}

/**
 * Submit handler for bpi_user_list_form_add_editors_to_channel.
 *
 * @param array $form
 *   The form.
 * @param array $form_state
 *   The form state.
 */
function bpi_user_list_form_add_editors_to_channel_submit(array $form, array &$form_state) {
  $values = $form_state['values'];
  $channel_id = $values['channel'];
  $editor_ids = array_filter($values['bpi_search_results']);

  $bpi = bpi_client_instance();
  $success = $bpi->addEditorToChannel($channel_id, bpi_get_current_bpi_user_id(), $editor_ids);

  if ($success) {
    drupal_set_message(t('Editors added to channel'));
  }

  drupal_goto('admin/bpi/channel/' . $channel_id);
}

/**
 * Show all channels or channels having a user as administrator or editor.
 *
 * @param array $form
 *   The form.
 * @param array $form_state
 *   The form state.
 *
 * @return array
 *   The form.
 */
function bpi_user_channel_list_form(array $form, array &$form_state) {
  $user_id = isset($form_state['build_info']['args'][0]) ? $form_state['build_info']['args'][0] : NULL;
  return bpi_channel_list_form($form, $form_state, $user_id);
}

/**
 * Get all bpi users.
 *
 * @return array|null
 *   The users.
 */
function bpi_get_users() {
  $page = pager_find_page();
  $query = _bpi_get_current_search_phrase();
  $filters = array();
  $sort = _bpi_get_current_sorting();

  return bpi_search_users($query, $page, $filters, $sort);
}

/**
 * Search bpi users.
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
 *   The users matching the search query and filters.
 */
function bpi_search_users($query = '', $page = 1, $filters = array(), $sorting = array()) {
  try {
    $bpi = bpi_client_instance();
    $amount = max(intval(_bpi_get_query_parameter('amount')), BPI_DEFAULT_PAGE_SIZE);
    $offset = $page * $amount;

    $users = $bpi->searchUsers(
      array()
    );

    // Convert to array.
    $list = [];
    foreach ($users as $user) {
      $list[] = $user;
    }

    if ($sorting) {
      // @FIXME: usort is not stable (cf. http://php.net/manual/en/function.usort.php).
      usort($list, function($c0, $c1) use ($sorting) {
        foreach ($sorting as $key => $dir) {
          $v0 = NULL;
          $v1 = NULL;
          $cmp = 0;
          switch ($key) {
            case 'name':
              $v0 = $c0->getName();
              $v1 = $c1->getName();
              break;

            case 'email':
              $v0 = $c0->getEmail();
              $v1 = $c1->getEmail();
              break;

            case 'agency':
              $v0 = $c0->getAgencyId();
              $v1 = $c1->getAgencyId();
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
      'users' => [],
    ];

    foreach ($list as $index => $user) {
      if ($offset <= $index && $index < $offset + $amount) {
        $result['users'][] = $user;
      }
      $index++;
    }

    return $result;
  }
  catch (Exception $e) {
    watchdog_exception('bpi', $e);
    drupal_set_message(t('Failed to fetch users. Check reports for more information.'), 'error');
  }

  return NULL;
}

/**
 * Build a tableselect element for selecting user.
 *
 * @param array $users
 *   The users.
 *
 * @return array
 *   The form element.
 */
function bpi_build_user_table(array $users) {
  $header = array(
    'name' => array(
      'data' => bpi_get_sort_link(t('Name'), 'name'),
      'class' => array('bpi-name', _bpi_get_current_sorting('name') ? 'active' : ''),
    ),
    'email' => array(
      'data' => bpi_get_sort_link(t('E-mail'), 'email'),
      'class' => array('bpi-email', _bpi_get_current_sorting('email') ? 'active' : ''),
    ),
    'agency' => array(
      'data' => bpi_get_sort_link(t('Agency'), 'agency'),
      'class' => array('bpi-agency', _bpi_get_current_sorting('agency') ? 'active' : ''),
    ),
  );

  $options = array();
  if ($users) {
    foreach ($users as $user) {
      $name = l($user->getName(), 'admin/bpi/user/' . $user->getId());
      $email = l($user->getEmail(), 'admin/bpi/user/' . $user->getId());
      $agency = $user->getAgencyId();

      $options[$user->getId()] = array(
        'name' => $name,
        'email' => $email,
        'agency' => $agency,
      );
    }
  }

  return array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $options,
  );
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
function theme_bpi_user_list(array $variables) {
  return theme('table', $variables);
}

/**
 * Implements hook_preprocess_HOOK().
 */
function bpi_preprocess_bpi_user_list(array &$variables) {
  $items = $variables['items'];

  // If no data, do not process anything.
  if (empty($items)) {
    return;
  }

  $table_head = array(
    'name' => array(
      'data' => bpi_get_sort_link(t('Name'), 'name'),
      'class' => array('bpi-name', _bpi_get_current_sorting('name') ? 'active' : ''),
    ),
    'email' => array(
      'data' => bpi_get_sort_link(t('E-mail'), 'email'),
      'class' => array('bpi-email', _bpi_get_current_sorting('email') ? 'active' : ''),
    ),
    'agency' => array(
      'data' => bpi_get_sort_link(t('Agency'), 'agency'),
      'class' => array('bpi-agency', _bpi_get_current_sorting('agency') ? 'active' : ''),
    ),
  );

  // Set table defaults.
  $variables['header'] = $table_head;
  $variables['sticky'] = TRUE;
  $variables['rows'] = array();

  $rows = array();
  foreach ($items as $item) {
    $name = l($item->getName(), 'admin/bpi/user/' . $item->getId());
    $email = l($item->getEmail(), 'admin/bpi/user/' . $item->getId());
    $agency = $item->getAgencyId();

    $rows[] = array(
      'name' => $name,
      'email' => $email,
      'agency' => $agency,
    );
  }

  // Unset old data.
  unset($variables['items']);

  // Set table data.
  $variables['rows'] = $rows;
}