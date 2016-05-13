(function($) {
  'use strict';

  $(document).ready(function() {
    if ($.fn.chosen) {
      $('.bpi-facets select[multiple]').each(function(index, el) {
        var no_results_text = $(el).data('no-matches');
        $(el).chosen({
          no_results_text: no_results_text,
          allow_single_deselect: true
        });
      });
    }

    var showLoading = function() {
      $('.bpi').addClass('bpi-loading');
    };

    $('form.bpi').on('submit', showLoading);
    $('#edit-bpi-search .button, #edit-bpi-filter .button').on('click', showLoading);

    $('#edit-bpi-filter select').change(function() {
      // Trigger jQuery submit to make sure that any on submit handlers are run.
      $(this.form).submit();
    });
  });
})(jQuery);
