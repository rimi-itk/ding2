(function($) {
  "use strict";

  $(document).ready(function() {
    // Add spinner to search field when searching the data well.
    $('input[name="search_block_form"]').keydown(function(event) {
      // When enter is hit in the search form.
      if (event.which == 13) {
        ting_search_toggle_spinner(true);
      }
    });

    // Hook into the search button as well.
    $('#search-block-form input[type="submit"]').click(function() {
      ting_search_toggle_spinner(true);

      // Continue normal click event.
      $(this).trigger('click');
      return true;
    });
  });

  /**
   * Add or remove spinner class to search block form wrapper.
   *
   * @param bool $state
   *   If TRUE the class spinner is added, FALSE it's removed.
   */
  function ting_search_toggle_spinner(state) {
    if (state === true) {
      $('.search-field-wrapper').addClass('spinner');
    }
    else {
      $('.search-field-wrapper').removeClass('spinner');
    }
  }

  // Override default auto-complete to add spinner class.
  Drupal.jsAC.prototype.setStatus = function (status) {
    switch (status) {
      case 'begin':
        ting_search_toggle_spinner(true);
        break;
      case 'cancel':
      case 'error':
      case 'found':
        ting_search_toggle_spinner(false);
        break;
    }
  };
}(jQuery));
